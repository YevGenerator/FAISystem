#pragma once

#include <cctype>
#include <cstddef>
#include <string_view>
#include <vector>

namespace NodeSystem::strings {
    struct TextToken {
        std::string_view text;
        std::size_t start = 0;
        std::size_t end = 0;
    };

    class StringUtils {
    private:
        static constexpr std::string_view DefaultTrimChars = " \t\r\n";

    public:
        StringUtils() = delete;

        ~StringUtils() = delete;

        StringUtils(const StringUtils &) = delete;

        StringUtils &operator=(const StringUtils &) = delete;

        [[nodiscard]] static constexpr bool is_whitespace(const char ch) noexcept {
            return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
        }

        [[nodiscard]] static constexpr auto trim(const std::string_view s,
                                                 const std::string_view trim_chars = DefaultTrimChars) noexcept ->
            std::string_view {
            const auto start = s.find_first_not_of(trim_chars);
            if (start == std::string_view::npos) {
                return {};
            }
            const auto end = s.find_last_not_of(trim_chars);
            return s.substr(start, end - start + 1);
        }

        [[nodiscard]] static constexpr auto trim_left(
            const std::string_view s,
            const std::string_view trim_chars = DefaultTrimChars
        ) noexcept -> std::string_view {
            const auto start = s.find_first_not_of(trim_chars);
            if (start == std::string_view::npos) {
                return {};
            }
            return s.substr(start);
        }

        [[nodiscard]] static constexpr auto trim_right(
            const std::string_view s,
            const std::string_view trim_chars = DefaultTrimChars
        ) noexcept -> std::string_view {
            const auto end = s.find_last_not_of(trim_chars);
            if (end == std::string_view::npos) {
                return {};
            }
            return s.substr(0, end + 1);
        }

        [[nodiscard]] static constexpr auto unwrap(std::string_view s,
                                                   const char open = '<',
                                                   const char close = '>') noexcept -> std::string_view {
            s = trim(s);
            if (s.size() >= 2 && s.front() == open && s.back() == close) {
                return s.substr(1, s.size() - 2);
            }
            return s;
        }

        [[nodiscard]] static constexpr auto unquote(const std::string_view s) noexcept -> std::string_view {
            return unwrap(s, '"', '"');
        }

        [[nodiscard]] static constexpr bool is_identifier(const std::string_view text) noexcept {
            if (text.empty()) {
                return false;
            }

            const char first = text.front();
            const bool valid_first = (first >= 'a' && first <= 'z')
                                     || (first >= 'A' && first <= 'Z')
                                     || first == '_';
            if (!valid_first) {
                return false;
            }

            for (const char ch: text) {
                const bool is_letter = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
                const bool is_digit = (ch >= '0' && ch <= '9');
                if (!is_letter && !is_digit && ch != '_') {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] static auto strip_comments(
            const std::string_view line,
            const char comment_char = '#'
        ) noexcept -> std::string_view {
            bool in_quotes = false;
            bool escaped = false;

            for (std::size_t i = 0; i < line.size(); ++i) {
                const char ch = line[i];

                if (in_quotes) {
                    if (escaped) {
                        escaped = false;
                    } else if (ch == '\\') {
                        escaped = true;
                    } else if (ch == '"') {
                        in_quotes = false;
                    }
                    continue;
                }

                if (ch == '"') {
                    in_quotes = true;
                    escaped = false;
                } else if (ch == comment_char) {
                    return line.substr(0, i);
                }
            }
            return line;
        }

        [[nodiscard]] static auto split_lines(const std::string_view text) -> std::vector<std::string_view> {
            std::vector<std::string_view> lines;
            std::size_t line_start = 0;

            for (std::size_t i = 0; i < text.size(); ++i) {
                if (text[i] != '\n') {
                    continue;
                }

                std::size_t line_end = i;
                if (line_end > line_start && text[line_end - 1] == '\r') {
                    --line_end;
                }

                lines.emplace_back(text.substr(line_start, line_end - line_start));
                line_start = i + 1;
            }

            if (line_start < text.size()) {
                lines.emplace_back(text.substr(line_start));
            } else if (text.empty() || text.back() == '\n') {
                lines.emplace_back("");
            }

            return lines;
        }

        [[nodiscard]] static auto split_by_char(
            const std::string_view text,
            const char delimiter
        ) -> std::vector<std::string_view> {
            std::vector<std::string_view> tokens;
            std::size_t start = 0;

            while (start <= text.size()) {
                const auto pos = text.find(delimiter, start);
                if (pos == std::string_view::npos) {
                    tokens.emplace_back(text.substr(start));
                    break;
                }
                tokens.emplace_back(text.substr(start, pos - start));
                start = pos + 1;
            }
            return tokens;
        }

        [[nodiscard]] static auto split_tokens(const std::string_view line) -> std::vector<TextToken> {
            std::vector<TextToken> tokens;
            std::size_t i = 0;

            while (i < line.size()) {
                // Пропускаємо ведучі пробільні символи
                while (i < line.size() && is_whitespace(line[i])) {
                    ++i;
                }

                if (i >= line.size()) {
                    break;
                }

                const std::size_t start = i;
                int angle_depth = 0;
                int square_depth = 0;
                bool in_string = false;
                bool escaped = false;

                while (i < line.size()) {
                    const char ch = line[i];
                    if (!in_string && angle_depth == 0 && square_depth == 0 && is_whitespace(ch)) {
                        break;
                    }

                    if (in_string) {
                        if (escaped) {
                            escaped = false;
                        } else if (ch == '\\') {
                            escaped = true;
                        } else if (ch == '"') {
                            in_string = false;
                        }
                        ++i;
                        continue;
                    }

                    if (ch == '"') {
                        in_string = true;
                        escaped = false;
                    } else if (ch == '<') {
                        ++angle_depth;
                    } else if (ch == '>') {
                        if (angle_depth > 0) --angle_depth;
                    } else if (ch == '[') {
                        ++square_depth;
                    } else if (ch == ']') {
                        if (square_depth > 0) --square_depth;
                    }

                    ++i;
                }

                tokens.push_back(TextToken{
                    .text = line.substr(start, i - start),
                    .start = start,
                    .end = i
                });
            }

            return tokens;
        }

        [[nodiscard]] static constexpr bool is_closed_composite(const std::string_view token) noexcept {
            const auto trimmed = trim(token);
            if (trimmed.empty()) {
                return false;
            }
            if (trimmed.front() == '<') {
                return trimmed.back() == '>';
            }
            if (trimmed.front() == '[') {
                return trimmed.back() == ']';
            }
            return true;
        }

        [[nodiscard]] static constexpr bool is_unclosed_composite(const std::string_view token) noexcept {
            const auto trimmed = trim(token);
            if (trimmed.empty()) {
                return false;
            }
            if (trimmed.front() == '<') {
                return trimmed.back() != '>';
            }
            if (trimmed.front() == '[') {
                return trimmed.back() != ']';
            }
            return false;
        }
    };
} // namespace NodeSystem::strings
