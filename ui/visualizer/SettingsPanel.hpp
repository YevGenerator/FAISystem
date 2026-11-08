#pragma once
#include <imgui.h>
#include "colors_map.hpp"
#include "floats_map.hpp"

namespace NodeSystem::Visual {
    inline void DrawSettingsPanel(ColorsMap& colorsMap, FloatsMap& floatsMap) {
        ImGui::Begin("Settings");

        if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& [name, colorKey] : colorsMap.map()) {
                ImVec4 colFloat = ImGui::ColorConvertU32ToFloat4(colorKey.color);

                if (ImGui::ColorEdit4(name.data(), (float*)&colFloat)) {
                    colorKey.color = ImGui::ColorConvertFloat4ToU32(colFloat);
                }
            }
        }

        if (ImGui::CollapsingHeader("Floats", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& [name, val] : floatsMap.map()) {
                ImGui::DragFloat(name.data(), &val, 0.5f);
            }
        }

        ImGui::End();
    }
}