#pragma once

#include <stop_token>

#include "nodes/Worker.hpp"
#include "queues/ZMQueueList.hpp"

namespace NodeSystem::ZMCore {
    class ZMWorker {
    public:
        using CoreWorker = Core::Nodes::Worker<Queues::ZMRouterPush, Queues::ZMWorkerPull>;

        ZMWorker(const Core::types::WorkerInt id, zmq::context_t &context, Core::Nodes::NodeTable &nodeStore)
            : id(id), coreWorker(nodeStore, Queues::ZMRouterPush{context}, Queues::ZMWorkerPull{context}) {
        }

        void run(const std::stop_token &stopToken) {
            coreWorker.puller.init();
            coreWorker.pusher.init();
            while (!stopToken.stop_requested()) {
                try {
                    coreWorker.process();
                } catch (zmq::error_t const &e) {
                    std::cout << e.what() << '\n';
                }
            }
        }

        Core::types::Byte id{};

    private:
        CoreWorker coreWorker;
    };
} // namespace NodeSystem::ZMCore
