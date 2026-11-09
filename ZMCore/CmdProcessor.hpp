#pragma once
#include <variant>
#include <stdexcept>

#include "CmdProcessorContext.hpp"
#include "WorkerPool.hpp"
#include "commands/CommandList.hpp"
#include "commands/NetworkPacket.hpp"
#include "types/overload.hpp"
#include "logger/Logger.hpp"

namespace NodeSystem::ZMCore {
    using namespace Core::Commands;

    template<bool IsServer>
    class CmdProcessor {
    private:

    public:
        CmdProcessorContext<IsServer> context;
        bool ipApplied = false;

        void init(CmdProcessorContext<IsServer> cmdContext) {
            this->context = cmdContext;
        }

        template<typename P>
        void executePacket(const NetworkPacket<P> &packet) {
            std::visit(Core::types::overload{
                           [&](std::monostate) -> auto {
                           },
                           [&](const auto &cmd) -> auto
                           {
                               auto samePacket = CommandList::CreatePacket(cmd, packet.eventHeader);
                               this->process(samePacket);
                           },
                       }, packet.data);
        }

        void process(const std::monostate &packet) {
        }

        void process(const NetworkPacket<CmdDevice> &cmd) {
            const auto id = context.deviceOverride.value_or(cmd.data.deviceId);
            context.deviceConfig->deviceId = id;
            Core::LoggerBin.setDeviceId(id);
            Core::LoggerBin.log(Core::types::logs::LogSetDeviceId{id}, event(cmd.eventHeader));
        }

        void process(const NetworkPacket<CmdWorkers> &cmd) {
            context.workerMaster->initWorkers(cmd.data.workerAmount);
            Core::LoggerBin.log(Core::types::logs::LogSetWorkers{cmd.data.workerAmount}, event(cmd.eventHeader));
        }

        void process(const NetworkPacket<CmdRun> &cmd) {
            if (cmd.data.device != context.deviceConfig->deviceId) {
                if constexpr (IsServer) {
                    context.outbound->push(cmd.data.device, cmd);
                }
                return;
            }
            Core::LoggerBin.log(Core::types::logs::LogRun{cmd.data.device, cmd.data.toRun, false}, event(cmd.eventHeader));
            context.run(cmd.data.toRun);
        }

        void process(const NetworkPacket<CmdIp> &cmd) {
            if (ipApplied) throw std::runtime_error("IP can only be configured once during startup");
            ipApplied = true;
            context.deviceConfig->serverAddress.host = cmd.data.host;
            context.deviceConfig->serverAddress.port = cmd.data.port;
            Core::LoggerBin.log(Core::types::logs::LogSetIp{cmd.data}, event(cmd.eventHeader));
            if (context.startBus) context.startBus();
            if (context.awaitBusReady) context.awaitBusReady();
        }

        void process(const NetworkPacket<CmdRead> &cmd) {
            //this->loadAndExecute(cmd.filename);
        }

        void process(const NodeResultPacket &cmd) {
            context.routerQueue->push(cmd);
        }

        void process(const NodeProcessPacket &cmd) {
            // this->router.pushResultMessage(CommandList::CreateNetworkPacket(cmd, tag));
        }

        void process(const NetworkPacket<CmdSensorCreate> &cmd) {
            if (cmd.data.device != context.deviceConfig->deviceId) {
                if constexpr (IsServer) {
                    context.outbound->push(cmd.data.device, cmd);
                }
                return;
            }
            context.sensors->sensors.add(cmd.data.id, cmd.data.pin_index);
            Core::LoggerBin.log(Core::types::logs::LogCreateSensor{.device=cmd.data.device, .id_val=cmd.data.id}, event(cmd.eventHeader));
        }

        void process(const NetworkPacket<CmdNodeCreate> &cmd) {
            if (cmd.data.device != context.deviceConfig->deviceId) {
                if constexpr (IsServer) {
                    context.outbound->push(cmd.data.device, cmd);
                }
                return;
            }
            context.nodeTable->createAndAddNode(cmd.data.toNodeCreateInfo());
            Core::LoggerBin.log(Core::types::logs::LogCreateNode{
                .device=cmd.data.device, .id_lvl=cmd.data.id_lvl, .id_val=cmd.data.id_val,
                .algo=cmd.data.algo, .output={cmd.data.output_c, cmd.data.output_a},
                .inputs_count=cmd.data.inputs_count
            }, event(cmd.eventHeader));
        }

