#include <lu_main.h>
#include <CrossWindow/CrossWindow.h>

#include <RenderDevice/RenderDevice.h>
#include <Texture/TextureLoader.h>

#include <ProgramRef/Shader_PS.h>
#include <ProgramRef/Shader_VS.h>
#include <ProgramRef/ImGuiPass_PS.h>
#include <ProgramRef/ImGuiPass_VS.h>

// ImGui
#include <Device/Device.h>
#include <Geometry/Geometry.h>

#include <imgui_internal.h>

class ImGuiPass
{
public:
	ImGuiPass(RenderDevice& device, RenderCommandList& command_list, std::shared_ptr<Resource> input_rtv, int width, int height);
	~ImGuiPass();

	void OnUpdate();
	void OnRender(RenderCommandList& command_list);
	void OnResize(int width, int height);

	void OnKey(xwin::Key key, xwin::ButtonState action);
	void OnMouse(bool first, double xpos, double ypos);
	void OnMouseButton(xwin::MouseInput button, xwin::ButtonState action);
	void OnScroll(double xoffset, double yoffset);
	void OnInputChar(unsigned int ch);

	void OnUpdateInout(std::shared_ptr<Resource> rtv);

private:
	void CreateFontsTexture(RenderCommandList& command_list);
	void InitKey();

	RenderDevice&			  m_device;
	std::shared_ptr<Resource> m_input_rtv;
	int						  m_width;
	int						  m_height;

	std::shared_ptr<Resource>					   m_font_texture_view;
	ProgramHolder<ImGuiPass_PS, ImGuiPass_VS>	   m_program;
	std::array<std::unique_ptr<IAVertexBuffer>, 3> m_positions_buffer;
	std::array<std::unique_ptr<IAVertexBuffer>, 3> m_texcoords_buffer;
	std::array<std::unique_ptr<IAVertexBuffer>, 3> m_colors_buffer;
	std::array<std::unique_ptr<IAIndexBuffer>, 3>  m_indices_buffer;
	std::shared_ptr<Resource>					   m_sampler;
};

