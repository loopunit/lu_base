#pragma once

#include <thread>
#include <atomic>

namespace lu
{
	// c++ conversion of https://github.com/grivet/mpsc-queue
	//
	// Copyright @ 2021 Gaetan Rivet
	// All rights reserved.
	//
	// Redistribution and use in source and binary forms, with or without
	// modification, are permitted provided that the following conditions
	// are met:
	//
	// 1. Redistributions of source code must retain the above copyright
	//    notice, this list of conditions and the following disclaimer.
	//
	// 2. Redistributions in binary form must reproduce the above
	//    copyright notice, this list of conditions and the following
	//    disclaimer in the documentation and/or other materials provided
	//    with the distribution.
	//
	// 3. Neither the name of the copyright holder nor the names of its
	//    contributors may be used to endorse or promote products derived
	//    from this software without specific prior written permission.
	//
	// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
	// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	struct mpsc_queue_node
	{
		std::atomic<mpsc_queue_node*> m_next;

		auto* next()
		{
			return atomic_load_explicit(&m_next, std::memory_order::acquire);
		}
	};

	struct mpsc_queue
	{
		std::atomic<mpsc_queue_node*> m_head;
		std::atomic<mpsc_queue_node*> m_tail;
		mpsc_queue_node				  m_stub;

		enum class poll_result
		{
			EMPTY,
			ITEM,
			RETRY,
		};

		mpsc_queue()
		{
			init();
		}

		inline void init()
		{
			atomic_store_explicit(&m_head, &m_stub, std::memory_order::relaxed);
			atomic_store_explicit(&m_tail, &m_stub, std::memory_order::relaxed);
			atomic_store_explicit(&m_stub.m_next, nullptr, std::memory_order::relaxed);
		}

		inline void push_front(mpsc_queue_node* node)
		{
			auto* tail = atomic_load_explicit(&m_tail, std::memory_order::relaxed);
			atomic_store_explicit(&node->m_next, tail, std::memory_order::relaxed);
			atomic_store_explicit(&m_tail, node, std::memory_order::relaxed);
		}

		inline poll_result poll(mpsc_queue_node** node)
		{
			auto* tail = atomic_load_explicit(&m_tail, std::memory_order::relaxed);
			auto* next = atomic_load_explicit(&tail->m_next, std::memory_order::acquire);

			if (tail == &m_stub)
			{
				if (next == nullptr)
				{
					return poll_result::EMPTY;
				}

				atomic_store_explicit(&m_tail, next, std::memory_order::relaxed);
				tail = next;
				next = atomic_load_explicit(&tail->m_next, std::memory_order::acquire);
			}

			if (next != nullptr)
			{
				atomic_store_explicit(&m_tail, next, std::memory_order::relaxed);
				*node = tail;
				return poll_result::ITEM;
			}

			auto* head = atomic_load_explicit(&m_head, std::memory_order::acquire);
			if (tail != head)
			{
				return poll_result::RETRY;
			}

			insert(&m_stub);

			next = atomic_load_explicit(&tail->m_next, std::memory_order::acquire);
			if (next != nullptr)
			{
				atomic_store_explicit(&m_tail, next, std::memory_order::relaxed);
				*node = tail;
				return poll_result::ITEM;
			}

			return poll_result::EMPTY;
		}

		inline mpsc_queue_node* pop()
		{
			poll_result		 result;
			mpsc_queue_node* node{nullptr};

			do
			{
				auto result = poll(&node);
				if (result == poll_result::EMPTY)
				{
					return nullptr;
				}
			}
			while (result == poll_result::RETRY);

			return node;
		}

		inline mpsc_queue_node* tail()
		{
			auto* tail = atomic_load_explicit(&m_tail, std::memory_order::relaxed);
			auto* next = atomic_load_explicit(&tail->m_next, std::memory_order::acquire);

			if (tail == &m_stub)
			{
				if (next == nullptr)
				{
					return nullptr;
				}

				atomic_store_explicit(&m_tail, next, std::memory_order::relaxed);
				tail = next;
			}

			return tail;
		}

		inline void insert(mpsc_queue_node* node)
		{
			atomic_store_explicit(&node->m_next, nullptr, std::memory_order::relaxed);
			auto* prev = atomic_exchange_explicit(&m_head, node, std::memory_order::acq_rel);
			atomic_store_explicit(&prev->m_next, node, std::memory_order::release);
		}
	};

	// Non-destructive iterator view
	struct mpsc_queue_view
	{
		mpsc_queue* m_q;

		mpsc_queue_view(mpsc_queue* q) : m_q{q} { }

		class iterator
		{
		public:
			iterator() : m_ptr{nullptr} { }

			iterator(mpsc_queue_node* ptr) : m_ptr{ptr} { }

			iterator operator++()
			{
				if (m_ptr != nullptr)
				{
					m_ptr = m_ptr->next();
				}
				return *this;
			}

			bool operator!=(const iterator& other) const
			{
				return m_ptr != other.m_ptr;
			}

			const mpsc_queue_node* operator*() const
			{
				return m_ptr;
			}

		private:
			mpsc_queue_node* m_ptr;
		};

		iterator begin()
		{
			return iterator(m_q->tail());
		}
		iterator end() const
		{
			return iterator(nullptr);
		}
	};

	// Destructive iterator view
	struct mpsc_queue_destructive_view
	{
		mpsc_queue* m_q;

		mpsc_queue_destructive_view(mpsc_queue* q) : m_q{q} { }

		class iterator
		{
		public:
			iterator() : m_ptr(nullptr) { }

			iterator(mpsc_queue* q, mpsc_queue_node* ptr) : m_q{q}, m_ptr{ptr} { }

			iterator operator++()
			{
				if (m_ptr != nullptr)
				{
					m_ptr = m_q->pop();
				}
				return *this;
			}

			bool operator!=(const iterator& other) const
			{
				return m_ptr != other.m_ptr;
			}

			const mpsc_queue_node* operator*() const
			{
				return m_ptr;
			}

		private:
			mpsc_queue_node* m_ptr;
			mpsc_queue*		 m_q;
		};

		iterator begin()
		{
			return iterator(m_q, m_q->tail());
		}
		iterator end() const
		{
			return iterator(m_q, nullptr);
		}
	};
} // namespace lu
