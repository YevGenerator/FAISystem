#pragma once
#include <imgui.h>
#include <string_view>
#include <unordered_map>

#include "color_key.hpp"
#include "color_names.hpp"

namespace NodeSystem::Visual {
    class ColorsMap {
        std::unordered_map<std::string_view, ColorKey> colors{};

        static constexpr auto toImU32(const std::uint8_t r = 255,
                                      const std::uint8_t g = 255,
                                      const std::uint8_t b = 255,
                                      const std::uint8_t a = 255) {
            return IM_COL32(r, g, b, a);
        }

    public:
        ColorKey &operator[](const std::string_view name) {
            return this->colors[name];
        }

        void init(const std::string_view key, const ImU32 color) {
            this->colors[key] = ColorKey{key, color};
        }

        auto& map() {
            return this->colors;
        }

        void init(const std::string_view key,
                  const std::uint8_t r = 255,
                  const std::uint8_t g = 255,
                  const std::uint8_t b = 255,
                  const std::uint8_t a = 255) {
            this->init(key, toImU32(r, g, b, a));
        }

        void init() {
            using namespace NodeSystem::Visual::Keys::Colors;

            this->init(CanvasBackground, 30, 30, 30, 255);
            this->init(PlayerBackground, 20, 20, 25, 220);

            this->init(BlockBackground, 40, 40, 45, 255);
            this->init(HeaderBackground, 50, 50, 60, 255);
            this->init(SelectedHeaderBackground, 80, 80, 100, 255);
            this->init(HeaderForeground, 240, 240, 240, 255);
            this->init(Border, 100, 100, 100, 255);
            this->init(SelectedBorder, 255, 200, 0, 255);

            this->init(SlotHoverBackground, 80, 80, 90, 255);
            this->init(SlotActiveBackground, 0, 100, 200, 50);
            this->init(SlotInBackground, 150, 150, 150, 255);
            this->init(SlotOutPositiveBackground, 50, 200, 50, 255);

            this->init(Link, 150, 150, 150, 200);
            this->init(LinkHighlight, 255, 200, 0, 255);
            this->init(TraceHighlight, 0, 255, 255, 180);
            this->init(TraceHighlightDot, 255, 200, 0, 255);

            this->init(FlightNormal, 0, 255, 255, 255);
            this->init(FlightHover, 255, 255, 0, 255);
            this->init(FlashEmit, 255, 200, 0, 255);
            this->init(FlashReceive, 0, 255, 100, 255);

            auto t = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        }
    };

    static ColorsMap colorsMap{};
}