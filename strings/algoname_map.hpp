#pragma once
#include <array>
#include <string_view>
#include "algos/algoholic.hpp"

namespace NodeSystem::strings {
    class AlgoNames {
    private:
        static constexpr std::array<std::string_view, Core::Algo::Algoholic::count()> algoNames = {"2SNO"};

    public:
        static constexpr auto algoId(std::string_view name) -> Core::types::Byte {
            for (unsigned i = 0; i < algoNames.size(); i++) {
                if (algoNames[i] == name) {
                    return i;
                }
            }
            return -1;
        }

        static constexpr auto algoName(Core::types::Byte algoId) -> std::string_view {
            return algoNames[algoId];
        }
    };
} // namespace NodeSystem::strings
