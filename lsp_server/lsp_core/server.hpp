#pragma once

#include <atomic>
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <nlohmann/json.hpp>

#include "transport.hpp"
#include "json_rpc/dispatcher.hpp"
#include "lifecycle/server_capabilities.hpp"
#include "lifecycle/server_state.hpp"
#include "protocol/completion/completion_item.hpp"
#include "protocol/completion/completion_params.hpp"
#include "protocol/diagnostic/publish_diagnostics.hpp"
#include "protocol/documents/document_store.hpp"
#include "protocol/documents/document_events/document_close.hpp"

namespace NodeSystem::lsp
{
    class Server
    {
    public:
        using DiagnosticHandler = std::function<std::vector<protocol::Diagnostic>
            (const protocol::documents::DocumentItem&)>;

        using CompletionHandler = std::function<std::vector<protocol::CompletionItem>
            (const protocol::documents::DocumentItem& doc,
             protocol::Position position)>;

        using SemanticTokensHandler = std::function<std::vector<std::uint32_t>(const protocol::documents::DocumentItem&)>;

        Server()
        {
            setup_internal_handlers();
        }

        int run()
        {
            Transport::init_stdio();

            while (!is_exited_)
            {
                auto incoming = Transport::read_message();
                if (incoming) {
                    std::cerr << ">>> RECEIVED: " << *incoming << "\n";
                }
                if (!incoming.has_value())
                {
                    break;
                }

                auto response = dispatcher().dispatch(*incoming);
                if (response.has_value())
                {
                    Transport::write_message(*response);
                }
            }

            return was_shutdown_ ? 0 : 1;
        }

        void trigger_diagnostics(const std::string_view uri) const
        {
            const auto* doc = doc_store_.get_document(uri);
            if (!doc)
            {
                return;
            }

            std::vector<protocol::Diagnostic> diagnostics;

            if (diagnostic_handler_)
            {
                diagnostics = diagnostic_handler_(*doc);
            }

            publish_diagnostics(uri, diagnostics);
        }

        static void publish_diagnostics(const std::string_view uri, std::span<const protocol::Diagnostic> diagnostics)
        {
            protocol::PublishDiagnosticsParams params;
            params.uri = std::string(uri);
            params.diagnostics.assign(diagnostics.begin(), diagnostics.end());
            Transport::write_message(params.dump_notification());
        }

        [[nodiscard]]
        json_rpc::Dispatcher& dispatcher() noexcept
        {
            return dispatcher_;
        }

        [[nodiscard]]
        auto current_state() const noexcept
        {
            return state;
        }

        void set_diagnostic_handler(DiagnosticHandler handler)
        {
            diagnostic_handler_ = std::move(handler);
        }


        void set_completion_handler(CompletionHandler handler)
        {
            completion_handler_ = std::move(handler);
        }

        void set_semantic_tokens_handler(SemanticTokensHandler handler)
        {
            semantic_tokens_handler_ = std::move(handler);
        }

    private:
        json_rpc::Dispatcher dispatcher_;
        protocol::documents::DocumentStore doc_store_;
        DiagnosticHandler diagnostic_handler_{nullptr};
        CompletionHandler completion_handler_{nullptr};
        SemanticTokensHandler semantic_tokens_handler_{nullptr};
        std::string_view state = lifecycle::ServerState::Uninitialized;
        bool was_shutdown_{false};
        bool is_exited_{false};

        void register_init_request()
        {
            dispatcher_.register_request(methods::init, [this](const json_rpc::Request& req)
            {
                const lifecycle::InitializeResult result = lifecycle::InitializeResult::Default();
                state = lifecycle::ServerState::Initialized;
                return json_rpc::Response::make_success(req.id, result.to_json());
            });
        }

        void register_inited_notification()
        {
            dispatcher_.register_notification(methods::inited, [](const json_rpc::Notification&)
            {
                std::cerr << "[LSP Server] Initialized notification received. Server is active.\n";
            });
        }

