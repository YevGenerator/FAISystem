cmake_minimum_required(VERSION 3.20)
project(RPVisualizer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. core_build)

add_executable(RPVisualizer main.cpp
        GraphCanvas.hpp
        VisualNode.hpp
        VisualBlock.hpp
        color_names.hpp
        VisualSensor.hpp
        VisualDevice.hpp
        SelectionType.hpp
        LogParser.hpp
        colors_map.hpp
        color_key.hpp
        float_names.hpp
        floats_map.hpp
        SettingsPanel.hpp
        VisualArchDevice.hpp
        ArchTypes.hpp
        ArchitectureCanvas.hpp
)

target_include_directories(RPVisualizer PRIVATE ../coreі)

target_link_libraries(RPVisualizer PRIVATE
        imgui::imgui
        glfw
        OpenGL::GL
        RP_Core_Strings
)

add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)