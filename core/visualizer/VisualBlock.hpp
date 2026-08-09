#pragma once
#include <imgui.h>
#include <string>
#include <map>
#include "color_names.hpp"
#include "SelectionType.hpp"

namespace NodeSystem::Visual {
    class VisualBlock {
    public:
        ImVec2 pos, size;
        bool isSelected = false;
        SelectionType blockType;

        int parentDeviceId;
        int blockLevel;
        int blockId;

        virtual ~VisualBlock() = default;

        VisualBlock(ImVec2 position, SelectionType type, int devId, int lvl, int id)
            : pos(position), size(floatsMap[Keys::Floats::width], 0), blockType(type),
              parentDeviceId(devId), blockLevel(lvl), blockId(id) {
        }

        virtual std::string title() = 0;

        virtual uint32_t getLevel() const { return blockLevel; }

        virtual float height() const = 0;

        virtual void draw(ImDrawList *dl, ImVec2 origin, float zoom, SelectionInfo &sel,
                          std::map<PinKey, ImVec2> &pinRegistry, const std::map<PinKey, float> &flashes) = 0;

        static bool isClicked(const ImVec2 &p_min, const ImVec2 &p_max) {
            return ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsMouseHoveringRect(p_min, p_max);
        }

        void drawBase(ImDrawList *drawList, ImVec2 origin, float zoom, SelectionInfo &sel) {
            this->isSelected = (sel.block == this);

            ImVec2 start = origin + (pos * zoom);
            this->size.y = height();
            ImVec2 blockMax = start + size * zoom;
            ImVec2 headerMax = start + ImVec2(size.x, floatsMap[Keys::Floats::titleHeight]) * zoom;

            // Замінено Background на BlockBackground
            drawList->AddRectFilled(start, blockMax, colorsMap[Keys::Colors::BlockBackground], 0);

            ImU32 headerBg = this->isSelected ? colorsMap[Keys::Colors::SelectedHeaderBackground] : colorsMap[Keys::Colors::HeaderBackground];
            drawList->AddRectFilled(start, headerMax, headerBg, 0);

            drawList->AddText(nullptr, floatsMap[Keys::Floats::fontSize] * zoom,
                              start + ImVec2(floatsMap[Keys::Floats::textPaddingX], floatsMap[Keys::Floats::textPaddingY]) * zoom,
                             colorsMap[Keys::Colors::HeaderForeground], title().c_str());

            ImU32 borderColor = this->isSelected ? colorsMap[Keys::Colors::SelectedBorder] : colorsMap[Keys::Colors::Border];
            float borderThick = (this->isSelected ? floatsMap[Keys::Floats::borderThick] : floatsMap[Keys::Floats::borderThin]) * zoom;
            drawList->AddRect(start, blockMax, borderColor, 0, 0, borderThick);

            if (isClicked(start, headerMax)) {
                sel = {this->blockType, this, -1};
            }
        }
    };
}
