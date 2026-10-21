#pragma once
#include <algorithm>

#include "algos/types.hpp"
#include "algos/utils.hpp"
#include "nodes/NodeInfo.hpp"

namespace NodeSystem::Core::Algo::twoSNO::steps
{
    struct ExtractTime
    {
        using result_t = AlgoLongSigned;

        static constexpr result_t calculate(const Nodes::NodeAlgoInfo& nodeState)
        {
            return to_long(nodeState.currentInput.theta);
        }
    };
}
