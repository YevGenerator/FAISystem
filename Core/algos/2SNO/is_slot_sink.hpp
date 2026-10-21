#pragma once
#include "steps/cf_slot_pipeline.hpp"

namespace NodeSystem::Core::Algo::twoSNO
{
    template <typename T>
    concept IsSlotSink = requires(T sink, const steps::CFSlotPipeline::result_t& trace)
    {
        { sink(trace) };
        { sink.result() } -> std::convertible_to<AlgoFloat>;
    };
}