        void process(const NetworkPacket<CmdNodeAddInput> &cmd) {
            if (cmd.data.device != context.deviceConfig->deviceId) {
                if constexpr (IsServer) {
                    context.outbound->push(cmd.data.device, cmd);
                }
                return;
            }
            context.nodeTable->addInputToNode({cmd.data.id_lvl, cmd.data.id_val}, cmd.data.input_data,
                                             cmd.data.input_index);
            Core::LoggerBin.log(Core::types::logs::LogNodeAddInput{
                .device=cmd.data.device, .id_lvl=cmd.data.id_lvl, .id_val=cmd.data.id_val,
                .input_index=cmd.data.input_index, .input_data=cmd.data.input_data
            }, event(cmd.eventHeader));
        }

        void process(const NetworkPacket<CmdBindSingle> &cmd) {
            const auto id = context.deviceConfig->deviceId;
            const bool fromLocal = cmd.data.from.device == id;
            // The server distributes a single authoritative topology.
            if (fromLocal || (IsServer && cmd.data.deviceTo != id)) {
                context.routeTable->add_address({cmd.data.from.id_lvl, cmd.data.from.id_val},
                                               Core::Nodes::ExternalAddress{cmd.data.deviceTo});
                Core::LoggerBin.log(Core::types::logs::LogSingleBind{
                    cmd.data.from, cmd.data.deviceTo, false
                }, event(cmd.eventHeader));
            }
            if constexpr (IsServer) {
                if (!fromLocal) context.outbound->push(cmd.data.from.device, cmd);
            }
        }

        void process(const NetworkPacket<CmdBindDouble> &cmd) {
            const auto id = context.deviceConfig->deviceId;
            const bool fromLocal = cmd.data.from.device == id;
            const bool toLocal = cmd.data.to.device == id;
            const Core::Nodes::NodeId source{cmd.data.from.id_lvl, cmd.data.from.id_val};
            const Core::Nodes::InternalAddress target{
                .targetNodeId = {cmd.data.to.id_lvl, cmd.data.to.id_val},
                .slotId = cmd.data.to.in_index
            };
            bool added = false;
            if (fromLocal && toLocal) {
                context.routeTable->add_address(source, target);
                added = true;
            } else if (fromLocal && !toLocal) {
                context.routeTable->add_address(source, Core::Nodes::ExternalAddress{cmd.data.to.device});
                added = true;
            } else if (!fromLocal && toLocal) {
                context.routeTable->add_address(source, target);
                added = true;
            } else if constexpr (IsServer) {
                // Central server bridges traffic between two remote devices.
                if (cmd.data.from.device != cmd.data.to.device) {
                    context.routeTable->add_address(source, Core::Nodes::ExternalAddress{cmd.data.to.device});
                    added = true;
                }
            }
            if (added) {
                Core::LoggerBin.log(Core::types::logs::LogDoubleBind{
                    cmd.data.from, cmd.data.to, false
                }, event(cmd.eventHeader));
            }
            if constexpr (IsServer) {
                {
                    if (!fromLocal)
                        context.outbound->push(cmd.data.from.device, cmd);
                    if (!toLocal && cmd.data.to.device != cmd.data.from.device)
                        context.outbound->push(cmd.data.to.device, cmd);
                }
            }
        }

        void process(const NodeTransportPacket &packet) {
            if constexpr (IsServer) {
                if (packet.data.deviceId != context.deviceConfig->deviceId) {
                    context.outbound->push(packet.data.deviceId, packet);
                    return;
                }
            }
            const Core::Nodes::NodeResultMessage message{
                .nodeId = packet.data.nodeId, .alpha = packet.data.alpha
            };
            context.routerQueue->push(CommandList::CreatePacket(message, packet.eventHeader));
        }

    private:
        static auto event(const Core::Events::EventHeader& parent) {
            return Core::LoggerBin.newEvent(parent);
        }
    };
} // namespace NodeSystem::ZMCore
