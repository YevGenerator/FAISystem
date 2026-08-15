add_library(ZMCore INTERFACE
        ConfigReader.hpp
        RpController.hpp
        ThreadRunner.hpp
        ZMWorker.hpp
        WorkerPool.hpp
        queues/ZMQueueBase.hpp
        queues/ZMQueueList.hpp
        queues/fixed_string.hpp
        queues/policies/connect_policy.hpp
        queues/policies/protocol_policy.hpp
        queues/policies/policy_list.hpp
        queues/ZMQueueList.hpp
        ZMRouter.hpp
        IpCharAddress.hpp
        IpCharAddress.hpp
        ZMBus.hpp
        ZMSensors.hpp
        CmdProcessor.hpp
        delegate.hpp
        CmdProcessorContext.hpp
        CmdProcessorContext.hpp
        ZMBusContext.hpp
)

find_package(cppzmq REQUIRED)
target_link_libraries(ZMCore INTERFACE cppzmq)
target_link_libraries(ZMCore INTERFACE RP_Core)
# target_link_libraries(ZMCore INTERFACE RP_Core_Strings)