cmake_minimum_required (VERSION 3.6)

project(RiptideGame CXX)
add_executable(RiptideGame WIN32 src/Main.cpp src/OpenVRInterface.cpp src/OpenVRInterface.h src/TexturedCube.cpp src/TexturedCube.hpp)
target_compile_options(RiptideGame PRIVATE -DUNICODE)
target_include_directories(RiptideGame PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/openvr/headers )
target_link_libraries(RiptideGame PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/openvr/lib/win64/openvr_api.lib)

set_common_target_properties(RiptideGame)
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

copy_required_dlls(RiptideGame)