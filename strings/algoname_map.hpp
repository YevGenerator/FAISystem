#pragma once
#include <array>
#include <string_view>
#include "algos/algoholic.hpp"

namespace NodeSystem::strings
{
    class AlgoNames
    {
    private:
        static constexpr std::array<std::string_view, Core::Algo::Algoholic::count()> algoNames =
            {"2SNO"};

    public:
        static constexpr auto algoId(std::string_view name) -> std::optional<Core::types::Byte>
        {
            for (unsigned i = 0; i < algoNames.size(); i++)
            {
                if (algoNames[i] == name)
                {
                    return i;
                }
            }
            return std::nullopt;
        }

        static constexpr auto algoName(Core::types::Byte algoId) -> std::optional<std::string_view>
        {
            if (algoId >= algoNames.size())
            {
                return std::nullopt;
            }
            return algoNames[algoId];
        }
    };
} // namespace NodeSystem::strings
