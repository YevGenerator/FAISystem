#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

#include "../protocol/position.hpp"
#include "../protocol/range.hpp"

namespace NodeSystem::lsp::rules
{
    struct CursorCheckpoint
    {
        std::size_t offset{0};
        protocol::Position position{0, 0};
    };

    class SourceCursor
    {
    public:
        constexpr explicit SourceCursor(std::string_view source) noexcept
            : source_(source)
        {
        }

        [[nodiscard]]
        constexpr bool is_eof() const noexcept
        {
            return offset_ >= source_.size();
        }

        [[nodiscard]]
        constexpr protocol::Position current_position() const noexcept
        {
            return current_pos_;
        }

        [[nodiscard]]
        constexpr std::size_t current_offset() const noexcept
        {
            return offset_;
        }

        [[nodiscard]]
        constexpr std::optional<char> peek(std::size_t lookahead = 0) const noexcept
        {
            const auto target = offset_ + lookahead;
            if (target >= source_.size())
            {
                return std::nullopt;
            }
            return source_[target];
        }

        [[nodiscard]]
        constexpr std::string_view remaining() const noexcept
        {
            if (is_eof())
            {
                return {};
            }
            return source_.substr(offset_);
        }

        [[nodiscard]]
        constexpr CursorCheckpoint mark() const noexcept
        {
            return CursorCheckpoint{
                .offset = offset_,
                .position = current_pos_
            };
        }

        constexpr void restore(const CursorCheckpoint& cp) noexcept
        {
            offset_ = cp.offset;
            current_pos_ = cp.position;
        }

        constexpr std::optional<char> advance() noexcept
        {
            if (is_eof())
            {
                return std::nullopt;
            }

            const char ch = source_[offset_];

            if (ch == '\r')
            {
                ++offset_;
                if (!is_eof() && source_[offset_] == '\n')
                {
                    ++offset_;
                }
                ++current_pos_.line;
                current_pos_.character = 0;
                return '\n';
            }

            if (ch == '\n')
            {
                ++offset_;
                ++current_pos_.line;
                current_pos_.character = 0;
                return '\n';
            }

            ++offset_;
            ++current_pos_.character;
            return ch;
        }

        [[nodiscard]] constexpr bool starts_with(std::string_view prefix) const noexcept
        {
            return remaining().starts_with(prefix);
        }

        constexpr bool consume_prefix(std::string_view prefix) noexcept
        {
            if (!starts_with(prefix))
            {
                return false;
            }

            const std::size_t target_offset = offset_ + prefix.size();
            while (offset_ < target_offset)
            {
                advance();
            }
            return true;
        }

        [[nodiscard]]
        constexpr protocol::Range range_since(const CursorCheckpoint& cp) const noexcept
        {
            return protocol::Range{
                .start = cp.position,
                .end = current_pos_
            };
        }

        [[nodiscard]] constexpr std::string_view slice_since(const CursorCheckpoint& cp) const noexcept
        {
            if (cp.offset > offset_)
            {
                return {};
            }
            return source_.substr(cp.offset, offset_ - cp.offset);
        }

    private:
        std::string_view source_;
        std::size_t offset_{0};
        protocol::Position current_pos_{0, 0};
    };
} // namespace NodeSystem::lsp::rules
