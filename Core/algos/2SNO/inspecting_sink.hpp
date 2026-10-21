#pragma once
#include <any>

#include "is_slot_sink.hpp"
#include "steps/direct_sum_accumulator.hpp"

namespace NodeSystem::Core::Algo::twoSNO
{
    template <IsSlotSink TSink, typename FInspector>
    struct InspectingSink
    {
        TSink sink{};
        FInspector inspector;

        constexpr void operator()(const steps::CFSlotPipeline::result_t& trace) noexcept
        {
            sink(trace);
            inspector(trace, sink.result());
        }

        [[nodiscard]] constexpr auto result() const noexcept
        {
            return sink.result();
        }
    };

    template <typename TSink, typename FInspector>
    InspectingSink(TSink, FInspector) -> InspectingSink<TSink, FInspector>;

    static_assert(IsSlotSink<InspectingSink<steps::DirectSumAccumulator, std::any>>);
}
