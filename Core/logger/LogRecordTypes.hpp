#pragma once
#include "commands/CmdTypes.hpp"
#include "nodes/NodeMessage.hpp"
#include "nodes/DataSlot.hpp"
#include "types/basic.hpp"
#include "types/NetField.hpp"
namespace NodeSystem::Core::types { using ProcessData = Nodes::InputMessage; }
namespace NodeSystem::Core::types::logs {
#pragma pack(push, 1)
struct LogSetDeviceId { ID deviceId{}; };
struct LogSetWorkers { WorkerInt workerAmount{}; };
struct LogSetIp { Commands::KgIP ipData{}; };
struct LogRun { ID device{}; NetBool toRun{}; NetBool isSender{}; };
struct LogCreateSensor { ID device{}; ID id_val{}; NetBool isSender{}; NetBool isRandom{}; MsInt period{}; NetDouble toEmit{}; };
struct LogCreateNode { ID device{}; ID id_lvl{}; ID id_val{}; Byte algo{}; Nodes::KgOutput output{}; ID inputs_count{}; NetBool isSender{}; };
struct LogNodeAddInput { ID device{}; ID id_lvl{}; ID id_val{}; ID input_index{}; Nodes::KgInput input_data{}; NetBool isSender{}; };
struct LogDoubleBind { Commands::KgBindFrom from{}; Commands::KgBindTo to{}; NetBool isSender{}; };
struct LogSingleBind { Commands::KgBindFrom from{}; ID deviceTo{}; NetBool isSender{}; };
struct LogSensorOutEvent { Nodes::NodeResultMessage result{}; };
struct LogRouterAccept { Nodes::NodeResultMessage result{}; };
struct LogRouterSendInner { Nodes::NodeProcessMessage message{}; };
struct LogRouterSendForward { Nodes::NodeResultForwardMessage result{}; };
struct LogWorkerAccept { WorkerInt workerId{}; Nodes::NodeProcessMessage message{}; };
struct LogNodeInputFilled { Nodes::NodeId nodeId{}; ID slotId{}; ProcessData data{}; };
struct LogNodeProcessed { Nodes::NodeId nodeId{}; Byte algo{}; NetDouble result{}; };
struct LogNodeDenied { WorkerInt workerId{}; Nodes::NodeProcessMessage message{}; };
struct LogWorkerSend { WorkerInt workerId{}; Nodes::NodeResultMessage result{}; };
struct LogReceivedTcpCommand { ID deviceFrom{}; ID deviceTo{}; };
struct LogSendTcpCommand { ID deviceFrom{}; ID deviceTo{}; };
#pragma pack(pop)
}