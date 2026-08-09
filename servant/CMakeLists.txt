add_executable(servantZMQ servant.cpp)
target_link_libraries(servantZMQ PRIVATE ZMCore)

add_custom_command(TARGET servantZMQ POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/build1
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/build2
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:servantZMQ> ${CMAKE_BINARY_DIR}/build1/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:servantZMQ> ${CMAKE_BINARY_DIR}/build2/
)