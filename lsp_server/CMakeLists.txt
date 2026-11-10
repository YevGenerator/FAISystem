find_package(nlohmann_json CONFIG REQUIRED)

add_executable(lsp
        main.cpp
        lsp_core/transport.hpp
        lsp_core/json_rpc/errors/error_codes.hpp
        lsp_core/json_rpc/errors/error_codes.hpp
        lsp_core/json_rpc/request.hpp
        lsp_core/json_rpc/field_extractor.hpp
        lsp_core/json_rpc/json_builder.hpp
        lsp_core/json_rpc/response.hpp
        lsp_core/json_rpc/notification.hpp
        lsp_core/json_rpc/dispatcher.hpp
        lsp_core/expected.hpp
        lsp_core/lifecycle/server_state.hpp
        lsp_core/lifecycle/server_capabilities.hpp
        lsp_core/lifecycle/server_state_list.hpp
        lsp_core/json_fields.hpp
        lsp_core/protocol/range.hpp
        lsp_core/protocol/diagnostic/diagnostic.hpp
        lsp_core/protocol/diagnostic/publish_diagnostics.hpp
        lsp_core/server.hpp
        lsp_core/methods.hpp
        lsp_core/protocol/documents/document_item.hpp
        lsp_core/protocol/documents/document_item.hpp
        lsp_core/protocol/documents/document_events/document_open.hpp
        lsp_core/protocol/documents/document_events/document_close.hpp
        lsp_core/protocol/documents/document_events/document_change.hpp
        lsp_core/protocol/documents/document_item.hpp
        lsp_core/protocol/documents/document_store.hpp
        lsp_core/rules/cursor.hpp
        lsp_core/rules/syntax_node.hpp
        lsp_core/rules/rule_result.hpp
        lsp_core/rules/rule.hpp
        lsp_core/rules/combinators/trivia.hpp
        lsp_core/rules/combinators/exact.hpp
        lsp_core/rules/combinators/satisfy.hpp
        lsp_core/rules/combinators/take_while.hpp
        lsp_core/rules/combinators/tag.hpp
        lsp_core/rules/combinators/sequence.hpp
        lsp_core/rules/combinators/choice.hpp
        lsp_core/rules/combinators/enclosed.hpp
        lsp_core/rules/combinators/repeat.hpp
        lsp_core/rules/combinators/predicate_rule.hpp
        lsp_core/protocol/completion/completion_item_kind.hpp
        lsp_core/protocol/completion/completion_item.hpp
        lsp_core/protocol/completion/completion_params.hpp
        fai/analyzer.hpp
        fai/syntax/let_declaration.hpp
        fai/types/uint8.hpp
        fai/syntax/signs.hpp
        fai/syntax/signs.hpp
        fai/syntax/keywords.hpp
        lsp_core/rules/combinators/expect.hpp
        fai/syntax/identifier.hpp
        fai/syntax/raw_value_token.hpp
        lsp_core/protocol/semantic_tokens/semantic_tokens.hpp
        fai/syntax/structs/struct_field_entry.hpp
)

target_link_libraries(lsp PRIVATE nlohmann_json::nlohmann_json)
if(MINGW)
    target_link_options(lsp PRIVATE -static -static-libgcc -static-libstdc++)
endif()

target_link_libraries(lsp PRIVATE RP_Core_Strings
)

