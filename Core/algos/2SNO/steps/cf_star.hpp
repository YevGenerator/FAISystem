#pragma once
#include <algorithm>
#include <cmath>

#include "cf.hpp"
#include "direct_sum_accumulator.hpp"
#include "evaluate_time.hpp"
#include "ro.hpp"
#include "algos/types.hpp"
#include "algos/2SNO/is_slot_sink.hpp"
#include "algos/2SNO/predefined.hpp"
#include "nodes/NodeInfo.hpp"

namespace NodeSystem::Core::Algo::twoSNO::steps
{
    struct CF_Star
    {
        struct result_t
        {
            AlgoFloat cf_star;
            AlgoFloat cf_imkg;
        };

    private:
        static constexpr auto bound(const AlgoFloat cf_star)
        {
            if (cf_star < -1)
            {
                return -1.0;
            }
            if (cf_star > 1)
            {
                return 1.0;
            }
            return cf_star;
        }

    public:
        template <IsSlotSink TSink = DirectSumAccumulator>
        static constexpr result_t calculate(const ExtractTime::result_t& time,
                                            std::span<const Nodes::InputDataSlot> inputs,
                                            TSink sink = TSink{})
        {
            for (const auto& slot : inputs)
            {
                auto result = CFSlotPipeline::calculate(time, slot);
                sink(result);
            }
            const auto cf_bound = bound(sink.result());
            return {
                .cf_star = sink.result(),
                .cf_imkg = cf_bound
            };
        }
    };
}
