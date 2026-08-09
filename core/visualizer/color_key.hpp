#pragma once
#include <cstdint>
#include <imgui.h>
#include <string>

namespace NodeSystem::Visual {
    struct ColorKey {
        std::string_view key;
        ImU32 color;

        constexpr ColorKey() = default;

        constexpr ColorKey(const std::string_view name, const ImU32 color) : key(name), color(color) {
        }

        [[nodiscard]]
        constexpr std::uint8_t red() const {
            return (this->color >> IM_COL32_R_SHIFT) & 0xFF;
        }

        [[nodiscard]]
        constexpr std::uint8_t green() const {
            return (this->color >> IM_COL32_G_SHIFT) & 0xFF;
        }

        [[nodiscard]]
        constexpr std::uint8_t blue() const {
            return (this->color >> IM_COL32_B_SHIFT) & 0xFF;
        }

        [[nodiscard]]
        constexpr std::uint8_t alpha() const {
            return (this->color >> IM_COL32_A_SHIFT) & 0xFF;
        }

        constexpr void set_red(const std::uint8_t red) {
            this->color = (this->color & ~(0xFF << IM_COL32_R_SHIFT)) | (red << IM_COL32_R_SHIFT);
        }

        constexpr void set_green(const std::uint8_t green) {
            this->color = (this->color & ~(0xFF << IM_COL32_G_SHIFT)) | (green << IM_COL32_G_SHIFT);
        }

        constexpr void set_blue(const std::uint8_t blue) {
            this->color = (this->color & ~(0xFF << IM_COL32_B_SHIFT)) | (blue << IM_COL32_B_SHIFT);
        }

        constexpr void set_a(const uint8_t alpha) {
            this->color = (this->color & ~(0xFF << IM_COL32_A_SHIFT)) | (alpha << IM_COL32_A_SHIFT);
        }

        constexpr operator ImU32() const {
            return this->color;
        }
    };
}
