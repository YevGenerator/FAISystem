#pragma once
#include "SensorStore.hpp"
#include "queue/concepts/is_queue_push.hpp"
#include "commands/CommandList.hpp"
#include "logger/Logger.hpp"

namespace NodeSystem::Core::Sensors {
    template<Queue::Concepts::IsQueueResultPush TQueueResultPush>
    struct SensorMaster {
        SensorStore sensors{};
        TQueueResultPush pusher;

        SensorMaster(TQueueResultPush pusher) : pusher(std::move(pusher)) {}
        void emitSensors() {
            this->sensors.emitSensors([&](const Nodes::NodeResultMessage &msg) -> auto {
                auto header = LoggerBin.newEvent();
                LoggerBin.log(types::logs::LogSensorOutEvent{msg}, header);
                auto packet = Commands::CommandList::CreatePacket(msg, header);
                this->pusher.push(packet);
            });
        }
    };
} // namespace NodeSystem::Core::Sensors
