cmake_minimum_required(VERSION 3.20)
project(RpConfigLspServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -O3)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(rpconfig-lsp main.cpp)

# 5. Прив'язуємо бібліотеку до твого проєкту
# nlohmann_json::nlohmann_json - це офіційне ім'я цілі цієї бібліотеки
target_link_libraries(rpconfig-lsp PRIVATE nlohmann_json::nlohmann_json)
target_link_options(rpconfig-lsp PRIVATE -static -static-libgcc -static-libstdc++)
# Якщо в тебе ядро (ConfigTypes.hpp) лежить в іншій папці, не забудь додати:
# target_include_directories(rpconfig-lsp PRIVATE ../Path/To/Core)