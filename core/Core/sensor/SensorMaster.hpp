#pragma once
#include "SensorStore.hpp"
#include "queue/concepts/is_queue_push.hpp"

namespace NodeSystem::Core::Sensors {
    template<Queue::Concepts::IsQueueResultPush TQueueResultPush>
    struct SensorMaster {
        SensorStore sensors;
        TQueueResultPush pusher;

        void emitSensors() {
            this->sensors.emitSensors([&](const Nodes::NodeResultMessage &msg) -> auto {
                auto packet = Commands::CommandList::CreatePacket(msg);
                this->pusher.push(packet);
            });
        }
    };
} // namespace NodeSystem::Core::Sensors
