#pragma once
#include <vector>

#include "ThreadRunner.hpp"
#include "ZMWorker.hpp"

namespace NodeSystem::ZMCore {
    class WorkerPool {
        using WorkerThread = ThreadRunner<ZMWorker>;

    public:
        WorkerPool(zmq::context_t &zmq_context) : context(zmq_context) {
        }

        void initWorkers(const Core::types::WorkerInt workersCount) {
            this->shutdown();
            this->workers.reserve(workersCount);
            for (auto i = 0; i < workersCount; i++) {
                this->workers.emplace_back(i, this->context, this->nodeStore);
            }
        }

        void addActive(const Core::types::WorkerInt id) {
            WorkerThread worker(id, this->context, this->nodeStore);
            worker.launch();
            this->workers.push_back(std::move(worker));
        }

        void startAll() {
            for (auto &worker: this->workers) {
                worker.launch();
            }
        }

        void shutdown() {
            for (auto &worker: this->workers) {
                worker.shutdown();
            }
            this->workers.clear();
        }

        [[nodiscard]]
        auto count() const -> Core::types::WorkerInt {
            return this->workers.size();
        }


    private:
        std::vector<WorkerThread> workers;
        zmq::context_t &context;
    public:
        Core::Nodes::NodeTable nodeStore;
    };
} // namespace NodeSystem::ZMCore
