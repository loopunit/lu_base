cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
cmake_policy(VERSION 3.18)

# ---- Project ----

project(lu_base_test)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- Dependencies ----

include(../cmake/CPM.cmake)

find_package(lu_base MODULE REQUIRED)
find_package(lu_main MODULE REQUIRED)

# ---- Shaders ----

set(pixel_shaders
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/Shader_PS.hlsl
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ImGuiPass_PS.hlsl)

set(vertex_shaders
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/Shader_VS.hlsl
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ImGuiPass_VS.hlsl)

set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_ENTRYPOINT mainPS)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_MODEL 6.5)
set_property(SOURCE ${pixel_shaders} PROPERTY VS_SHADER_FLAGS "/Zi /auto-binding-space 0")

set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_ENTRYPOINT mainVS)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_MODEL 6.5)
set_property(SOURCE ${vertex_shaders} PROPERTY VS_SHADER_FLAGS "/Zi /auto-binding-space 0")

set(shaders_files ${pixel_shaders} ${vertex_shaders})

include(${FlyCube_SOURCE_DIR}/cmake/shader_parser.cmake)

set(assets_path "${CMAKE_CURRENT_SOURCE_DIR}")

gen_shaders_ref(lu_base_test "${shaders_files}" shaders_ref)

# ---- Add source files ----



# ---- Target ----

foreach(output_file ${shaders_ref})
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp APPEND PROPERTY OBJECT_DEPENDS ${output_file})
endforeach()

lu_add_executable(lu_base_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp
    ${shaders_ref})

target_include_directories(lu_base_test PRIVATE "${CMAKE_BINARY_DIR}/gen/lu_base_test")

target_link_libraries(lu_base_test loopunit::lu_base loopunit::lu_main
    Texture
    RenderDevice
    assimp)
	
set_target_properties(lu_base_test PROPERTIES CXX_STANDARD 20)

target_compile_definitions(lu_base_test 
    PUBLIC
        ASSETS_PATH="${assets_path}")
