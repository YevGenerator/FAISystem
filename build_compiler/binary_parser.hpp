#pragma once
#include <optional>
#include <variant>

#include "command_parser.hpp"
#include "commands/CommandList.hpp"
#include "dto/dto.hpp"

namespace NodeSystem::Compiler
{
    class BinaryParser
    {
    private:
        const char* ptr;
        std::size_t available;

        strings::dto::KgNode assembleNode(const Core::Commands::CmdNodeCreate& cmd)
        {
            strings::dto::KgNode inner{
                .deviceId = cmd.device,
                .id = Core::Nodes::NodeId{cmd.id_lvl, cmd.id_val},
                .algo = cmd.algo,
                .output = Core::Nodes::KgOutput{.c = cmd.output_c, .a = cmd.output_a},
                .inputs = {}
            };

            inner.inputs.reserve(cmd.inputs_count);
            for (Core::types::ID i = 0; i < cmd.inputs_count; ++i)
            {
                if (available == 0) break;

                const char* peekPtr = ptr;
                std::size_t peekAvail = available;
                auto nextPkt = Core::Commands::CommandList::nextFromFile(peekPtr, peekAvail);

                if (nextPkt && std::holds_alternative<Core::Commands::CmdNodeAddInput>(*nextPkt))
                {
                    inner.inputs.push_back(std::get<Core::Commands::CmdNodeAddInput>(*nextPkt).input_data);
                    ptr = peekPtr;
                    available = peekAvail;
                }
                else
                {
                    break;
                }
            }
            return strings::dto::KgNode{inner};
        }

        strings::dto::KgBindDouble assembleBindDouble(const Core::Commands::CmdBindDouble& firstCmd)
        {
            strings::dto::KgBindDouble inner{
                .from = {
                    .device = firstCmd.from.device,
                    .nodeId = Core::Nodes::NodeId{firstCmd.from.id_lvl, firstCmd.from.id_val}
                },
                .to = {
                    {
                        .device = firstCmd.to.device,
                        .nodeId = Core::Nodes::NodeId{firstCmd.to.id_lvl, firstCmd.to.id_val},
                        .in_index = firstCmd.to.in_index
                    }
                }
            };

            while (available > 0)
            {
                const char* peekPtr = ptr;
                std::size_t peekAvail = available;
                auto nextPkt = Core::Commands::CommandList::nextFromFile(peekPtr, peekAvail);

                if (nextPkt && std::holds_alternative<Core::Commands::CmdBindDouble>(*nextPkt))
                {
                    const auto& nextBind = std::get<Core::Commands::CmdBindDouble>(*nextPkt);
                    if (nextBind.from.device == firstCmd.from.device &&
                        nextBind.from.id_lvl == firstCmd.from.id_lvl &&
                        nextBind.from.id_val == firstCmd.from.id_val)
                    {
                        inner.to.push_back(strings::dto::KgBindTo{
                            .device = nextBind.to.device,
                            .nodeId = Core::Nodes::NodeId{nextBind.to.id_lvl, nextBind.to.id_val},
                            .in_index = nextBind.to.in_index
                        });
                        ptr = peekPtr;
                        available = peekAvail;
                        continue;
                    }
                }
                break;
            }
            return strings::dto::KgBindDouble{inner};
        }

        struct PacketVisitor
        {
            BinaryParser& parser;

            std::optional<strings::ParsedCommand> operator()(std::monostate) const { return std::nullopt; }

            std::optional<strings::ParsedCommand> operator()(const Core::Commands::CmdDevice& cmd) const
            {
                return strings::dto::Id{cmd.deviceId};
            }

            std::optional<strings::ParsedCommand> operator()(const Core::Commands::CmdIp& cmd) const
            {
                const Core::Commands::KgIP ip{.host = cmd.host, .port = cmd.port};
                return strings::dto::KgIp{ip};
            }

            std::optional<strings::ParsedCommand> operator()(const Core::Commands::CmdSensorCreate& cmd) const
            {
                return strings::dto::KgSensor{cmd};
            }

            std::optional<strings::ParsedCommand> operator()(const Core::Commands::CmdRun& cmd) const
            {
                return strings::dto::KgRun{cmd};
            }

            std::optional<strings::ParsedCommand> operator()(const Core::Commands::CmdBindSingle& cmd) const
            {
                return strings::dto::KgBindSingle{
                    strings::dto::KgBindSingle{
                        .from = strings::dto::KgBindFrom{
                            .device = cmd.from.device,
                            .nodeId = Core::Nodes::NodeId{cmd.from.id_lvl, cmd.from.id_val}
                        },
                        .deviceTo = cmd.deviceTo
                    }
                };
            }

            std::optional<strings::ParsedCommand> operator()(const Core::Commands::CmdNodeCreate& cmd) const
            {
                return parser.assembleNode(cmd);
            }

            std::optional<strings::ParsedCommand> operator()(const Core::Commands::CmdBindDouble& cmd) const
            {
                return parser.assembleBindDouble(cmd);
            }

            template <typename T>
            std::optional<strings::ParsedCommand> operator()(const T&) const { return std::nullopt; }
        };

    public:
        BinaryParser(const char* buffer, std::size_t size)
            : ptr(buffer), available(size)
        {
        }

        std::optional<strings::ParsedCommand> nextCommand()
        {
            while (available > 0)
            {
                auto packetOpt = Core::Commands::CommandList::nextFromFile(ptr, available);
                if (!packetOpt)
                {
                    return std::nullopt;
                }

                if (auto parsed = std::visit(PacketVisitor{*this}, *packetOpt))
                {
                    return parsed;
                }
            }
            return std::nullopt;
        }
    };
}
