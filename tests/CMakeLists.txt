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
