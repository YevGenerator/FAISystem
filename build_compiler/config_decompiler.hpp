#pragma once
#include <ostream>

#include "binary_parser.hpp"
#include "text_writer.hpp"

namespace NodeSystem::Compiler
{
    class ConfigDecompiler
    {
    public:
        static bool decompile(const char* buffer, std::size_t size, std::ostream& out)
        {
            BinaryParser parser{buffer, size};
            const TextWriter emitter{out};

            while (auto parsedCmd = parser.nextCommand())
            {
                std::visit(emitter, *parsedCmd);
            }
            return true;
        }
    };
}