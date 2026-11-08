cmake_minimum_required(VERSION 3.20)
project(Fai_ui_core CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. core_build)

add_executable(Fai_ui_core main.cpp
)

target_link_libraries(Fai_ui_core PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
        RP_Core_Strings
)

add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)