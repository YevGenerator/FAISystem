#pragma once
#include <thread>

namespace NodeSystem::ZMCore {
    template<typename T>
    concept IsRunnable = requires(T &t, const std::stop_token &st)
    {
        { t.run(st) } -> std::same_as<void>;
    };

    template<IsRunnable TTask>
    class ThreadRunner {
    private:
        TTask task;
        std::jthread thread;

    public:
        template<typename... Args>
        explicit ThreadRunner(Args &&... args)
            : task(std::forward<Args>(args)...) {
        }

        void launch() {
            if (this->is_running()) {
                return;
            }

            this->thread = std::jthread([this](const std::stop_token &st) -> auto {
                this->task.run(st);
            });
        }

        void shutdown() {
            this->thread.request_stop();
        }

        [[nodiscard]]
        auto is_running() const -> bool {
            return this->thread.joinable();
        }
    };
} // namespace NodeSystem::ZMCore
