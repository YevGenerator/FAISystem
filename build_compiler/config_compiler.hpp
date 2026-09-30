#pragma once
#include <iostream>
#include "binary_writer.hpp"
#include "command_parser.hpp"
#include "ParserContext.hpp"

namespace NodeSystem::Compiler
{
    class ConfigCompiler
    {
    public:
        static bool compile(std::string_view source, std::ostream& out)
        {
            strings::ParserContext ctx;
            std::string_view stream = source;
            const BinaryWriter emitter{out};

            while (true)
            {
                strings::Preparser::consumeWhitespaceAndComments(stream);
                if (stream.empty())
                {
                    break;
                }

                std::string_view lookahead = stream;
                std::string_view firstToken = strings::Preparser::consumeNextToken(lookahead);

                if (firstToken == strings::Keywords::Cmd::Let)
                {
                    stream = lookahead;
                    if (!strings::CommandParser::parseLet(stream, ctx))
                    {
                        std::cerr << "[Error] Failed to parse 'let' statement!\n";
                        return false;
                    }
                    continue;
                }

                auto parsedCmd = strings::CommandParser::parseNextCommand(stream, ctx);
                if (!parsedCmd)
                {
                    std::cerr << "[Error] Syntax error or unknown command near: "
                              << stream.substr(0, std::min<size_t>(stream.size(), 40)) << "...\n";
                    return false;
                }

                std::visit(emitter, *parsedCmd);
            }

            return true;
        }
    };
}