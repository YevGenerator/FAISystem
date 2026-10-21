add_library(RP_Core_Strings INTERFACE
        cmd_keywords.hpp
        command_parser.hpp
        algoname_map.hpp
        ParserContext.hpp
        convert/primitive/net_bool_converter.hpp
        convert/primitive/node_id_converter.hpp
        convert/primitive/numbers_converter.hpp
        convert/type_parser.hpp
        dto/dto.hpp
        convert/all_converters.hpp
        convert/primitive/algo_type_converter.hpp
        convert/util_quotes.hpp
        convert/collections/list_converter.hpp
        preparser.hpp
        convert/structs/field.hpp
        convert/structs/struct_converter.hpp
        convert/structs/struct_converter.hpp
        convert/structs/kg_run_converter.hpp
        convert/structs/scheme.hpp
        convert/structs/kg_ip_converter.hpp
        convert/primitive/ipv4_converter.hpp
        convert/structs/kg_sensor_converter.hpp
        convert/structs/kg_input_converter.hpp
        convert/structs/kg_output_converter.hpp
        convert/structs/kg_node_converter.hpp
        convert/structs/kg_bindfrom_converter.hpp
        convert/structs/kg_bindto_converter.hpp
        convert/structs/kg_bindsingle_converter.hpp
        convert/structs/kg_binddouble_converter.hpp
        convert/primitive/is_string_converter.hpp
        convert/structs/is_struct_converter.hpp
        dto/base_dto.hpp
        dto/base_dto_extractor.hpp
)

target_link_libraries(RP_Core_Strings INTERFACE RP_Core)

target_include_directories(RP_Core_Strings INTERFACE
        .
)