void xmain(int argc, const char** argv)
{
	// Create Window Description
	xwin::WindowDesc windowDesc;
	windowDesc.name	   = "Test";
	windowDesc.title   = "My Title";
	windowDesc.visible = true;
	windowDesc.width   = 1280;
	windowDesc.height  = 720;

	bool closed = false;

	// Initialize
	xwin::Window	 window;
	xwin::EventQueue eventQueue;

	if (!window.create(windowDesc, eventQueue))
	{
		return;
	}

	//{
	Settings settings;
	settings.api_type					 = ApiType::kDX12;
	settings.frame_count				 = 2;
	settings.vsync						 = true;
	std::shared_ptr<RenderDevice> device = CreateRenderDevice(settings, window.getHwnd(), windowDesc.width, windowDesc.height);

	std::shared_ptr<RenderCommandList> upload_command_list = device->CreateRenderCommandList();

	// std::vector<uint32_t>			   ibuf				   = {0, 1, 2, 0, 2, 3};
	// std::shared_ptr<Resource>		   index			   = device->CreateBuffer(BindFlag::kIndexBuffer | BindFlag::kCopyDest, sizeof(uint32_t) * ibuf.size());
	// upload_command_list->UpdateSubresource(index, 0, ibuf.data(), 0, 0);
	// std::vector<glm::vec3> pbuf = {
	//	glm::vec3(-0.5, 0.5, 0.0),
	//	glm::vec3(0.5, 0.5, 0.0),
	//	glm::vec3(0.5, -0.5, 0.0),
	//	glm::vec3(-0.5, -0.5, 0.0),
	// };
	// std::vector<glm::vec2> uvbuf = {
	//	glm::vec2(0.0, 0.0),
	//	glm::vec2(1.0, 0.0),
	//	glm::vec2(1.0, 1.0),
	//	glm::vec2(0.0, 1.0),
	// };
	// std::shared_ptr<Resource> pos = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(glm::vec3) * pbuf.size());
	// upload_command_list->UpdateSubresource(pos, 0, pbuf.data(), 0, 0);
	// std::shared_ptr<Resource> uv = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(glm::vec2) * uvbuf.size());
	// upload_command_list->UpdateSubresource(uv, 0, uvbuf.data(), 0, 0);
	//
	//  std::shared_ptr<Resource> g_sampler = device->CreateSampler({SamplerFilter::kAnisotropic, SamplerTextureAddressMode::kWrap, SamplerComparisonFunc::kNever});
	//   std::shared_ptr<Resource> diffuseTexture = CreateTexture(*device, *upload_command_list, ASSETS_PATH"model/export3dcoat/export3dcoat_lambert3SG_color.dds");

	// ImGui
	auto	  render_target_view = device->GetBackBuffer(device->GetFrameIndex());
	ImGuiPass imgui_pass(*device, *upload_command_list, render_target_view, windowDesc.width, windowDesc.height);

	upload_command_list->Close();
	device->ExecuteCommandLists({upload_command_list});

	// ProgramHolder<Shader_PS, Shader_VS> program(*device);
	//  program.ps.cbuffer.Settings.color = glm::vec4(1, 0, 0, 1);

	std::vector<std::shared_ptr<RenderCommandList>> command_lists;
	for (uint32_t i = 0; i < settings.frame_count; ++i)
	{
		decltype(auto) command_list = device->CreateRenderCommandList();
		command_lists.emplace_back(command_list);
	}

	// IMGUI Demo: Our state
	bool   show_demo_window	   = true;
	bool   show_another_window = false;
	ImVec4 clear_color		   = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	//}

	// Engine loop
	bool isRunning = true;

	while (isRunning)
	{
		// Update the event queue
		eventQueue.update();

		// Iterate through that queue:
		while (!eventQueue.empty())
		{
			const xwin::Event& event = eventQueue.front();

			if (event.type == xwin::EventType::MouseMove)
			{
				const xwin::MouseMoveData mouse = event.data.mouseMove;
			}
			if (event.type == xwin::EventType::Close)
			{
				window.close();
				isRunning = false;
			}

			eventQueue.pop();
		}

		if (isRunning)
		{
			int32_t frameIndex = device->GetFrameIndex();
			device->Wait(command_lists[frameIndex]->GetFenceValue());
			command_lists[frameIndex]->Reset();

			const auto window_size = window.getWindowSize();
			if ((window_size.x != windowDesc.width) || (window_size.y != windowDesc.height))
			{
				imgui_pass.OnResize(window_size.x, window_size.y);
				windowDesc.width  = window_size.x;
				windowDesc.height = window_size.y;
			}

			imgui_pass.OnUpdate();
			render_target_view = device->GetBackBuffer(frameIndex);
			imgui_pass.OnUpdateInout(render_target_view);
			{
				ImGui::NewFrame();
				ImGui::ShowDemoWindow();
			}
			device->Wait(command_lists[frameIndex]->GetFenceValue());
			command_lists[frameIndex]->BeginEvent("ImGui Pass");
			imgui_pass.OnRender(*command_lists[frameIndex]);
			command_lists[frameIndex]->EndEvent();
			command_lists[frameIndex]->Close();
			device->ExecuteCommandLists({command_lists[device->GetFrameIndex()]});
			device->Present();
		}
	}

	device->WaitForIdle();
}

/////////////////

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Geometry/IABuffer.h>
#include <Utilities/FormatHelper.h>

ImGuiPass::ImGuiPass(RenderDevice& device, RenderCommandList& command_list, std::shared_ptr<Resource> input_rtv, int width, int height)
	: m_device(device)
	, m_input_rtv(input_rtv)
	, m_width(width)
	, m_height(height)
	, m_program(device)
{
	ImGui::CreateContext();
	ImGuiIO& io		   = ImGui::GetIO();
	io.DisplaySize	   = ImVec2((float)width, (float)height);
	float scale		   = 1.0f;
	io.FontGlobalScale = scale;
	ImGui::GetStyle().ScaleAllSizes(scale);

	InitKey();
	CreateFontsTexture(command_list);
	m_sampler = m_device.CreateSampler({SamplerFilter::kMinMagMipLinear, SamplerTextureAddressMode::kWrap, SamplerComparisonFunc::kAlways});
}

