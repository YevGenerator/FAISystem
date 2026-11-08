#include "lsp_core/server.hpp"
#include "fai/analyzer.hpp"

int main()
{
    using namespace NodeSystem::lsp;

    Server server;

    server.set_diagnostic_handler([](const protocol::documents::DocumentItem& doc) {
        return fai::Analyzer::analyze(doc);
    });

    server.set_completion_handler([](const protocol::documents::DocumentItem& doc, protocol::Position pos) {
        return fai::Analyzer::complete(doc, pos);
    });

    server.set_semantic_tokens_handler([](const protocol::documents::DocumentItem& doc) {
        return fai::Analyzer::highlight(doc);
    });

    return server.run();
}