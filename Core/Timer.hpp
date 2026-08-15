#pragma once
#include <chrono>

#include "types/basic.hpp"

namespace NodeSystem::Core {
    template<typename T>
    concept IsDuration =
            std::same_as<T, std::chrono::microseconds> ||
            std::same_as<T, std::chrono::milliseconds> ||
            std::same_as<T, std::chrono::seconds>;

    struct Timer {
        inline static std::chrono::system_clock::time_point systemStart{};
        inline static std::chrono::steady_clock::time_point steadyStart{};

        static void init() {
            systemStart = std::chrono::system_clock::now();
            steadyStart = std::chrono::steady_clock::now();
        }

        static auto now() {
            return std::chrono::steady_clock::now();
        }

        template<typename TTarget, typename TNumber>
            requires IsDuration<TTarget> && IsDuration<TNumber>
        static constexpr auto add(std::convertible_to<types::UsInt> auto&& target,
                                  std::convertible_to<types::UsInt> auto&& number) -> TTarget {
            return std::chrono::duration_cast<TTarget>(TTarget(target) + TNumber(number));
        }


        static auto elapsedNow() -> types::UsInt {
            const auto duration = now() - steadyStart;
            return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        }

        static auto systemElapsedNow() -> types::UsInt {
            const auto duration = std::chrono::system_clock::now() - systemStart;
            return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        }
    };
    
} // namespace NodeSystem::Core