ImGuiPass::~ImGuiPass()
{
	ImGui::DestroyContext();
}

void ImGuiPass::OnUpdate() { }

void ImGuiPass::OnUpdateInout(std::shared_ptr<Resource> input_rtv)
{
	m_input_rtv = input_rtv;
}

void ImGuiPass::OnRender(RenderCommandList& command_list)
{
	// ImGui::NewFrame();

	ImGui::Render();

	ImDrawData* draw_data = ImGui::GetDrawData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::vec4> colors;
	std::vector<uint32_t>  indices;

	for (int i = 0; i < draw_data->CmdListsCount; ++i)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[i];
		for (int j = 0; j < cmd_list->VtxBuffer.Size; ++j)
		{
			auto&	 pos = cmd_list->VtxBuffer.Data[j].pos;
			auto&	 uv	 = cmd_list->VtxBuffer.Data[j].uv;
			uint8_t* col = reinterpret_cast<uint8_t*>(&cmd_list->VtxBuffer.Data[j].col);
			positions.push_back({pos.x, pos.y});
			texcoords.push_back({uv.x, uv.y});
			colors.push_back({col[0] / 255.0, col[1] / 255.0, col[2] / 255.0, col[3] / 255.0});
		}

		for (int j = 0; j < cmd_list->IdxBuffer.Size; ++j)
		{
			indices.push_back(cmd_list->IdxBuffer.Data[j]);
		}
	}

	static uint32_t index = 0;
	m_positions_buffer[index].reset(new IAVertexBuffer(m_device, command_list, positions));
	m_texcoords_buffer[index].reset(new IAVertexBuffer(m_device, command_list, texcoords));
	m_colors_buffer[index].reset(new IAVertexBuffer(m_device, command_list, colors));
	m_indices_buffer[index].reset(new IAIndexBuffer(m_device, command_list, indices, gli::format::FORMAT_R32_UINT_PACK32));

	command_list.UseProgram(m_program);
	command_list.Attach(m_program.vs.cbv.vertexBuffer, m_program.vs.cbuffer.vertexBuffer);

	m_program.vs.cbuffer.vertexBuffer.ProjectionMatrix = glm::ortho(0.0f, 1.0f * m_width, 1.0f * m_height, 0.0f);

	RenderPassBeginDesc render_pass_desc				  = {};
	render_pass_desc.colors[m_program.ps.om.rtv0].texture = m_input_rtv;
	render_pass_desc.colors[m_program.ps.om.rtv0].load_op = RenderPassLoadOp::kLoad;

	m_indices_buffer[index]->Bind(command_list);
	m_positions_buffer[index]->BindToSlot(command_list, m_program.vs.ia.POSITION);
	m_texcoords_buffer[index]->BindToSlot(command_list, m_program.vs.ia.TEXCOORD);
	m_colors_buffer[index]->BindToSlot(command_list, m_program.vs.ia.COLOR);

	command_list.Attach(m_program.ps.sampler.sampler0, m_sampler);

	command_list.SetBlendState({true, Blend::kSrcAlpha, Blend::kInvSrcAlpha, BlendOp::kAdd, Blend::kInvSrcAlpha, Blend::kZero, BlendOp::kAdd});

	command_list.SetRasterizeState({FillMode::kSolid, CullMode::kNone});
	command_list.SetDepthStencilState({false, ComparisonFunc::kLessEqual});
	command_list.SetViewport(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

	int vtx_offset = 0;
	int idx_offset = 0;
	command_list.BeginRenderPass(render_pass_desc);
	for (int i = 0; i < draw_data->CmdListsCount; ++i)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[i];
		for (int j = 0; j < cmd_list->CmdBuffer.Size; ++j)
		{
			const ImDrawCmd& cmd = cmd_list->CmdBuffer[j];
			command_list.Attach(m_program.ps.srv.texture0, *(std::shared_ptr<Resource>*)cmd.TextureId);
			command_list.SetScissorRect(cmd.ClipRect.x, cmd.ClipRect.y, cmd.ClipRect.z, cmd.ClipRect.w);
			command_list.DrawIndexed(cmd.ElemCount, 1, idx_offset, vtx_offset, 0);
			idx_offset += cmd.ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}
	command_list.EndRenderPass();

	index = (index + 1) % 3;
}

