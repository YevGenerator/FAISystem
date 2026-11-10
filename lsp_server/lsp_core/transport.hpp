#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <optional>
#include <charconv>
#include <cstdint>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

namespace NodeSystem::lsp
{
    class Transport
    {
    public:
        static void init_stdio() noexcept
        {
#if defined(_WIN32)
            _setmode(_fileno(stdin), _O_BINARY);
            _setmode(_fileno(stdout), _O_BINARY);
#endif
            std::ios_base::sync_with_stdio(false);
            std::cin.tie(nullptr);
        }

        static std::optional<std::string> read_message(std::istream& in = std::cin)
        {
            std::size_t content_length = 0;
            bool found_content_length = false;

            while (true)
            {
                std::string line;
                if (!std::getline(in, line))
                {
                    return std::nullopt;
                }

                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }

                if (line.empty())
                {
                    break;
                }

                constexpr std::string_view cl_header = "Content-Length:";
                if (line.starts_with(cl_header))
                {
                    auto val_view = std::string_view(line).substr(cl_header.size());

                    while (!val_view.empty() && val_view.front() == ' ')
                    {
                        val_view.remove_prefix(1);
                    }

                    auto [ptr, ec] = std::from_chars(val_view.data(), val_view.data() + val_view.size(),
                                                     content_length);
                    if (ec == std::errc())
                    {
                        found_content_length = true;
                    }
                }
            }

            if (!found_content_length || content_length == 0)
            {
                return std::nullopt;
            }

            std::string body;
            body.resize(content_length);

            in.read(body.data(), static_cast<std::streamsize>(content_length));

            if (static_cast<std::size_t>(in.gcount()) != content_length)
            {
                return std::nullopt;
            }

            return body;
        }

        static bool write_message(std::string_view payload, std::ostream& out = std::cout)
        {
            out << "Content-Length: " << payload.size() << "\r\n\r\n" << payload;
            out.flush();
            return static_cast<bool>(out);
        }
    };
}
