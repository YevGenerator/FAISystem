#pragma once
#include <imgui.h>
#include <string_view>

namespace NodeSystem::Visual {
    struct ColorKey {
        std::string_view name{};
        ImU32 color{};

        constexpr operator ImU32() const noexcept { return color; }
    };
}