void ImGuiPass::OnResize(int width, int height)
{
	m_width					   = width;
	m_height				   = height;
	ImGui::GetIO().DisplaySize = ImVec2((float)m_width, (float)m_height);
}

void ImGuiPass::OnKey(xwin::Key key, xwin::ButtonState action)
{
	// if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
	//{
	//     ImGuiIO& io = ImGui::GetIO();
	//     if (action == GLFW_PRESS)
	//         io.KeysDown[key] = true;
	//     if (action == GLFW_RELEASE)
	//         io.KeysDown[key] = false;
	//
	//     io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	//     io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	//     io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
	//     io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
	// }
}

void ImGuiPass::OnMouse(bool first, double xpos, double ypos)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(xpos, ypos);
}

void ImGuiPass::OnMouseButton(xwin::MouseInput button, xwin::ButtonState action)
{
	ImGuiIO& io = ImGui::GetIO();
	if (button == xwin::MouseInput::Left)
	{
		io.MouseDown[0] = action == xwin::ButtonState::Pressed;
	}
	else if (button == xwin::MouseInput::Right)
	{
		io.MouseDown[1] = action == xwin::ButtonState::Pressed;
	}
	else if (button == xwin::MouseInput::Middle)
	{
		io.MouseDown[2] = action == xwin::ButtonState::Pressed;
	}
}

void ImGuiPass::OnScroll(double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel += yoffset;
}

void ImGuiPass::OnInputChar(unsigned int ch)
{
	ImGuiIO& io = ImGui::GetIO();
	if (ch > 0 && ch < 0x10000)
	{
		io.AddInputCharacter((unsigned short)ch);
	}
}

void ImGuiPass::CreateFontsTexture(RenderCommandList& command_list)
{
	// Build texture atlas
	ImGuiIO&	   io = ImGui::GetIO();
	unsigned char* pixels;
	int			   width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	m_font_texture_view = m_device.CreateTexture(BindFlag::kShaderResource | BindFlag::kCopyDest, gli::format::FORMAT_RGBA8_UNORM_PACK8, 1, width, height);
	size_t num_bytes	= 0;
	size_t row_bytes	= 0;
	GetFormatInfo(width, height, gli::format::FORMAT_RGBA8_UNORM_PACK8, num_bytes, row_bytes);
	command_list.UpdateSubresource(m_font_texture_view, 0, pixels, row_bytes, num_bytes);

	// Store our identifier
	io.Fonts->TexID = (void*)&m_font_texture_view;
}

void ImGuiPass::InitKey()
{
	ImGuiIO& io					   = ImGui::GetIO();
	io.KeyMap[ImGuiKey_LeftArrow]  = (int)xwin::Key::Left;
	io.KeyMap[ImGuiKey_RightArrow] = (int)xwin::Key::Right;
	io.KeyMap[ImGuiKey_UpArrow]	   = (int)xwin::Key::Up;
	io.KeyMap[ImGuiKey_DownArrow]  = (int)xwin::Key::Down;
	io.KeyMap[ImGuiKey_PageUp]	   = (int)xwin::Key::PgUp;
	io.KeyMap[ImGuiKey_PageDown]   = (int)xwin::Key::PgDn;
	io.KeyMap[ImGuiKey_Home]	   = (int)xwin::Key::Home;
	io.KeyMap[ImGuiKey_End]		   = (int)xwin::Key::End;
	io.KeyMap[ImGuiKey_Delete]	   = (int)xwin::Key::Del;
	io.KeyMap[ImGuiKey_Backspace]  = (int)xwin::Key::Back;
	io.KeyMap[ImGuiKey_Enter]	   = (int)xwin::Key::Enter;
	io.KeyMap[ImGuiKey_Escape]	   = (int)xwin::Key::Escape;
}
