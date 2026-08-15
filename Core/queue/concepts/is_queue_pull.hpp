#pragma once
#include <concepts>
#include <optional>

namespace NodeSystem::Core::Queue::Concepts {
    template<typename T, typename TTargetPull>
    concept IsQueuePull = requires(T puller)
    {
        typename T::target_t;
        requires std::same_as<typename T::target_t, TTargetPull>;
        { puller.pull() } -> std::same_as<std::optional<typename T::target_t> >;
    };

    template<typename T>
    concept IsQueueResultPull = IsQueuePull<T, Commands::NodeResultPacket>;

    template<typename T>
    concept IsQueueProcessPull = IsQueuePull<T, Commands::NodeProcessPacket>;
} // namespace NodeSystem::Core::Queue::Concepts
