#pragma once
#include <iostream>
#include <stop_token>
#include <thread>
#include <chrono>

#include "queues/ZMQueueList.hpp"
#include "sensor/SensorMaster.hpp"

namespace NodeSystem::ZMCore {
    class ZMSensors {
    public:
        using CoreSensors = Core::Sensors::SensorMaster<Queues::ZMRouterPush>;
        CoreSensors sensors;

        ZMSensors(zmq::context_t &context) : sensors(Queues::ZMRouterPush{context}), context(context) {
        }

        void run(const std::stop_token &stopToken) {
            sensors.pusher.init();
            while (!stopToken.stop_requested()) {
                try {
                    sensors.emitSensors();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                } catch (zmq::error_t const &e) {
                    std::cout << e.what() << '\n';
                }
            }
        }

    private:
        zmq::context_t &context;
    };
} // namespace NodeSystem::ZMCore
