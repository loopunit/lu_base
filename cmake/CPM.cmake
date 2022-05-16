set(CPM_DOWNLOAD_VERSION 0.35.0)

if(CPM_SOURCE_CACHE)
  # Expand relative path. This is important if the provided path contains a tilde (~)
  get_filename_component(CPM_SOURCE_CACHE ${CPM_SOURCE_CACHE} ABSOLUTE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})

# TODO: cribbed from crosswindow, relocate to common script

set(LU_API AUTO CACHE STRING "A more specific platform selector to choose from, choose the exact OS API to use, can be WIN32, UWP, COCOA, UIKIT, XCB, XLIB, MIR, WAYLAND, ANDROID, WASM, NOOP.")
set_property(
    CACHE
    LU_API PROPERTY
    STRINGS AUTO WIN32 UWP COCOA UIKIT XCB XLIB MIR WAYLAND ANDROID WASM NOOP
)

set(LU_OS AUTO CACHE STRING "Optional: Choose the OS to build for, defaults to AUTO, but can be WINDOWS, MACOS, LINUX, ANDROID, IOS, WASM.") 
set_property(
    CACHE
    LU_OS PROPERTY
    STRINGS AUTO WINDOWS MACOS LINUX ANDROID IOS WASM NOOP
)

if( NOT (LU_OS STREQUAL "AUTO") AND LU_API STREQUAL "AUTO")
    if(LU_OS STREQUAL "WINDOWS")
        set(LU_API "WIN32")
    elseif(LU_OS STREQUAL "MACOS")
        set(LU_API "COCOA")
    elseif(LU_OS STREQUAL "LINUX")
        set(LU_API "XLIB")
    elseif(LU_OS STREQUAL "ANDROID")
        set(LU_API "ANDROID")
    elseif(LU_OS STREQUAL "IOS")
        set(LU_API "UIKIT")
    elseif(LU_OS STREQUAL "WASM")
        set(LU_API "WASM")
    elseif(LU_OS STREQUAL "NOOP")
        set(LU_API "NOOP")
    else()
        message( SEND_ERROR "LU_OS can only be either AUTO, NOOP, WINDOWS, MACOS, LINUX, ANDROID, IOS, or WASM.")
    endif()
endif()

if(LU_API STREQUAL "AUTO")
    if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
    set(LU_API "WIN32" CACHE STRING "A more specific platform selector to choose from, choose the exact OS protocol to use, can be WIN32, UWP, COCOA, UIKIT, XCB, XLIB, MIR, WAYLAND, ANDROID, WASM, NOOP." FORCE)
    elseif (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")
    set(LU_API "COCOA" CACHE STRING "A more specific platform selector to choose from, choose the exact OS protocol to use, can be WIN32, UWP, COCOA, UIKIT, XCB, XLIB, MIR, WAYLAND, ANDROID, WASM, NOOP." FORCE)
    elseif (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    set(LU_API "XCB" CACHE STRING "A more specific platform selector to choose from, choose the exact OS protocol to use, can be WIN32, UWP, COCOA, UIKIT, XCB, XLIB, MIR, WAYLAND, ANDROID, WASM, NOOP." FORCE)
    endif()
endif()

set(LU_API_PATH "Noop")

if(LU_API STREQUAL "WIN32")
    set(LU_API_PATH "Win32")
elseif(LU_API STREQUAL "UWP")
    set(LU_API_PATH "UWP")
elseif(LU_API STREQUAL "COCOA")
    set(LU_API_PATH "Cocoa")
elseif(LU_API STREQUAL "UIKIT")
    set(LU_API_PATH "UIKit")
elseif(LU_API STREQUAL "XCB")
    set(LU_API_PATH "XCB")
elseif(LU_API STREQUAL "XLIB")
    set(LU_API_PATH "XLIB")
elseif(LU_API STREQUAL "ANDROID")
    set(LU_API_PATH "Android")
elseif(LU_API STREQUAL "UIKIT")
    set(LU_API_PATH "UIKit")
elseif(LU_API STREQUAL "WASM")
    set(LU_API_PATH "WASM")
elseif(LU_API STREQUAL "NOOP")
    set(LU_API_PATH "Noop")
else()
    message( SEND_ERROR "LU_API can only be either AUTO, NOOP, WIN32, UWP, COCOA, UIKIT, XCB, XLIB, MIR, WAYLAND, ANDROID, or WASM.")
endif()

message( STATUS "Building for " ${LU_API_PATH} )

#

function(lu_add_executable targetProject targetSources)
    message("Creating executable:")

    # TODO: add default stubs
	#foreach(source IN LISTS XMAIN_SOURCES)
    #    source_group("" FILES "${source}")
    #endforeach()
    set(LU_FILES "${targetSources}")

    if(LU_API STREQUAL "WIN32" OR LU_API STREQUAL "UWP")
        add_executable(
            ${targetProject}
            WIN32
            "${LU_FILES}"
        )
    elseif(LU_API STREQUAL "COCOA" OR LU_API STREQUAL "UIKIT")
        add_executable(
            ${targetProject}
            MACOSX_BUNDLE
            ${LU_FILES}
        )
    elseif(LU_API STREQUAL "XCB" OR LU_API STREQUAL "XLIB")
        add_executable(
            ${targetProject}
            ${LU_FILES}
        )
    elseif(LU_API STREQUAL "ANDROID")
        add_executable(
            ${targetProject}
            ${LU_FILES}
        )
    elseif(LU_API STREQUAL "WASM")
        add_executable(
            ${targetProject}
            ${LU_FILES}
        )
    elseif(LU_API STREQUAL "NOOP")
        add_executable(
            ${targetProject}
            ${LU_FILES}
        )
    endif()

    target_compile_definitions(${targetProject} PRIVATE XWIN_${XWIN_API}=1 LU_${XWIN_API}=1)

endfunction()

