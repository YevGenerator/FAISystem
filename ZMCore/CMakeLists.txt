add_library(ZMCore INTERFACE
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
)
target_include_directories(ZMCore INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

find_package(cppzmq REQUIRED)
target_link_libraries(ZMCore INTERFACE cppzmq cppzmq-static)
target_link_libraries(ZMCore INTERFACE RP_Core)
# target_link_libraries(ZMCore INTERFACE RP_Core_Strings)