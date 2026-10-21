#pragma once
#include <algorithm>

#include "cf_slot_pipeline.hpp"
#include "algos/types.hpp"
#include "algos/2SNO/is_slot_sink.hpp"

namespace NodeSystem::Core::Algo::twoSNO::steps
{
    struct DirectSumAccumulator
    {
    private:
        AlgoFloat m_cf_star{0.0};
        bool m_is_empty = true;

        static constexpr auto direct_sum(const AlgoFloat left,
                                         const AlgoFloat right) -> AlgoFloat
        {
            if ((left > 0 and right > 0) or (left < 0 and right < 0))
            {
                return left + right;
            }
            return (left + right) / (1 - std::min(std::abs(left), std::abs(right)));
        }

    public:
        constexpr void fold(AlgoFloat contribution) noexcept
        {
            if (m_is_empty)
            {
                m_cf_star = contribution;
                m_is_empty = false;
            }
            else
            {
                m_cf_star = direct_sum(m_cf_star, contribution);
            }
        }

        constexpr void operator()(const steps::CFSlotPipeline::result_t& trace) noexcept
        {
            fold(trace.contribution);
        }

        [[nodiscard]] constexpr AlgoFloat result() const noexcept
        {
            return m_cf_star;
        }

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return m_is_empty;
        }
    };

    static_assert(IsSlotSink<DirectSumAccumulator>);
}
