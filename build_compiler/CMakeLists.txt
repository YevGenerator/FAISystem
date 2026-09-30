add_executable(rpcompiler main_compiler.cpp
        config_compiler.hpp
        binary_writer.hpp
        text_writer.hpp
        binary_parser.hpp
        cli_base.hpp
)
add_executable(rpdecompiler main_decompiler.cpp
        config_decompiler.hpp
        binary_parser.hpp
        cli_base.hpp
)

target_link_options(rpcompiler PRIVATE -static)
target_link_options(rpdecompiler PRIVATE -static)

target_link_libraries(rpcompiler PRIVATE
        RP_Core_Strings
)

target_link_libraries(rpdecompiler PRIVATE
        RP_Core_Strings
)