        void register_shutdown_request()
        {
            dispatcher_.register_request(methods::shutdown, [this](const json_rpc::Request& req)
            {
                was_shutdown_ = true;
                state = lifecycle::ServerState::Shutdown;
                return json_rpc::Response::make_success(req.id, nullptr);
            });
        }

        void register_exit_notification()
        {
            dispatcher_.register_notification(methods::exit, [this](const json_rpc::Notification&)
            {
                state = lifecycle::ServerState::Exited;
                is_exited_ = true;
            });
        }

        void register_docOpen_notification()
        {
            dispatcher_.register_notification(methods::open, [this](const json_rpc::Notification& notif)
            {
                if (!notif.params.has_value()) return;

                if (auto params = protocol::documents::DidOpenTextDocumentParams::from_json(*notif.params))
                {
                    doc_store_.open_document(*params);
                    trigger_diagnostics(params->textDocument.uri);
                }
            });
        }

        void register_docChange_notification()
        {
            dispatcher_.register_notification(methods::change, [this](const json_rpc::Notification& notif)
            {
                if (!notif.params.has_value()) return;

                if (auto params = protocol::documents::DidChangeTextDocumentParams::from_json(*notif.params))
                {
                    if (doc_store_.update_document(*params))
                    {
                        trigger_diagnostics(params->uri);
                    }
                }
            });
        }

        void register_docClose_notification()
        {
            dispatcher_.register_notification(methods::close, [this](const json_rpc::Notification& notif)
            {
                if (!notif.params.has_value()) return;

                if (auto params = protocol::documents::DidCloseTextDocumentParams::from_json(*notif.params))
                {
                    doc_store_.close_document(params->uri);
                    publish_diagnostics(params->uri, {});
                }
            });
        }

        void register_completion_request()
        {
            dispatcher_.register_request(methods::completion, [this](const json_rpc::Request& req)
            {
                if (!req.params.has_value() || !completion_handler_)
                {
                    return json_rpc::Response::make_success(req.id, nlohmann::json::array());
                }

                auto params = protocol::CompletionParams::from_json(*req.params);
                if (!params.has_value())
                {
                    return json_rpc::Response::make_success(req.id, nlohmann::json::array());
                }

                const auto* doc = doc_store_.get_document(params->uri);
                if (!doc)
                {
                    return json_rpc::Response::make_success(req.id, nlohmann::json::array());
                }

                const auto items = completion_handler_(*doc, params->position);

                nlohmann::json items_json = nlohmann::json::array();
                for (const auto& item : items)
                {
                    items_json.push_back(item.to_json());
                }

                return json_rpc::Response::make_success(req.id, std::move(items_json));
            });
        }

        void register_semantic_tokens_request()
        {
            dispatcher_.register_request("textDocument/semanticTokens/full", [this](const json_rpc::Request& req) {
                if (!req.params.has_value() || !semantic_tokens_handler_)
                {
                    return json_rpc::Response::make_success(req.id, nlohmann::json{{"data", nlohmann::json::array()}});
                }

                const auto uri = (*req.params)["textDocument"].value("uri", "");
                const auto* doc = doc_store_.get_document(uri);
                if (!doc)
                {
                    return json_rpc::Response::make_success(req.id, nlohmann::json{{"data", nlohmann::json::array()}});
                }

                auto data = semantic_tokens_handler_(*doc);
                return json_rpc::Response::make_success(req.id, nlohmann::json{{"data", std::move(data)}});
            });
        }

        void setup_internal_handlers()
        {
            register_init_request();
            register_inited_notification();
            register_shutdown_request();
            register_exit_notification();
            register_docOpen_notification();
            register_docChange_notification();
            register_docClose_notification();
            register_completion_request();
            register_semantic_tokens_request();
        }
    };
} // namespace NodeSystem::lsp
