#pragma once
#include <algorithm>
#include <cstddef>
#include <string>

namespace NodeSystem::ZMCore {
    template<std::size_t N>
    struct FixedString {
        constexpr FixedString(const char (&str)[N]) {
            std::copy_n(str, N, this->data);
        }

        constexpr operator const char *() const { return data; }
        constexpr operator std::string_view() const { return {data, N - 1}; }
        char data[N]{};
    };
} // namespace NodeSystem::ZMCore
