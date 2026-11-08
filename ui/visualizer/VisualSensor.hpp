#pragma once

#include "VisualBlock.hpp"

namespace NodeSystem::Visual {
    class VisualSensor : public VisualBlock {
    public:
        uint64_t period = 0;
        double toEmit = 0.0;

        VisualSensor(int devId, int id, ImVec2 position)
            : VisualBlock(position, SelectionType::Sensor, devId, 0, id) {
        }

        std::string title() override { return std::format("S0.{0}", blockId); }
        float height() const override { return floatsMap[Keys::Floats::titleHeight]; }

        void draw(ImDrawList *dl, ImVec2 origin, float zoom, SelectionInfo &sel, std::map<PinKey, ImVec2> &pinRegistry,
                  const std::map<PinKey, float> &flashes) override {
            drawBase(dl, origin, zoom, sel);

            ImVec2 start = origin + (pos * zoom);
            ImVec2 headerMax = start + ImVec2(size.x, floatsMap[Keys::Floats::titleHeight]) * zoom;

            PinKey outKey = {parentDeviceId, blockLevel, blockId, -1};
            if (flashes.contains(outKey)) {
                float intensity = flashes.at(outKey);
                auto c = colorsMap[Keys::Colors::FlashEmit];
                dl->AddRectFilled(start, headerMax, IM_COL32(c.red(), c.green(), c.blue(), static_cast<int>(180 * intensity)));
            }

            float cHalf = (floatsMap[Keys::Floats::connectorSize] / 2.0f) * zoom;
            ImVec2 outConnCenter = ImVec2(headerMax.x, start.y + (floatsMap[Keys::Floats::titleHeight] / 2.0f) * zoom);

            pinRegistry[outKey] = outConnCenter;

            dl->AddRectFilled(outConnCenter - ImVec2(cHalf, cHalf),
                              outConnCenter + ImVec2(cHalf, cHalf),
                              colorsMap[Keys::Colors::SlotOutPositiveBackground]);
        }
    };
}
