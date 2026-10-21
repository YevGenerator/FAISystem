#pragma once

#include <stop_token>

#include "nodes/Router.hpp"
#include "queues/ZMQueueList.hpp"

namespace NodeSystem::ZMCore
{
    class ZMRouter
    {
    public:
        using CoreRouter =
        Core::Nodes::Router<Queues::ZMWorkerPush, Queues::ZMForwardPush, Queues::ZMRouterPull>;

        ZMRouter(zmq::context_t& context) :
            coreRouter(
                Queues::ZMWorkerPush{context},
                Queues::ZMForwardPush{context},
                Queues::ZMRouterPull{context}),
            context(context)
        {
        }

        void run(const std::stop_token& stopToken)
        {
            coreRouter.puller.init();
            coreRouter.pusher.init();
            coreRouter.forward.init();
            while (!stopToken.stop_requested())
            {
                try
                {
                    coreRouter.routeNewPacket();
                }
                catch (zmq::error_t const& e)
                {
                    std::cout << e.what() << '\n';
                }
            }
        }

    public:
        CoreRouter coreRouter;
        zmq::context_t& context;
    };
} // namespace NodeSystem::ZMCore
