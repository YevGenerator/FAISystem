#pragma once
#include <ostream>
#include "commands/CommandList.hpp"
#include "dto/dto.hpp"
#include "types/basic.hpp"

namespace NodeSystem::Compiler
{
    struct BinaryWriter
    {
        std::ostream& out;

        template <typename TCommand>
        void write(const TCommand& cmd) const
        {
            constexpr Core::types::Byte cmd_id = Core::Commands::CommandList::id<TCommand>;
            out.write(reinterpret_cast<const char*>(&cmd_id), sizeof(cmd_id));
            out.write(reinterpret_cast<const char*>(&cmd), sizeof(TCommand));
        }

        void operator()(const strings::dto::Id& deviceId) const
        {
            write(Core::Commands::CmdDevice{.deviceId = deviceId});
        }

        void operator()(const strings::dto::KgIp& ip) const
        {
            write(Core::Commands::CmdIp{ip});
        }

        void operator()(const strings::dto::KgSensor& sensor) const
        {
            write(sensor);
        }

        void operator()(const strings::dto::KgRun& run) const
        {
            write(run);
        }

        void operator()(const strings::dto::KgBindSingle& bind) const
        {
            write(Core::Commands::CmdBindSingle{
                .from = {bind.from.device, bind.from.nodeId.level, bind.from.nodeId.index},
                .deviceTo = bind.deviceTo
            });
        }

        void operator()(const strings::dto::KgBindDouble& bind) const
        {
            for (const auto& to : bind.to)
            {
                write(Core::Commands::CmdBindDouble{
                    .from = {.device = bind.from.device, .id_lvl = bind.from.nodeId.level, .id_val = bind.from.nodeId.index},
                    .to = {.device = to.device, .id_lvl = to.nodeId.level, .id_val = to.nodeId.index, .in_index = to.in_index}
                });
            }
        }

        void operator()(const strings::dto::KgNode& node) const
        {
            write(Core::Commands::CmdNodeCreate{
                .device = node.deviceId,
                .id_lvl = node.id.level,
                .id_val = node.id.index,
                .algo = node.algo,
                .output_c = node.output.c,
                .output_a = node.output.a,
                .inputs_count = static_cast<Core::types::ID>(node.inputs.size())
            });

            for (Core::types::ID i = 0; i < node.inputs.size(); ++i)
            {
                write(Core::Commands::CmdNodeAddInput{
                    .device = node.deviceId,
                    .id_lvl = node.id.level,
                    .id_val = node.id.index,
                    .input_index = i,
                    .input_data = node.inputs[i]
                });
            }
        }
    };
}
