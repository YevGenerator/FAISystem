#pragma once
#include "VisualBlock.hpp"
#include <vector>
#include <map>

namespace NodeSystem::Visual {
    class VisualDevice : public VisualBlock {
    public:
        std::vector<VisualBlock*> blocks;
        Core::types::SmallInt workersNumber = 0;
        std::string ipAddress = "0.0.0.0:0";

        VisualDevice(int id, ImVec2 position)
            : VisualBlock(position, SelectionType::Device, id, -1, id) {}

        std::string title() override { return std::format("Device {0}", blockId); }
        float height() const override { return size.y; }

        void performLayout() {
            if (blocks.empty()) return;

            std::map<uint32_t, std::vector<VisualBlock*>> levels;
            uint32_t maxLevel = 0;
            for (auto* b : blocks) {
                levels[b->getLevel()].push_back(b);
                maxLevel = std::max(maxLevel, b->getLevel());
            }

            std::map<uint32_t, float> columnHeights;
            float maxColumnHeight = 0.0f;

            for (auto& [level, colBlocks] : levels) {
                float h = 0;
                for (auto* b : colBlocks) h += b->height() + floatsMap[Keys::Floats::nodeSpacing];
                h -= floatsMap[Keys::Floats::nodeSpacing];
                columnHeights[level] = h;
                maxColumnHeight = std::max(maxColumnHeight, h);
            }

            float totalWidth = (maxLevel + 1) * floatsMap[Keys::Floats::width] + maxLevel * floatsMap[Keys::Floats::columnSpacing];
            this->size = ImVec2(totalWidth + floatsMap[Keys::Floats::devicePadding] * 2, maxColumnHeight + floatsMap[Keys::Floats::devicePadding] * 2 + floatsMap[Keys::Floats::titleHeight]);

            for (auto& [level, colBlocks] : levels) {
                float currentX = floatsMap[Keys::Floats::devicePadding] + level * (floatsMap[Keys::Floats::width] + floatsMap[Keys::Floats::columnSpacing]);
                float startY = floatsMap[Keys::Floats::titleHeight] + floatsMap[Keys::Floats::devicePadding] + (maxColumnHeight - columnHeights[level]) / 2.0f;
                float currentY = startY;
                for (auto* b : colBlocks) {
                    b->pos = ImVec2(currentX, currentY);
                    currentY += b->height() + floatsMap[Keys::Floats::nodeSpacing];
                }
            }
        }

        void draw(ImDrawList* dl, ImVec2 origin, float zoom, SelectionInfo& sel, std::map<PinKey, ImVec2>& pinRegistry, const std::map<PinKey, float>& flashes) override {
            drawBase(dl, origin, zoom, sel);
            ImVec2 start = origin + (pos * zoom);

            // Реєстрація висячого SingleBind (DeviceId, -1, DeviceId, -1)
            pinRegistry[{this->parentDeviceId, -1, this->blockId, -1}] = ImVec2(start.x, start.y + (floatsMap[Keys::Floats::titleHeight] / 2.0f) * zoom);

            for (auto* b : blocks) {
                b->draw(dl, start, zoom, sel, pinRegistry, flashes);
            }
        }
    };
}