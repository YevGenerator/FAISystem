#pragma once
#include <array>

#include "algos/algoholic.hpp"

namespace NodeSystem::Core::strings {
    class AlgoNames {
    private:
        static constexpr std::array<std::string, Algo::Algoholic::count()> algoNames{
            "2SNO",
            "Dummy"
        };

    public:
        static constexpr types::Byte algoId(std::string_view name) {
            for (unsigned i = 0; i < algoNames.size(); i++) {
                if (algoNames[i] == name) {
                    return i;
                }
            }
            return -1;
        }

        static constexpr std::string_view algoName(Core::types::Byte algoId) {
            return algoNames[algoId];
        }
    };
}
