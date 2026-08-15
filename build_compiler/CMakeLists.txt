cmake_minimum_required(VERSION 3.20)
project(RpConfigToolchain)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -O3)



add_executable(rpcompiler ConfigCompiler.cpp
        ConfigReader.hpp
        config_compiler.hpp
)
add_executable(rpdecompiler ConfigDecompiler.cpp
        ConfigReader.hpp
        config_decompiler.hpp
)

target_link_libraries(rpcompiler PRIVATE RP_Core_Strings)
target_link_libraries(rpdecompiler PRIVATE RP_Core_Strings)

target_link_options(rpcompiler PRIVATE -static -static-libgcc -static-libstdc++)
target_link_options(rpdecompiler PRIVATE -static -static-libgcc -static-libstdc++)
