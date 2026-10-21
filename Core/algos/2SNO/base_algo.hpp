#pragma once
#include <cmath>
#include <iostream>
#include <random>

#include "predefined.hpp"
#include "algos/AlgoResult.hpp"
#include "algos/types.hpp"
#include "nodes/NodeInfo.hpp"
#include "steps/alpha.hpp"
#include "steps/evaluate_time.hpp"
#include "steps/cf.hpp"
#include "steps/cf_star.hpp"
#include "steps/ro.hpp"

namespace NodeSystem::Core::Algo::twoSNO
{
    struct AlgoTwoSNO
    {
        template <IsSlotSink TSink = steps::DirectSumAccumulator>
        static constexpr AlgoResult calculate(const Nodes::NodeAlgoInfo& nodeState, TSink sink = TSink{})
        {
            auto step1 = steps::ExtractTime::calculate(nodeState);
            auto step2 = steps::CF_Star::calculate(step1, nodeState.inputs, sink);
            auto step3 = steps::Alpha::calculate(step2, nodeState);
            return {
                .toSwitch = step3.toSwitch,
                .alpha = step3.alpha
            };
        }
    };
} // namespace NodeSystem::Core::Algo
