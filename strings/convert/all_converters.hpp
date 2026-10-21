#pragma once
#include "type_parser.hpp"
#include "primitive/algo_type_converter.hpp"
#include "primitive/net_bool_converter.hpp"
#include "primitive/node_id_converter.hpp"
#include "primitive/numbers_converter.hpp"
#include "structs/kg_binddouble_converter.hpp"
#include "structs/kg_bindfrom_converter.hpp"
#include "structs/kg_bindsingle_converter.hpp"
#include "structs/kg_bindto_converter.hpp"
#include "structs/kg_input_converter.hpp"
#include "structs/kg_ip_converter.hpp"
#include "structs/kg_node_converter.hpp"
#include "structs/kg_output_converter.hpp"
#include "structs/kg_run_converter.hpp"
#include "structs/kg_sensor_converter.hpp"

namespace NodeSystem::strings::convert
{
    class ConvertersRegistry
    {
    private:
        template <typename Target, typename Tuple>
        struct FindInTuple;

        template <typename Target>
        struct FindInTuple<Target, std::tuple<>>
        {
            using type = void;
            static constexpr bool found = false;
        };

        template <typename Target, IsAnyStringConverter Head, IsAnyStringConverter... Tail>
        struct FindInTuple<Target, std::tuple<Head, Tail...>>
        {
        private:
            static constexpr bool is_match = std::is_same_v<typename Head::target_t, Target>;
            using next_step = FindInTuple<Target, std::tuple<Tail...>>;

        public:
            using type = std::conditional_t<is_match, Head, typename next_step::type>;
            static constexpr bool found = is_match || next_step::found;
        };

        template <typename Target, typename Tuple>
        struct ResolveConverter
        {
            static_assert(FindInTuple<Target, Tuple>::found,
                          "Converter not found!");

            using type = FindInTuple<Target, Tuple>::type;
        };

    public:
        using AllConverters = std::tuple<
            NetBoolConverter
            , NodeIdConverter
            , IdConverter
            , DoubleConverter
            , AlgoNameConverter
            , KgRunConverter
            , KgIpConverter
            , KgInputConverter
            , KgOutputConverter
            , KgSensorConverter
            , KgNodeConverter
            , KgBindFromConverter
            , KgBindToConverter
            , KgBindSingleConverter
            , KgBindDoubleConverter
        >;

        template <typename T>
        static constexpr bool Exists = FindInTuple<T, AllConverters>::found;

        template <typename T>
        using For = ResolveConverter<T, AllConverters>::type;
    };

    static_assert(ConvertersRegistry::Exists<dto::Id>);
    static_assert(ConvertersRegistry::Exists<dto::Float>);
    static_assert(ConvertersRegistry::Exists<dto::NodeId>);
    static_assert(ConvertersRegistry::Exists<dto::Bool>);
    static_assert(ConvertersRegistry::Exists<dto::AlgoType>);
    static_assert(ConvertersRegistry::Exists<dto::KgRun>);
    static_assert(ConvertersRegistry::Exists<dto::KgIp>);
    static_assert(ConvertersRegistry::Exists<dto::KgInput>);
    static_assert(ConvertersRegistry::Exists<dto::KgOutput>);
    static_assert(ConvertersRegistry::Exists<dto::KgSensor>);
    static_assert(ConvertersRegistry::Exists<dto::KgNode>);
    static_assert(ConvertersRegistry::Exists<dto::KgBindFrom>);
    static_assert(ConvertersRegistry::Exists<dto::KgBindTo>);
    static_assert(ConvertersRegistry::Exists<dto::KgBindSingle>);
    static_assert(ConvertersRegistry::Exists<dto::KgBindDouble>);
}
