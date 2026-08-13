#pragma once
#include <variant>

#include "CmdProcessorContext.hpp"
#include "WorkerPool.hpp"
#include "commands/CommandList.hpp"
#include "commands/NetworkPacket.hpp"
#include "types/overload.hpp"

namespace NodeSystem::ZMCore {
    using namespace Core::Commands;

    template<bool IsServer>
    class CmdProcessor {
    private:

    public:
        CmdProcessorContext<IsServer> context;

        void init(CmdProcessorContext<IsServer> cmdContext) {
            this->context = cmdContext;
        }

        template<typename P>
        void executePacket(const NetworkPacket<P> &packet) {
            std::visit(Core::types::overload{
                           [&](std::monostate) -> auto {
                           },
                           [&](const auto &cmd) -> auto { this->process(packet); },
                       }, packet.data);
        }

        void process(const std::monostate &packet) {
        }

        void process(const NetworkPacket<CmdDevice> &cmd) {
            context.deviceConfig.deviceId = cmd.data.deviceId;
            //LoggerBin.log(logs::LogSetDeviceId{cmd.deviceId}, header);
        }

        void process(const NetworkPacket<CmdWorkers> &cmd) {
            context.workerMaster.initWorkers(cmd.data.workerAmount);
            //LoggerBin.log(logs::LogSetWorkers{cmd.workerAmount}, header);
        }

        void process(const NetworkPacket<CmdRun> &cmd) {
            if constexpr (IsServer) {
                if (cmd.data.device != context.deviceConfig.deviceId) {
                    context.tcpQueue.push(cmd.data.device, cmd);
                    return;
                }
            }
            context.run(cmd.data.toRun);
        }

        void process(const NetworkPacket<CmdIp> &cmd) {
            context.deviceConfig.serverAddress.host = cmd.data.host;
            context.deviceConfig.serverAddress.port = cmd.data.port;
            context.need_reconnect.store(true, std::memory_order_relaxed);
        }

        void process(const NetworkPacket<CmdRead> &cmd) {
            //this->loadAndExecute(cmd.filename);
        }

        void process(const NodeResultPacket &cmd) {
            context.routerQueue.push(cmd);
        }

        void process(const NodeProcessPacket &cmd) {
            // this->router.pushResultMessage(CommandList::CreateNetworkPacket(cmd, tag));
        }

        void process(const NetworkPacket<CmdSensorCreate> &cmd) {
            if constexpr (IsServer) {
                if (cmd.data.device != context.deviceConfig.deviceId) {
                    context.tcpQueue.pull(cmd.data.device, cmd);
                    return;
                }
            }
            context.sensors.sensors.add({cmd.data.device, cmd.data.id}, cmd.data.pin_index);
        }

        void process(const NetworkPacket<CmdNodeCreate> &cmd) {
            if constexpr (IsServer) {
                if (cmd.data.device != context.deviceConfig.deviceId) {
                    context.tcpQueue.push(cmd.data.device, cmd);
                    return;
                }
            }
            context.nodeTable.createAndAddNode(cmd.data.toNodeCreateInfo());
        }

        void process(const NetworkPacket<CmdNodeAddInput> &cmd) {
            if constexpr (IsServer) {
                if (cmd.data.device != context.deviceConfig.deviceId) {
                    context.tcpQueue.push(cmd.data.device, cmd);
                    return;
                }
            }
            context.nodeTable.addInputToNode({cmd.data.id_lvl, cmd.data.id_val}, cmd.data.input_data,
                                             cmd.data.input_index);
        }

        void process(const NetworkPacket<CmdBindSingle> &cmd) {
            if constexpr (IsServer) {
                if (cmd.data.from.device != context.deviceConfig.deviceId) {
                    context.tcpQueue.push(cmd.data.from.device, cmd);
                    return;
                }
                if (cmd.data.deviceTo != context.deviceConfig.deviceId) {
                    context.routeTable.add_address({cmd.data.from.id_lvl, cmd.data.from.id_val},
                                                   Core::Nodes::ExternalAddress{cmd.data.deviceTo});
                    return;
                }
            }
            context.routeTable.add_address({cmd.data.from.id_lvl, cmd.data.from.id_val},
                                           Core::Nodes::ExternalAddress{});
        }

        void process(const NetworkPacket<CmdBindDouble> &cmd) {
            if constexpr (IsServer) {
                if (cmd.data.from.device != context.deviceConfig.deviceId) {
                    if (cmd.data.to.device == cmd.data.from.device) {
                        context.tcpQueue.push(cmd.data.from.device, cmd);
                        return;
                    }
                    if (cmd.data.to.device == context.deviceConfig.deviceId) {
                        const auto singleCmd = CmdBindSingle{.from = cmd.data.from};
                        context.tcpQueue.push(cmd.data.from.device,
                                              CommandList::CreatePacket(singleCmd, cmd.eventHeader));
                    } else {
                        const auto singleCmd = CmdBindSingle{.from = cmd.data.from};
                        context.tcpQueue.push(cmd.data.from.device,
                                              CommandList::CreatePacket(singleCmd, cmd.eventHeader));
                        context.routeTable.add_address({cmd.data.from.id_lvl, cmd.data.from.id_val},
                                                       Core::Nodes::ExternalAddress{cmd.data.to.device});

                        context.tcpQueue.push(cmd.data.to.device, cmd);
                        return;
                    }
                } else {
                    if (cmd.data.to.device != context.deviceConfig.deviceId) {
                        context.routeTable.add_address({cmd.data.from.id_lvl, cmd.data.from.id_val},
                                                       Core::Nodes::ExternalAddress{cmd.data.to.device});
                        context.tcpQueue.push(cmd.data.to.device, cmd);
                        return;
                    }
                }
            }
            context.routeTable.add_address({cmd.data.from.id_lvl, cmd.data.from.id_val},
                                           Core::Nodes::InternalAddress{
                                               .targetNodeId = {
                                                   .level = cmd.data.to.id_lvl, .index = cmd.data.to.id_val,
                                               },
                                               .slotId = cmd.data.to.in_index,
                                           });
            //LoggerBin.log(logs::LogDoubleBind{cmd.from, cmd.to, false}, header);
        }
    };
} // namespace NodeSystem::ZMCore
