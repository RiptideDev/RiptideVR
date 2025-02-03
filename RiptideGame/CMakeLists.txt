cmake_minimum_required (VERSION 3.10)

project(RiptideGame)

if(PLATFORM_WIN32)
    set(SOURCE 
        src/Win32/SampleAppWin32.cpp
        src/Win32/InputControllerWin32.cpp
        src/SampleApp.cpp
    )
    set(INCLUDE 
        include/SampleApp.hpp
        include/Win32/InputControllerWin32.hpp
    )
    set(WIN32_RESOURCES
        src/Win32/resources/Win32AppResource.h
        src/Win32/resources/directx11-logo.bmp
        src/Win32/resources/directx12-logo.bmp
        src/Win32/resources/vulkan-logo.bmp
        src/Win32/resources/opengl-logo.bmp
        src/Win32/resources/webgpu-logo.bmp
        src/Win32/resources/Win32AppResource.rc
    )

    function(append_sample_base_win32_source TARGET_NAME)
        get_target_property(SAMPLE_BASE_SOURCE_DIR RiptideGame SOURCE_DIR)
        # We need to add .res file to executable target to make resources available
        set(RES_FILE ${SAMPLE_BASE_SOURCE_DIR}/src/Win32/resources/Win32AppResource.rc)
        target_sources(${TARGET_NAME} PRIVATE ${RES_FILE})
        source_group("resources" FILES ${RES_FILE})
    endfunction()

elseif(PLATFORM_LINUX)
    set(SOURCE 
        src/Linux/InputControllerLinux.cpp
        src/Linux/SampleAppLinux.cpp
        src/SampleApp.cpp
    )
    set(INCLUDE 
        include/Linux/InputControllerLinux.hpp
        include/SampleApp.hpp
    )
else()
    message(FATAL_ERROR "Unknown platform")
endif()

list(APPEND SOURCE
    src/FirstPersonCamera.cpp
    src/SampleBase.cpp
)

list(APPEND INCLUDE
    include/FirstPersonCamera.hpp
    include/TrackballCamera.hpp
    include/InputController.hpp
    include/SampleBase.hpp
)


add_library(RiptideGame STATIC ${SOURCE} ${INCLUDE})
set_common_target_properties(RiptideGame)

target_include_directories(RiptideGame
PUBLIC
    include
)

if(MSVC)
    target_compile_options(RiptideGame PRIVATE -DUNICODE)

    if(PLATFORM_UNIVERSAL_WINDOWS)
        # Disable w4189: local variable is initialized but not referenced
        # Disable w4063: case is not a valid value for switch of enum
        # Consume the windows runtime extensions (/ZW)
        target_compile_options(RiptideGame INTERFACE /wd4189 /wd4063 /ZW)
    endif()
endif()

get_supported_backends(ENGINE_LIBRARIES)

target_link_libraries(RiptideGame 
PRIVATE 
    Diligent-BuildSettings
PUBLIC
    Diligent-Common
    Diligent-GraphicsTools
    Diligent-TextureLoader
    Diligent-TargetPlatform
    Diligent-Imgui
    Diligent-GraphicsAccessories
    ${ENGINE_LIBRARIES}
    Diligent-NativeAppBase
)

if(PLATFORM_UNIVERSAL_WINDOWS)
    target_link_libraries(RiptideGame PRIVATE dxguid.lib)
elseif(PLATFORM_LINUX)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(RiptideGame PRIVATE XCBKeySyms OpenGL::GL OpenGL::GLX X11::X11)
endif()

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

if(PLATFORM_WIN32)
    target_sources(RiptideGame PRIVATE ${WIN32_RESOURCES})
    source_group("resources" FILES ${WIN32_RESOURCES})
endif()
