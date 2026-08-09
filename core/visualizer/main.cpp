#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Підключаємо твою логіку
#include "../Core/nodes/Node.hpp"

// Підключаємо візуалізацію
#include "ArchitectureCanvas.hpp"
#include "colors_map.hpp"
#include "floats_map.hpp"
#include "GraphCanvas.hpp"
#include "../Core/logger/Logger.hpp"
#include "LogParser.hpp"
#include "SettingsPanel.hpp"
#include "VisualSensor.hpp"

using namespace NodeSystem::Core;

int main() {
    // 1. Ініціалізація GLFW та OpenGL (стандартний бойлерплейт)
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "RPVisualizer", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);


    NodeSystem::Visual::colorsMap.init();
    NodeSystem::Visual::floatsMap.init();

    // 2. Ініціалізація ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // =======================================================
    // 3. ПІДГОТОВКА ДАНИХ ДЛЯ ВІЗУАЛІЗАЦІЇ
    // =======================================================
    NodeSystem::Visual::ArchitectureCanvas archCanvas;
    NodeSystem::Visual::GraphCanvas canvas;

    // Вектор шляхів до файлів логів різних процесів/пристроїв
    std::vector<std::string> logFiles = {
        R"(C:\Users\perce\CLionProjects\RP2\include\system\cmake-build-debug\server\log.bin)",
          R"(C:\Users\perce\CLionProjects\RP2\include\system\cmake-build-debug\build1\log.bin)",
          R"(C:\Users\perce\CLionProjects\RP2\include\system\cmake-build-debug\build2\log.bin)"
    };

    // Парсер проаналізує всі файли, зшиє їх по parentId->eventId і зупиниться на LogRun
    NodeSystem::Visual::LogParser::parseAndExecute(logFiles, canvas, archCanvas);

    float currentY = 50.0f;
    for (auto& dev : canvas.devices) {
        dev.pos = ImVec2(50, currentY);
        dev.performLayout();
        currentY += dev.size.y + 40.0f; // Відступ між пристроями
    }


    // =======================================================
    // 4. ГОЛОВНИЙ ЦИКЛ ДОДАТКА
    // =======================================================
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        NodeSystem::Visual::DrawSettingsPanel(NodeSystem::Visual::colorsMap, NodeSystem::Visual::floatsMap);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver); // Змінив на FirstUseEver, щоб не перекривало панель
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

        // Рендеринг OpenGL
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Очищення пам'яті
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
