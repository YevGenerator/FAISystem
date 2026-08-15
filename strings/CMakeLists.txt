add_library(RP_Core_Strings INTERFACE
        cmd_keywords.hpp
        CommandParser.hpp
        algoname_map.hpp
        utils.hpp
)

target_link_libraries(RP_Core_Strings INTERFACE RP_Core)

target_include_directories(RP_Core_Strings INTERFACE
        .
)