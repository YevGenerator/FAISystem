#pragma once
#include "AlgoResult.hpp"
#include "2SNO/base_algo.hpp"
#include "nodes/NodeInfo.hpp"


namespace NodeSystem::Core::Algo {
    template<typename... TAlgos>
    struct AlgoRegister {
        static constexpr auto count() {
            return sizeof...(TAlgos);
        }

        using AlgoTuple = std::tuple<TAlgos...>;

        template<std::size_t Index>
        using GetAlgo = std::tuple_element_t<Index, AlgoTuple>;

        constexpr static auto execute(std::size_t index, const Nodes::NodeAlgoInfo& state) -> AlgoResult {
            return dispatch<0>(index, state);
        }

    private:
        template<std::size_t I>
        constexpr static auto dispatch(std::size_t index, const Nodes::NodeAlgoInfo& state) -> AlgoResult {
            if constexpr (I < sizeof...(TAlgos)) {
                if (I == index) {
                    using CurrentAlgo = GetAlgo<I>;
                    return CurrentAlgo::calculate(state);
                }
                return dispatch<I + 1>(index, state);
            } else {
                using DefaultAlgo = GetAlgo<0>;
                return DefaultAlgo::calculate(state);
            }
        }
    };

    using Algoholic = AlgoRegister<twoSNO::AlgoTwoSNO>;
} // namespace NodeSystem::Core::Algo
