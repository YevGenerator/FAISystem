#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>

#include "nodes/Node.hpp"
#include "ArchitectureCanvas.hpp"
#include "colors_map.hpp"
#include "floats_map.hpp"
#include "GraphCanvas.hpp"
#include "logger/Logger.hpp"
#include "LogParser.hpp"
#include "SettingsPanel.hpp"
#include "VisualSensor.hpp"

using namespace NodeSystem::Core;

int main(int argc, char** argv) {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "RPVisualizer", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);


    NodeSystem::Visual::colorsMap.init();
    NodeSystem::Visual::floatsMap.init();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    NodeSystem::Visual::ArchitectureCanvas archCanvas;
    NodeSystem::Visual::GraphCanvas canvas;

    std::vector<std::filesystem::path> logFiles;
    for (int i = 1; i < argc; ++i) logFiles.emplace_back(argv[i]);
    if (logFiles.empty()) {
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
            if (!entry.is_regular_file()) continue;
            const auto& name = entry.path().filename().native();
            const auto suffix = std::filesystem::path(".log.bin").native();
            if (name.ends_with(suffix)) logFiles.push_back(entry.path());
        }
        std::sort(logFiles.begin(), logFiles.end());
    }
    if (logFiles.empty()) {
        std::cerr << "No log files supplied. Usage: RPVisualizer <server.log.bin> <servant-2.log.bin> ...\n";
    }

    NodeSystem::Visual::LogParser::parseAndExecute(logFiles, canvas, archCanvas);

    float currentY = 50.0f;
    for (auto& dev : canvas.devices) {
        dev.pos = ImVec2(50, currentY);
        dev.performLayout();
        currentY += dev.size.y + 40.0f;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        NodeSystem::Visual::DrawSettingsPanel(NodeSystem::Visual::colorsMap, NodeSystem::Visual::floatsMap);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("MainEngineWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);


        if (ImGui::BeginTabBar("MainViewTabs")) {

            if (ImGui::BeginTabItem("Logical graph")) {
                canvas.render();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Architecture")) {
                archCanvas.render();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
