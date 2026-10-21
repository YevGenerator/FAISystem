#pragma once
#include <array>

#include "algos/types.hpp"

namespace NodeSystem::Core::Algo::twoSNO
{
    struct Predefined
    {
        static constexpr auto std_dev = 0.01f;

        static constexpr auto delta = 0.9f;

        static constexpr auto q_min = -1.1f;
        static constexpr auto q_max = -q_min;
        static constexpr auto q_step = 0.1f;
        static constexpr auto Card = static_cast<unsigned>((q_max - q_min) / q_step + 1);

        static constexpr AlgoFloat us_in_second = 1'000'000.0f;
        static constexpr AlgoFloat dead_threshold_seconds = 5.0f;

        static constexpr auto x_sequence = []
        {
            std::array<AlgoFloat, Card> arr{};
            for (std::size_t i = 0; i < Card; i++)
            {
                arr[i] = q_min + q_step * static_cast<AlgoFloat>(i);
            }
            return arr;
        }();
    };
}
