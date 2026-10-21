#pragma once
#include <variant>

#include "convert/primitive/numbers_converter.hpp"
#include "convert/structs/kg_binddouble_converter.hpp"
#include "convert/structs/kg_bindsingle_converter.hpp"
#include "convert/structs/kg_ip_converter.hpp"
#include "convert/structs/kg_node_converter.hpp"
#include "convert/structs/kg_run_converter.hpp"
#include "convert/structs/kg_sensor_converter.hpp"

namespace NodeSystem::strings
{
    using ParsedCommand = std::variant<
        convert::IdConverter::target_t,
        convert::KgNodeConverter::target_t,
        convert::KgIpConverter::target_t,
        convert::KgSensorConverter::target_t,
        convert::KgRunConverter::target_t,
        convert::KgBindSingleConverter::target_t,
        convert::KgBindDoubleConverter::target_t
    >;


    template <const std::string_view& Key, typename Converter>
    struct Route
    {
        static constexpr std::string_view key = Key;
        using converter_t = Converter;
    };


    class CommandParser
    {
    private:
        using LetRules = std::tuple<
            Route<Keywords::Types::Uint32, convert::IdConverter>,
            Route<Keywords::Types::Uint16, convert::NumberConverter<std::uint16_t>>,
            Route<Keywords::Types::Uint8, convert::NumberConverter<std::uint8_t>>,
            Route<Keywords::Types::Float, convert::DoubleConverter>,
            Route<Keywords::Types::NodeId, convert::NodeIdConverter>,
            Route<Keywords::Types::Direction, convert::NetBoolConverter>,
            Route<Keywords::Types::Algo, convert::AlgoNameConverter>,
            Route<Keywords::Types::KgNode, convert::KgNodeConverter>,
            Route<Keywords::Types::KgInput, convert::KgInputConverter>,
            Route<Keywords::Types::KgOutput, convert::KgOutputConverter>
        >;

        using CmdRules = std::tuple<
            Route<Keywords::Cmd::Device, convert::IdConverter>,
            Route<Keywords::Cmd::Node, convert::KgNodeConverter>,
            Route<Keywords::Cmd::Ip, convert::KgIpConverter>,
            Route<Keywords::Cmd::Sensor, convert::KgSensorConverter>,
            Route<Keywords::Cmd::Run, convert::KgRunConverter>,
            Route<Keywords::Cmd::Bind, convert::KgBindSingleConverter>,
            Route<Keywords::Cmd::BindE, convert::KgBindDoubleConverter>
        >;

        static constexpr std::pair<bool, std::string_view> decomposeType(std::string_view rawType)
        {
            using namespace Keywords::Chars;
            if (rawType.starts_with(ListStart))
            {
                auto withoutOpen = rawType.substr(ListStart.size());
                if (withoutOpen.starts_with(ListEnd))
                {
                    return {true, withoutOpen.substr(ListEnd.size())};
                }
            }
            return {false, rawType};
        }

        template <typename TargetConv>
        static void parseAndSetConstant(std::string_view& stream, ParserContext& ctx, std::string_view nameToken)
        {
            if (auto val = convert::TypeParser::parseToken<TargetConv>(stream, ctx))
            {
                ctx.setConstant(nameToken, std::move(*val));
            }
        }

        template <typename TRule>
        static bool matchLetRule(std::string_view& stream, ParserContext& ctx, std::string_view baseType,
                                 std::string_view nameToken, bool isList)
        {
            if (baseType != TRule::key) return false;

            if (isList)
                parseAndSetConstant<convert::ListConverter<typename TRule::converter_t>>(stream, ctx, nameToken);
            else
                parseAndSetConstant<typename TRule::converter_t>(stream, ctx, nameToken);

            return true;
        }

        template <typename... TRules>
        static void processLet(std::string_view& stream, ParserContext& ctx, std::string_view baseType,
                               std::string_view nameToken, bool isList, std::tuple<TRules...>)
        {
            (matchLetRule<TRules>(stream, ctx, baseType, nameToken, isList) || ...);
        }

        template <typename TRoute>
        static bool matchCmdRule(std::string_view& stream, const ParserContext& ctx, std::string_view cmdToken,
                                 std::optional<ParsedCommand>& result)
        {
            if (cmdToken != TRoute::key) return false;

            if (auto val = convert::TypeParser::parseToken<typename TRoute::converter_t>(stream, ctx))
            {
                result = std::move(*val);
            }
            return true;
        }

        template <typename... TRoutes>
        static void processCmd(std::string_view& stream, const ParserContext& ctx, std::string_view cmdToken,
                               std::optional<ParsedCommand>& result, std::tuple<TRoutes...>)
        {
            (matchCmdRule<TRoutes>(stream, ctx, cmdToken, result) || ...);
        }

    public:
        static bool parseLet(std::string_view& stream, ParserContext& ctx)
        {
            std::string_view typeToken = Preparser::consumeNextToken(stream);
            std::string_view nameToken = Preparser::consumeNextToken(stream);
            std::string_view eqToken = Preparser::consumeNextToken(stream);
            if (eqToken != Keywords::Chars::AssignParam) return false;

            auto [isList, baseType] = decomposeType(typeToken);
            bool matched = false;
            auto tryMatch = [&]<typename... TRules>(std::tuple<TRules...>)
            {
                matched = (matchLetRule<TRules>(stream, ctx, baseType, nameToken, isList) || ...);
            };
            tryMatch(LetRules{});
            return matched;
        }

        static std::optional<ParsedCommand> parseNextCommand(std::string_view& stream, ParserContext& ctx)
        {
            std::string_view cmdToken = Preparser::consumeNextToken(stream);
            if (cmdToken.empty() || cmdToken.starts_with(Keywords::Chars::Comment))
            {
                return std::nullopt;
            }
            std::optional<ParsedCommand> result;
            processCmd(stream, ctx, cmdToken, result, CmdRules{});
            return result;
        }
    };
}
