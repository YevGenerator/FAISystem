add_executable(tests main.cpp
        calculate/2SNO/pipeline.hpp
        calculate/2SNO/writers/table_writer.hpp
        calculate/2SNO/algo_trace_record.hpp
        calculate/2SNO/writers/csv_writer.hpp
        calculate/2SNO/writers/markdown_writer.hpp
        calculate/2SNO/writers/text_writer.hpp
        calculate/2SNO/matrix_trace.hpp
        calculate/2SNO/writers/matrix_writers.hpp)

target_link_libraries(tests PRIVATE RP_Core)
target_include_directories(tests PRIVATE .)

add_executable(logging_smoke logging_smoke.cpp)
target_link_libraries(logging_smoke PRIVATE RP_Core)
add_test(NAME logging_smoke COMMAND logging_smoke)

add_executable(pipeline_smoke pipeline_smoke.cpp)
target_link_libraries(pipeline_smoke PRIVATE RP_Core)
add_test(NAME pipeline_smoke COMMAND pipeline_smoke)

add_test(NAME compiler_roundtrip
    COMMAND ${CMAKE_COMMAND}
        "-DCOMPILER=$<TARGET_FILE:rpcompiler>"
        "-DDECOMPILER=$<TARGET_FILE:rpdecompiler>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/basic.rpconfig"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/roundtrip.cmake"
)

add_executable(zmq_queue_smoke zmq_queue_smoke.cpp)
target_link_libraries(zmq_queue_smoke PRIVATE ZMCore)
add_test(NAME zmq_queue_smoke COMMAND zmq_queue_smoke)
