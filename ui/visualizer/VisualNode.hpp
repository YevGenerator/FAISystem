#pragma once
#include <imgui.h>
#include <vector>
#include <string>
#include <algorithm>
#include <format>
#include <map>

#include "algoname_map.hpp"
#include "color_names.hpp"
#include "../Core/nodes/Node.hpp"
#include "SelectionType.hpp"
#include "VisualBlock.hpp"

namespace NodeSystem::Visual {
    class VisualNode : public VisualBlock {
    public:
        Core::Nodes *coreNode;

        std::map<int, Core::types::ProcessData> lastReceivedData;

        VisualNode(int devId, Core::Nodes *node, ImVec2 position)
            : VisualBlock(position, SelectionType::Node, devId, node->id.level, node->id.index), coreNode(node) {
        }

        [[nodiscard]] auto algoName() const {
            return Core::strings::AlgoNames::algoName(coreNode->algoType).begin();
        }

        std::string title() override {
            return std::format("N{0}.{1}", coreNode->id.level, coreNode->id.index);
        }

        uint32_t getLevel() const override {
            return coreNode->id.level;
        }

        float height() const override {
            const auto bodyHeight = std::max(floatsMap[Keys::Floats::minBodyHeight],
                                             static_cast<float>(coreNode->inputs.size()) *
                                             floatsMap[Keys::Floats::inputSlotHeight]);
            return floatsMap[Keys::Floats::titleHeight] + bodyHeight;
        }

        void draw(ImDrawList *dl, ImVec2 origin, float zoom, SelectionInfo &sel, std::map<PinKey, ImVec2> &pinRegistry,
                  const std::map<PinKey, float> &flashes) override {
            drawBase(dl, origin, zoom, sel);

            ImVec2 start = origin + (pos * zoom);
            ImVec2 bodyMin = start + ImVec2(0, floatsMap[Keys::Floats::titleHeight]) * zoom;
            ImVec2 bodyMax = start + size * zoom;
            float halfWidth = floatsMap[Keys::Floats::width] / 2.0f;

            for (size_t i = 0; i < coreNode->inputs.size(); ++i) {
                ImVec2 slotMin = bodyMin + ImVec2(0, i * floatsMap[Keys::Floats::inputSlotHeight]) * zoom;
                ImVec2 slotMax = slotMin + ImVec2(halfWidth, floatsMap[Keys::Floats::inputSlotHeight]) * zoom;

                PinKey inKey = {parentDeviceId, blockLevel, blockId, static_cast<int>(i)};
                bool isSlotSelected = (sel.type == SelectionType::InputSlot && sel.block == this && sel.slotIndex == static_cast<int>(i));

                if (flashes.contains(inKey)) {
                    float intensity = flashes.at(inKey);
                    auto c = colorsMap[Keys::Colors::FlashReceive];
                    // Беремо RGB з мапи, а Alpha вираховуємо
                    dl->AddRectFilled(slotMin, slotMax, IM_COL32(c.red(), c.green(), c.blue(), static_cast<int>(200 * intensity)));
                } else if (isSlotSelected) {
                    dl->AddRectFilled(slotMin, slotMax, colorsMap[Keys::Colors::SlotHoverBackground]);
                } else if (lastReceivedData.contains(static_cast<int>(i))) {
                    dl->AddRectFilled(slotMin, slotMax, colorsMap[Keys::Colors::SlotActiveBackground]);
                }

                if (isClicked(slotMin, slotMax)) {
                    sel = {SelectionType::InputSlot, this, static_cast<int>(i)};
                }

                ImVec2 connCenter = ImVec2(slotMin.x, slotMin.y + (floatsMap[Keys::Floats::inputSlotHeight] / 2.0f) * zoom);
                pinRegistry[inKey] = connCenter;

                float cHalf = (floatsMap[Keys::Floats::connectorSize] / 2.0f) * zoom;
                dl->AddRectFilled(connCenter - ImVec2(cHalf, cHalf), connCenter + ImVec2(cHalf, cHalf), colorsMap[Keys::Colors::SlotInBackground]);

                dl->AddText(nullptr, floatsMap[Keys::Floats::slotFontSize] * zoom,
                            slotMin + ImVec2(floatsMap[Keys::Floats::textPaddingX], floatsMap[Keys::Floats::textPaddingY]) * zoom,
                            colorsMap[Keys::Colors::HeaderForeground], ("In " + std::to_string(i)).c_str());
            }

            ImVec2 outMin = bodyMin + ImVec2(halfWidth, 0) * zoom;
            ImVec2 outMax = bodyMax;

            PinKey outKey = {parentDeviceId, blockLevel, blockId, -1};
            bool isOutSelected = (sel.type == SelectionType::OutputSlot && sel.block == this);

            if (flashes.contains(outKey)) {
                float intensity = flashes.at(outKey);
                auto c = colorsMap[Keys::Colors::FlashEmit];
                dl->AddRectFilled(outMin, outMax, IM_COL32(c.red(), c.green(), c.blue(), static_cast<int>(200 * intensity)));
            } else if (isOutSelected) {
                dl->AddRectFilled(outMin, outMax, colorsMap[Keys::Colors::SlotHoverBackground]);
            }

            if (isClicked(outMin, outMax)) {
                sel = {SelectionType::OutputSlot, this, -1};
            }

            float actualBodyHeight = bodyMax.y - bodyMin.y;
            ImVec2 outConnCenter = ImVec2(outMax.x, outMin.y + (actualBodyHeight / 2.0f));
            pinRegistry[outKey] = outConnCenter;

            float cHalfOut = (floatsMap[Keys::Floats::connectorSize] / 2.0f) * zoom;
            dl->AddRectFilled(outConnCenter - ImVec2(cHalfOut, cHalfOut), outConnCenter + ImVec2(cHalfOut, cHalfOut), colorsMap[Keys::Colors::SlotInBackground]);
            dl->AddText(nullptr, floatsMap[Keys::Floats::slotFontSize] * zoom,
                        outMin + ImVec2(floatsMap[Keys::Floats::textPaddingX], floatsMap[Keys::Floats::textPaddingY]) * zoom,
                        colorsMap[Keys::Colors::HeaderForeground], "Output");
        }
    };
}
