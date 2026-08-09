#pragma once
#include "commands/CommandList.hpp"

namespace NodeSystem::Core::Queue::Concepts {
    template<typename T, typename TTargetPush>
    concept IsQueuePush = requires(T pusher, const TTargetPush &obj)
    {
        pusher.push(obj);
    };

    template<typename T>
    concept IsQueueProcessPush = IsQueuePush<T, Commands::NodeProcessPacket>;

    template<typename T>
    concept IsQueueForwardPush = IsQueuePush<T, Commands::NodeTransportPacket>;

    template<typename T>
    concept IsQueueResultPush = IsQueuePush<T, Commands::NodeResultPacket>;

    template<typename T>
    concept IsRouterQueuePush = IsQueueProcessPush<T> && IsQueueForwardPush<T>;
} // namespace NodeSystem::Core::Queue::Concepts
