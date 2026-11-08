#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "document_item.hpp"
#include "document_events/document_change.hpp"
#include "document_events/document_open.hpp"

namespace NodeSystem::lsp::protocol::documents
{
    class DocumentStore
    {
    public:
        void open_document(const DidOpenTextDocumentParams& params)
        {
            documents_[params.textDocument.uri] = params.textDocument;
        }

        bool update_document(const DidChangeTextDocumentParams& params)
        {
            const auto it = documents_.find(params.uri);
            if (it == documents_.end())
            {
                return false;
            }

            it->second.version = params.version;
            it->second.text = params.text;
            return true;
        }

        void close_document(const std::string_view uri)
        {
            documents_.erase(std::string(uri));
        }

        [[nodiscard]]
        const DocumentItem* get_document(const std::string_view uri) const noexcept
        {
            const auto it = documents_.find(std::string(uri));
            if (it != documents_.end())
            {
                return &it->second;
            }
            return nullptr;
        }

    private:
        std::unordered_map<std::string, DocumentItem> documents_;
    };
} // namespace NodeSystem::lsp
