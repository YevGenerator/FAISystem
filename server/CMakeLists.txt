add_executable(serverZMQ server.cpp
)

target_link_libraries(serverZMQ PRIVATE ZMCore)