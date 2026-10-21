#pragma once
#include <algorithm>
#include <concepts>

#include "cf.hpp"
#include "evaluate_time.hpp"
#include "extract_time.hpp"
#include "ro.hpp"
#include "algos/types.hpp"
#include "algos/2SNO/predefined.hpp"

namespace NodeSystem::Core::Algo::twoSNO::steps
{
    struct CFSlotPipeline
    {
        struct result_t
        {
            EvaluateTime::result_t time;
            CF::result_t cf;
            RO::result_t ro;
            AlgoFloat contribution;
        };

        static constexpr result_t calculate(
            const ExtractTime::result_t& input_theta, const Nodes::InputDataSlot& slot)
        {
            const auto timeRes = EvaluateTime::calculate(input_theta, slot);
            const auto cfRes = CF::calculate(timeRes, slot);
            const auto roRes = RO::calculate(cfRes, slot);
            const auto contrib = slot.kgt.g * roRes.ro;

            return {
                .time = timeRes,
                .cf = cfRes,
                .ro = roRes,
                .contribution = contrib
            };
        }
    };
}
