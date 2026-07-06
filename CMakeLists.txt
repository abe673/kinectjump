
cmake_minimum_required(VERSION 3.20)
project(project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

add_executable(project WIN32 ${SRC_FILES})

set(APP_LIBS "C:/Users/me/LIB" CACHE PATH "Global library directory")

add_library(imgui STATIC
    ${APP_LIBS}/imgui/imgui.cpp
    ${APP_LIBS}/imgui/imgui_draw.cpp
    ${APP_LIBS}/imgui/imgui_tables.cpp
    ${APP_LIBS}/imgui/imgui_widgets.cpp
    ${APP_LIBS}/rlImGui/rlImGui.cpp
)


target_include_directories(imgui PUBLIC
    ${APP_LIBS}/imgui
    ${APP_LIBS}/imgui/backends
    ${APP_LIBS}/raylib-6.0_win64_msvc16/include   # <-- INI PENTING
)


target_include_directories(project PRIVATE
    "C:/Program Files/Microsoft SDKs/Kinect/v1.8/inc"
    ${APP_LIBS}/raylibmsvc/include
    ${APP_LIBS}/rlImGui
    ${APP_LIBS}/rlImGui/extras
)

target_link_directories(project PRIVATE
    "C:/Program Files/Microsoft SDKs/Kinect/v1.8/lib/amd64"
    ${APP_LIBS}/raylib-6.0_win64_msvc16/lib
)

target_link_libraries(project PRIVATE
    raylib
    imgui
    Kinect10
    user32 gdi32 ole32 kernel32 winmm
)

