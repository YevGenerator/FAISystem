#pragma once
#include <mutex>
#include "ThreadRunner.hpp"
#include "WorkerPool.hpp"
#include "ZMRouter.hpp"
#include "ZMSensors.hpp"

namespace NodeSystem::ZMCore {
    // The bus runs independently of CmdRun. CmdRun gates all computation.
    class RuntimeController {
    public:
        RuntimeController(WorkerPool& workers, ZMRouter& router, ZMSensors& sensors)
            : workers_(workers), routerThread_(router), sensorThread_(sensors) {
            workers_.activate();
        }

        void run(bool enabled) {
            std::scoped_lock lock(mutex_);
            if (enabled) {
                // Start the router before producers (workers and sensors).
                routerThread_.launch();
                workers_.run(true);
                sensorThread_.launch();
            } else {
                // Stop producers before stopping the router they send to.
                sensorThread_.shutdown();
                workers_.run(false);
                routerThread_.shutdown();
            }
        }

        ~RuntimeController() { run(false); }

    private:
        WorkerPool& workers_;
        ThreadRunner<ZMRouter&> routerThread_;
        ThreadRunner<ZMSensors&> sensorThread_;
        std::mutex mutex_;
    };
}
