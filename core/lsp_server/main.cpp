
#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <utility>

#include <nlohmann/json.hpp>

#include "../Core/commands/CommandList.hpp"
#include "../Core/commands/CmdTypes.hpp"
#include "../Core/types.hpp"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

using json = nlohmann::json;

namespace CoreTypes = NodeSystem::Core::types;

enum class TokenType {
    Keyword,
    Type,
    Number,
    String,
    Comment,
    Error,
    Operator,
    Property,
    Variable
};

struct SemanticToken {
    int line = 0;
    int startChar = 0;
    int length = 0;
    TokenType type = TokenType::Error;
};

struct Diagnostic {
    int startLine = 0;
    int startChar = 0;
    int endLine = 0;
    int endChar = 0;
    std::string message;
    std::string severity = "error";
};

struct HoverInfo {
    int startLine = 0;
    int startChar = 0;
    int endLine = 0;
    int endChar = 0;
    std::string markdown;
};

enum class ValueType {
    UInt32,
    UInt16,
    UInt8,
    Bool,
    Double01,
    Direction,
    KgNodeId,
    AlgoType,
    KgOutput,
    KgInput,
    ListUInt32,
    ListKgInput,
    KgIP,
    KgRun,
    KgNode,
    KgSensor,
    KgBindExternal,
    KgBind,
    KgBindInput,
    ListKgBindInput,
};

struct VariableInfo {
    std::string name;
    ValueType type = ValueType::UInt32;
    std::uint64_t scalarValue = 0;
    std::uint32_t deviceIdValue = 0;
    std::size_t listCount = 0;
    std::string resolvedText;
    int declarationLine = 0;
    int nameStart = 0;
    int nameEnd = 0;
};

using VariableMap = std::map<std::string, VariableInfo, std::less<>>;

struct LogicalStatement {
    std::string text;
    std::vector<std::pair<int, int>> offsetMap;
    std::size_t startLine = 0;
    std::size_t endLine = 0;
};

struct DocumentState {
    std::string uri;
    std::string content;
    int version = 0;
    std::vector<std::string> lines;
    std::vector<SemanticToken> tokens;
    std::vector<Diagnostic> diagnostics;
    std::vector<HoverInfo> hovers;
    VariableMap variables;
    std::vector<LogicalStatement> statements;
    std::map<std::pair<std::uint32_t, std::uint64_t>, std::size_t> declaredNodes;
};

struct LineWord {
    std::string_view text;
    std::size_t start = 0;
    std::size_t end = 0;
};

struct StructuredCompletionState {
    std::array<bool, 2> assigned{false, false};
    std::size_t nextPositionalIndex = 0;
    bool positionalBlocked = false;
};

template<std::size_t N>
struct StructuredCompletionStateN {
    std::array<bool, N> assigned{};
    std::size_t nextPositionalIndex = 0;
    bool positionalBlocked = false;
};

struct CommandInfo {
    std::string_view keyword;
    std::string_view parameterName;
    std::string_view description;
    ValueType valueType = ValueType::UInt32;
};

struct TypeInfo {
    std::string_view keyword;
    std::string_view description;
    std::uint64_t maxValue = 0;
    ValueType valueType = ValueType::UInt32;
};

constexpr std::array<CommandInfo, 8> kAllCommands{{
    {
        CoreTypes::Keyword::Device,
        CoreTypes::Param::Device,
        "Top-level command. Sets device identifier.",
        ValueType::UInt32
    },
    {
        CoreTypes::Keyword::Workers,
        "workersAmount",
        "Top-level command. Sets workers count.",
        ValueType::UInt8
    },
    {
        CoreTypes::Keyword::Ip,
        "KgIp",
        "Top-level command. Sets IP address and port.",
        ValueType::KgIP
    },
    {
        CoreTypes::Keyword::Run,
        "KgRun",
        "Top-level command. Controls run state for a device.",
        ValueType::KgRun
    },
    {
        CoreTypes::Keyword::Node,
        "KgNode",
        "Top-level command. Creates a processing node.",
        ValueType::KgNode
    },
    {
        CoreTypes::Keyword::Sensor,
        "KgSensor",
        "Top-level command. Creates a sensor.",
        ValueType::KgSensor
    },
    {
        CoreTypes::Keyword::BindE,
        "KgBindExternal",
        "Top-level command. Binds node to external device.",
        ValueType::KgBindExternal
    },
    {
        CoreTypes::Keyword::Bind,
        "KgBind",
        "Top-level command. Binds node to inputs.",
        ValueType::KgBind
    }
}};

constexpr std::string_view kTypeDouble = "double";
constexpr std::string_view kTypeDirection = "Direction";
constexpr std::string_view kTypeKgNodeId = "KgNodeId";
constexpr std::string_view kTypeAlgo = "AlgoType";
constexpr std::string_view kTypeKgOutput = "KgOutput";
constexpr std::string_view kTypeKgInput = "KgInput";
constexpr std::string_view kTypeListUInt32 = "[]uint32";
constexpr std::string_view kTypeListKgInput = "[]KgInput";
constexpr std::string_view kTypeKgIp = "KgIp";
constexpr std::string_view kTypeKgRun = "KgRun";
constexpr std::string_view kTypeKgNode = "KgNode";
constexpr std::string_view kTypeKgSensor = "KgSensor";
constexpr std::string_view kTypeBool = "bool";
constexpr std::string_view kTypeKgBindExternal = "KgBindExternal";
constexpr std::string_view kTypeKgBind = "KgBind";
constexpr std::string_view kTypeKgBindInput = "KgBindInput";
constexpr std::string_view kTypeListKgBindInput = "[]KgBindInput";

constexpr std::array<TypeInfo, 20> kSupportedTypes{{
    { "uint32", "Unsigned 32-bit integer.", std::numeric_limits<CoreTypes::ID>::max(), ValueType::UInt32 },
    { "uint16", "Unsigned 16-bit integer.", std::numeric_limits<CoreTypes::PortInt>::max(), ValueType::UInt16 },
    { "uint8", "Unsigned 8-bit integer.", std::numeric_limits<CoreTypes::SmallInt>::max(), ValueType::UInt8 },
    { kTypeBool, "Boolean value: true/false or 1/0.", 1, ValueType::Bool },
    { kTypeDouble, "Double in range 0..1.", 0, ValueType::Double01 },
    { kTypeDirection, "Direction string: \"up\" or \"down\".", 0, ValueType::Direction },
    { kTypeKgNodeId, "Node identifier: level.index.", 0, ValueType::KgNodeId },
    { kTypeAlgo, "Algorithm name from Algoholic.", 0, ValueType::AlgoType },
    { kTypeKgOutput, "Output structure: <c=... a=...>.", 0, ValueType::KgOutput },
    { kTypeKgInput, "Input structure: <c_i=... a_i=... b_i=... v_i=... g_i=...>.", 0, ValueType::KgInput },
    { kTypeListUInt32, "List of uint32 values.", 0, ValueType::ListUInt32 },
    { kTypeListKgInput, "List of KgInput values.", 0, ValueType::ListKgInput },
    { kTypeKgIp, "IP address structure: <host=... port=...>. ", 0, ValueType::KgIP },
    { kTypeKgRun, "Run structure: <deviceId=... toRun=...>.", 0, ValueType::KgRun },
    { kTypeKgNode, "Node structure: <deviceId=... id=... algo=... output=... inputs=...>.", 0, ValueType::KgNode },
{ kTypeKgSensor, "Sensor structure: <deviceId=... id=... period=... toEmit=...>.", 0, ValueType::KgSensor },
    { kTypeKgBindExternal, "External Bind structure: <deviceId=... id=... to=...>.", 0, ValueType::KgBindExternal },
    { kTypeKgBind, "Bind structure: <deviceId=... id=... to=...>.", 0, ValueType::KgBind },
    { kTypeKgBindInput, "Bind Input structure: <deviceId=... id=... in=...>.", 0, ValueType::KgBindInput },
    { kTypeListKgBindInput, "List of KgBindInput values.", 0, ValueType::ListKgBindInput }
}};

class RPConfigLSP {
private:
    std::map<std::string, DocumentState> documents;
    const LogicalStatement* activeLogicalStatement = nullptr;

    static constexpr int kCompletionKindKeyword = 14;
    void log(const std::string& message) {
        std::cerr << "[LSP] " << message << std::endl;
    }

    static std::vector<std::string> splitLines(const std::string& text) {
        std::vector<std::string> lines;
        std::size_t lineStart = 0;

        for (std::size_t i = 0; i < text.size(); ++i) {
            if (text[i] != '\n') {
                continue;
            }

            std::size_t lineEnd = i;
            if (lineEnd > lineStart && text[lineEnd - 1] == '\r') {
                --lineEnd;
            }

            lines.emplace_back(text.substr(lineStart, lineEnd - lineStart));
            lineStart = i + 1;
        }

        if (lineStart < text.size()) {
            lines.emplace_back(text.substr(lineStart));
        } else if (text.empty() || text.back() == '\n') {
            lines.emplace_back("");
        }

        return lines;
    }

    static bool isWhitespace(const char ch) {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    static std::string_view trimView(const std::string_view text) {
        std::size_t start = 0;
        std::size_t end = text.size();

        while (start < end && isWhitespace(text[start])) {
            ++start;
        }
        while (end > start && isWhitespace(text[end - 1])) {
            --end;
        }

        return text.substr(start, end - start);
    }

    static std::vector<LineWord> splitWords(const std::string_view line) {
        std::vector<LineWord> words;
        std::size_t i = 0;

        while (i < line.size()) {
            while (i < line.size() && isWhitespace(line[i])) {
                ++i;
            }

            if (i >= line.size()) {
                break;
            }

            const std::size_t start = i;
            int angleDepth = 0;
            int squareDepth = 0;
            bool inString = false;
            bool escaped = false;

            while (i < line.size()) {
                const char ch = line[i];
                if (!inString && angleDepth == 0 && squareDepth == 0 && isWhitespace(ch)) {
                    break;
                }

                if (inString) {
                    if (escaped) {
                        escaped = false;
                    } else if (ch == '\\') {
                        escaped = true;
                    } else if (ch == '"') {
                        inString = false;
                    }
                    ++i;
                    continue;
                }

                if (ch == '"') {
                    inString = true;
                    escaped = false;
                } else if (ch == '<') {
                    ++angleDepth;
                } else if (ch == '>') {
                    angleDepth = std::max(angleDepth - 1, 0);
                } else if (ch == '[') {
                    ++squareDepth;
                } else if (ch == ']') {
                    squareDepth = std::max(squareDepth - 1, 0);
                }

                ++i;
            }

            words.push_back(LineWord{
                line.substr(start, i - start),
                start,
                i
            });
        }

        return words;
    }

    static const CommandInfo* findCommandInfo(const std::string_view keyword) {
        for (const auto& command : kAllCommands) {
            if (command.keyword == keyword) {
                return &command;
            }
        }
        return nullptr;
    }

    static const TypeInfo* findTypeInfoByKeyword(const std::string_view keyword) {
        const std::string_view normalizedKeyword = keyword == "KgIP"
            ? kTypeKgIp
            : keyword;

        for (const auto& type : kSupportedTypes) {
            if (type.keyword == normalizedKeyword) {
                return &type;
            }
        }
        return nullptr;
    }

    static const TypeInfo* findTypeInfoByValueType(const ValueType valueType) {
        for (const auto& type : kSupportedTypes) {
            if (type.valueType == valueType) {
                return &type;
            }
        }
        return nullptr;
    }

    static bool isIdentifier(const std::string_view text) {
        if (text.empty()) {
            return false;
        }

        const char first = text.front();
        const bool validFirst = (first >= 'a' && first <= 'z')
            || (first >= 'A' && first <= 'Z')
            || first == '_';
        if (!validFirst) {
            return false;
        }

        for (const char ch : text) {
            const bool isLetter = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
            const bool isDigit = ch >= '0' && ch <= '9';
            if (!isLetter && !isDigit && ch != '_') {
                return false;
            }
        }

        return true;
    }

    template<typename UInt>
    static bool parseUnsignedExact(const std::string_view text, UInt& value) {
        if (text.empty()) {
            return false;
        }

        UInt parsed{};
        const auto* begin = text.data();
        const auto* end = text.data() + text.size();
        const auto[ptr, ec] = std::from_chars(begin, end, parsed);

        if (ec != std::errc{} || ptr != end) {
            return false;
        }

        value = parsed;
        return true;
    }

    static bool parseBoolExact(const std::string_view text, bool& value) {
        if (text == "true" || text == "1") {
            value = true;
            return true;
        }

        if (text == "false" || text == "0") {
            value = false;
            return true;
        }

        return false;
    }

    static bool parseDouble01Exact(const std::string_view text, double& value) {
        if (text.empty()) {
            return false;
        }

        std::stringstream stream;
        stream << std::string(text);
        double parsed = 0.0;
        char tail = '\0';
        if (!(stream >> parsed) || (stream >> tail)) {
            return false;
        }

        if (parsed < 0.0 || parsed > 1.0) {
            return false;
        }

        value = parsed;
        return true;
    }

    static bool parseQuotedStringExact(const std::string_view text, std::string& value) {
        if (text.size() < 2 || text.front() != '"' || text.back() != '"') {
            return false;
        }

        value.assign(text.substr(1, text.size() - 2));
        return true;
    }

    static std::vector<std::string_view> getAlgoNames() {
        std::vector<std::string_view> names;
        names.reserve(NodeSystem::Core::Algoholic::count());
        for (std::size_t i = 0; i < NodeSystem::Core::Algoholic::count(); ++i) {
            names.push_back(NodeSystem::Core::Algoholic::algos[i].name);
        }
        return names;
    }

    static std::size_t utf8CodePointLength(const unsigned char leadByte) {
        if ((leadByte & 0b1000'0000) == 0) return 1;
        if ((leadByte & 0b1110'0000) == 0b1100'0000) return 2;
        if ((leadByte & 0b1111'0000) == 0b1110'0000) return 3;
        if ((leadByte & 0b1111'1000) == 0b1111'0000) return 4;
        return 1;
    }

    static int utf16UnitsForByteRange(const std::string& text, std::size_t byteCount) {
        int utf16Units = 0;
        std::size_t i = 0;
        const std::size_t safeLimit = std::min(byteCount, text.size());

        while (i < safeLimit) {
            const auto leadByte = static_cast<unsigned char>(text[i]);
            const std::size_t codePointLength = utf8CodePointLength(leadByte);

            utf16Units += codePointLength == 4 ? 2 : 1;
            i += std::min(codePointLength, safeLimit - i);
        }

        return utf16Units;
    }

    static int byteOffsetToUtf16(const std::string& text, const int byteOffset) {
        return utf16UnitsForByteRange(text, static_cast<std::size_t>(std::max(byteOffset, 0)));
    }

    static int byteRangeToUtf16Length(const std::string& text, const int startByte, const int endByte) {
        const int safeStart = std::max(startByte, 0);
        const int safeEnd = std::max(endByte, safeStart);
        return utf16UnitsForByteRange(text, static_cast<std::size_t>(safeEnd))
            - utf16UnitsForByteRange(text, static_cast<std::size_t>(safeStart));
    }

    static LogicalStatement buildLogicalStatement(const std::vector<std::string>& lines, const std::size_t startLine) {
        LogicalStatement statement;
        statement.startLine = startLine;
        statement.endLine = startLine;

        int angleDepth = 0;
        int squareDepth = 0;
        bool inString = false;
        bool escaped = false;

        auto appendCharacter = [&](const char ch, const int line, const int character) {
            statement.offsetMap.emplace_back(line, character);
            statement.text.push_back(ch);
        };

        for (std::size_t lineIndex = startLine; lineIndex < lines.size(); ++lineIndex) {
            if (lineIndex > startLine) {
                appendCharacter('\n', static_cast<int>(lineIndex - 1), static_cast<int>(lines[lineIndex - 1].size()));
            }

            const std::string& line = lines[lineIndex];
            for (std::size_t charIndex = 0; charIndex < line.size(); ++charIndex) {
                const char ch = line[charIndex];
                appendCharacter(ch, static_cast<int>(lineIndex), static_cast<int>(charIndex));

                if (inString) {
                    if (escaped) {
                        escaped = false;
                    } else if (ch == '\\') {
                        escaped = true;
                    } else if (ch == '"') {
                        inString = false;
                    }
                    continue;
                }

                if (ch == '"') {
                    inString = true;
                    escaped = false;
                } else if (ch == '<') {
                    ++angleDepth;
                } else if (ch == '>') {
                    angleDepth = std::max(angleDepth - 1, 0);
                } else if (ch == '[') {
                    ++squareDepth;
                } else if (ch == ']') {
                    squareDepth = std::max(squareDepth - 1, 0);
                }
            }

            statement.endLine = lineIndex;
            if (!inString && angleDepth == 0 && squareDepth == 0) {
                break;
            }
        }

        const std::size_t finalLine = statement.endLine;
        const int finalCharacter = finalLine < lines.size() ? static_cast<int>(lines[finalLine].size()) : 0;
        statement.offsetMap.emplace_back(static_cast<int>(finalLine), finalCharacter);
        return statement;
    }

    const LogicalStatement* findLogicalStatementForLine(const DocumentState& doc, const int targetLine) const {
        if (targetLine < 0 || targetLine >= static_cast<int>(doc.lines.size())) {
            return nullptr;
        }

        for (const auto& statement : doc.statements) {
            if (targetLine >= static_cast<int>(statement.startLine) && targetLine <= static_cast<int>(statement.endLine)) {
                return &statement;
            }
        }

        return nullptr;
    }

    static std::size_t logicalOffsetForPosition(const LogicalStatement& statement, const int line, const int character) {
        for (std::size_t offset = 0; offset < statement.offsetMap.size(); ++offset) {
            const auto[mappedLine, mappedCharacter] = statement.offsetMap[offset];
            if (mappedLine == line && mappedCharacter == character) {
                return offset;
            }
        }

        for (std::size_t offset = statement.offsetMap.size(); offset > 0; --offset) {
            const auto[mappedLine, mappedCharacter] = statement.offsetMap[offset - 1];
            if (mappedLine < line || (mappedLine == line && mappedCharacter <= character)) {
                return offset - 1;
            }
        }

        return 0;
    }

    std::pair<int, int> mapLogicalBoundary(
        const std::size_t offset,
        const std::size_t fallbackOffset,
        const bool preferEnd
    ) const {
        if (activeLogicalStatement == nullptr || activeLogicalStatement->offsetMap.empty()) {
            return {0, 0};
        }

        const auto clampOffset = [&](const std::size_t value) {
            return std::min(value, activeLogicalStatement->text.size());
        };

        const std::size_t safeOffset = clampOffset(offset);
        const std::size_t safeFallback = clampOffset(fallbackOffset);

        if (!preferEnd) {
            for (std::size_t index = safeOffset; index < activeLogicalStatement->text.size(); ++index) {
                if (activeLogicalStatement->text[index] == '\n') {
                    continue;
                }
                return activeLogicalStatement->offsetMap[index];
            }
            return activeLogicalStatement->offsetMap[safeFallback];
        }

        for (std::size_t index = safeOffset; index > 0; --index) {
            if (activeLogicalStatement->text[index - 1] == '\n') {
                continue;
            }
            const auto [line, character] = activeLogicalStatement->offsetMap[index - 1];
            return {line, character + 1};
        }

        return activeLogicalStatement->offsetMap[safeFallback];
    }

    static std::string makeVariableHoverMarkdown(const VariableInfo& variable) {
        const TypeInfo* typeInfo = findTypeInfoByValueType(variable.type);
        const std::string_view typeName = typeInfo != nullptr ? typeInfo->keyword : "unknown";

        std::ostringstream hover;
        hover << "```rpconfig\nlet " << typeName << ' ' << variable.name << " = " << variable.resolvedText << "\n```\n"
              << "Named value of type `" << typeName << "`.";
        return hover.str();
    }

    static std::string formatScalarValue(const ValueType valueType, const std::uint64_t value) {
        if (valueType == ValueType::Bool) {
            return value != 0 ? "true" : "false";
        }

        if (valueType == ValueType::Direction) {
            return value != 0 ? "\"down\"" : "\"up\"";
        }

        if (valueType == ValueType::KgNodeId) {
            return formatKgNodeId(
                static_cast<std::uint32_t>(value >> 32),
                static_cast<std::uint32_t>(value & 0xFFFF'FFFFULL)
            );
        }

        if (valueType == ValueType::AlgoType) {
            if (value < NodeSystem::Core::Algoholic::count()) {
                return "\"" + std::string(NodeSystem::Core::Algoholic::algos[static_cast<std::size_t>(value)].name) + "\"";
            }
            return "\"\"";
        }

        if (valueType == ValueType::Double01) {
            std::ostringstream stream;
            stream << (static_cast<double>(value) / 1000000.0);
            return stream.str();
        }

        return std::to_string(value);
    }

    void addScalarValueToken(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t start,
        const std::size_t end,
        const ValueType valueType,
        const std::string_view rawToken,
        const VariableInfo* resolvedVariable
    ) {
        if (resolvedVariable != nullptr) {
            addToken(doc, lineIndex, start, end, TokenType::Variable);
            return;
        }

        if (valueType == ValueType::AlgoType || valueType == ValueType::Direction) {
            addToken(doc, lineIndex, start, end, TokenType::String);
            return;
        }

        if (valueType == ValueType::Bool) {
            bool boolValue = false;
            if (parseBoolExact(rawToken, boolValue) && (rawToken == "true" || rawToken == "false")) {
                addToken(doc, lineIndex, start, end, TokenType::Keyword);
                return;
            }
        }

        addToken(doc, lineIndex, start, end, TokenType::Number);
    }

    bool resolveValueToken(
        const VariableMap& variables,
        const std::string_view token,
        const ValueType expectedType,
        std::uint64_t& resolvedValue,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        const TypeInfo* typeInfo = findTypeInfoByValueType(expectedType);
        if (typeInfo == nullptr) {
            errorMessage = "Internal error: unknown expected type.";
            return false;
        }

        if (expectedType == ValueType::Bool) {
            bool boolValue = false;
            if (parseBoolExact(token, boolValue)) {
                resolvedValue = boolValue ? 1 : 0;
                if (resolvedVariable != nullptr) {
                    *resolvedVariable = nullptr;
                }
                return true;
            }

            if (!isIdentifier(token)) {
                errorMessage = "Expected `bool` literal or variable name.";
                return false;
            }

            const auto variableIt = variables.find(token);
            if (variableIt == variables.end()) {
                errorMessage = "Unknown variable `" + std::string(token) + "`.";
                return false;
            }

            const VariableInfo& variable = variableIt->second;
            if (variable.type != ValueType::Bool) {
                const TypeInfo* actualType = findTypeInfoByValueType(variable.type);
                errorMessage = "Variable `" + variable.name + "` has type `"
                    + std::string(actualType != nullptr ? actualType->keyword : "unknown")
                    + "`, expected `bool`.";
                return false;
            }

            resolvedValue = variable.scalarValue;
            if (resolvedVariable != nullptr) {
                *resolvedVariable = &variable;
            }
            return true;
        }

        if (expectedType == ValueType::Double01) {
            double doubleValue = 0.0;
            if (parseDouble01Exact(token, doubleValue)) {
                resolvedValue = static_cast<std::uint64_t>(doubleValue * 1000000.0);
                if (resolvedVariable != nullptr) {
                    *resolvedVariable = nullptr;
                }
                return true;
            }

            if (!isIdentifier(token)) {
                errorMessage = "Expected `double` literal in range `0..1` or variable name.";
                return false;
            }

            const auto variableIt = variables.find(token);
            if (variableIt == variables.end()) {
                errorMessage = "Unknown variable `" + std::string(token) + "`.";
                return false;
            }

            const VariableInfo& variable = variableIt->second;
            if (variable.type != ValueType::Double01) {
                const TypeInfo* actualType = findTypeInfoByValueType(variable.type);
                errorMessage = "Variable `" + variable.name + "` has type `"
                    + std::string(actualType != nullptr ? actualType->keyword : "unknown")
                    + "`, expected `double`.";
                return false;
            }

            resolvedValue = variable.scalarValue;
            if (resolvedVariable != nullptr) {
                *resolvedVariable = &variable;
            }
            return true;
        }

        if (expectedType == ValueType::Direction) {
            std::string stringValue;
            if (parseQuotedStringExact(token, stringValue) && (stringValue == "up" || stringValue == "down")) {
                resolvedValue = stringValue == "down" ? 1 : 0;
                if (resolvedVariable != nullptr) {
                    *resolvedVariable = nullptr;
                }
                return true;
            }

            if (!isIdentifier(token)) {
                errorMessage = "Expected `\"up\"`, `\"down\"`, or variable name.";
                return false;
            }

            const auto variableIt = variables.find(token);
            if (variableIt == variables.end()) {
                errorMessage = "Unknown variable `" + std::string(token) + "`.";
                return false;
            }

            const VariableInfo& variable = variableIt->second;
            if (variable.type != ValueType::Direction) {
                const TypeInfo* actualType = findTypeInfoByValueType(variable.type);
                errorMessage = "Variable `" + variable.name + "` has type `"
                    + std::string(actualType != nullptr ? actualType->keyword : "unknown")
                    + "`, expected `Direction`.";
                return false;
            }

            resolvedValue = variable.scalarValue;
            if (resolvedVariable != nullptr) {
                *resolvedVariable = &variable;
            }
            return true;
        }

        if (expectedType == ValueType::AlgoType) {
            std::string stringValue;
            if (parseQuotedStringExact(token, stringValue)) {
                for (const auto algoName : getAlgoNames()) {
                    if (algoName == stringValue) {
                        resolvedValue = NodeSystem::Core::Algoholic::getIdByName(algoName);
                        if (resolvedVariable != nullptr) {
                            *resolvedVariable = nullptr;
                        }
                        return true;
                    }
                }

                errorMessage = "Unknown algorithm `" + stringValue + "`.";
                return false;
            }

            if (!isIdentifier(token)) {
                errorMessage = "Expected quoted algorithm name or variable name.";
                return false;
            }

            const auto variableIt = variables.find(token);
            if (variableIt == variables.end()) {
                errorMessage = "Unknown variable `" + std::string(token) + "`.";
                return false;
            }

            const VariableInfo& variable = variableIt->second;
            if (variable.type != ValueType::AlgoType) {
                const TypeInfo* actualType = findTypeInfoByValueType(variable.type);
                errorMessage = "Variable `" + variable.name + "` has type `"
                    + std::string(actualType != nullptr ? actualType->keyword : "unknown")
                    + "`, expected `AlgoType`.";
                return false;
            }

            resolvedValue = variable.scalarValue;
            if (resolvedVariable != nullptr) {
                *resolvedVariable = &variable;
            }
            return true;
        }

        if (expectedType == ValueType::KgNodeId) {
            std::string resolvedText;
            return resolveNodeIdToken(variables, token, resolvedValue, resolvedText, errorMessage, resolvedVariable);
        }

        std::uint64_t literalValue = 0;
        if (parseUnsignedExact(token, literalValue)) {
            if (literalValue > typeInfo->maxValue) {
                errorMessage = "Value exceeds allowed maximum `" + std::to_string(typeInfo->maxValue) + "`.";
                return false;
            }

            resolvedValue = literalValue;
            if (resolvedVariable != nullptr) {
                *resolvedVariable = nullptr;
            }
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected `" + std::string(typeInfo->keyword) + "` literal or variable name.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != expectedType) {
            const TypeInfo* actualType = findTypeInfoByValueType(variable.type);
            errorMessage = "Variable `" + variable.name + "` has type `"
                + std::string(actualType != nullptr ? actualType->keyword : "unknown")
                + "`, expected `" + std::string(typeInfo->keyword) + "`.";
            return false;
        }

        resolvedValue = variable.scalarValue;
        if (resolvedVariable != nullptr) {
            *resolvedVariable = &variable;
        }
        return true;
    }

    static std::string formatHost(std::uint32_t host) {
        std::ostringstream stream;
        stream << ((host >> 24) & 0xFF) << '.'
               << ((host >> 16) & 0xFF) << '.'
               << ((host >> 8) & 0xFF) << '.'
               << (host & 0xFF);
        return stream.str();
    }

    static std::string formatKgIp(std::uint32_t host, std::uint64_t port) {
        return "<host=" + formatHost(host) + " port=" + std::to_string(port) + ">";
    }

    static std::string formatKgRun(std::uint32_t deviceId, const bool toRun) {
        return "<deviceId=" + std::to_string(deviceId) + " toRun=" + std::string(toRun ? "true" : "false") + ">";
    }

    static std::string formatKgNodeId(const std::uint32_t level, const std::uint32_t index) {
        return std::to_string(level) + "." + std::to_string(index);
    }

    static std::string formatKgOutput(const std::uint64_t cValue, const std::uint64_t aValue) {
        return "<c=" + formatScalarValue(ValueType::Direction, cValue) + " a=" + formatScalarValue(ValueType::Double01, aValue) + ">";
    }

    static std::string formatKgInput(
        const std::uint64_t cValue,
        const std::uint64_t aValue,
        const std::uint64_t bValue,
        const std::uint64_t vValue,
        const std::uint64_t gValue
    ) {
        return "<c_i=" + formatScalarValue(ValueType::Direction, cValue)
            + " a_i=" + formatScalarValue(ValueType::Double01, aValue)
            + " b_i=" + std::to_string(bValue)
            + " v_i=" + formatScalarValue(ValueType::Double01, vValue)
            + " g_i=" + formatScalarValue(ValueType::Double01, gValue)
            + ">";
    }

    static std::string formatListUInt32(const std::vector<std::uint64_t>& values) {
        std::ostringstream stream;
        stream << '[';
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i != 0) {
                stream << ' ';
            }
            stream << values[i];
        }
        stream << ']';
        return stream.str();
    }

    static std::string formatKgNode(
        const std::uint32_t deviceId,
        const std::uint64_t nodeId,
        const std::uint64_t algoId,
        const std::string_view outputText,
        const std::string_view inputsText
    ) {
        return "<deviceId=" + std::to_string(deviceId)
            + " id=" + formatScalarValue(ValueType::KgNodeId, nodeId)
            + " algo=" + formatScalarValue(ValueType::AlgoType, algoId)
            + " output=" + std::string(outputText)
            + " inputs=" + std::string(inputsText)
            + ">";
    }

    static std::string formatKgSensor(const std::uint32_t deviceId, const std::uint32_t id, const std::uint64_t period, const std::uint64_t toEmit) {
        return "<deviceId=" + std::to_string(deviceId) + " id=" + std::to_string(id) +
               " period=" + std::to_string(period) + " toEmit=" + formatScalarValue(ValueType::Double01, toEmit) + ">";
    }

    static std::string formatKgBindExternal(const std::uint32_t deviceId, const std::uint64_t id, const std::uint32_t to) {
        return "<deviceId=" + std::to_string(deviceId) + " id=" + formatScalarValue(ValueType::KgNodeId, id) + " to=" + std::to_string(to) + ">";
    }

    static std::string formatKgBindInput(const std::uint32_t deviceId, const std::uint64_t id, const std::uint32_t in) {
        return "<deviceId=" + std::to_string(deviceId) + " id=" + formatScalarValue(ValueType::KgNodeId, id) + " in=" + std::to_string(in) + ">";
    }

    static std::string formatKgBind(const std::uint32_t deviceId, const std::uint64_t id, const std::string_view toText) {
        return "<deviceId=" + std::to_string(deviceId) + " id=" + formatScalarValue(ValueType::KgNodeId, id) + " to=" + std::string(toText) + ">";
    }

    bool analyzeScalarValueAt(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        const ValueType expectedType,
        std::uint64_t& resolvedValue
    ) {
        std::string errorMessage;
        const VariableInfo* resolvedVariable = nullptr;
        if (!resolveValueToken(variables, token, expectedType, resolvedValue, errorMessage, &resolvedVariable)) {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), errorMessage);
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }

        if (resolvedVariable != nullptr) {
            addHover(doc, lineIndex, valueStart, valueStart + token.size(), makeVariableHoverMarkdown(*resolvedVariable));
        }

        addScalarValueToken(doc, lineIndex, valueStart, valueStart + token.size(), expectedType, token, resolvedVariable);
        return true;
    }

    bool analyzeNodeIdAt(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        std::uint64_t& resolvedValue,
        std::string& resolvedText,
        std::pair<std::size_t, std::size_t>* idValueRange = nullptr
    ) {
        std::string errorMessage;
        const VariableInfo* resolvedVariable = nullptr;
        if (!resolveNodeIdToken(variables, token, resolvedValue, resolvedText, errorMessage, &resolvedVariable)) {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), errorMessage);
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }

        if (idValueRange != nullptr) {
            *idValueRange = {valueStart, valueStart + token.size()};
        }

        if (resolvedVariable != nullptr) {
            addHover(doc, lineIndex, valueStart, valueStart + token.size(), makeVariableHoverMarkdown(*resolvedVariable));
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Variable);
            return true;
        }

        const std::size_t dot = token.find('.');
        if (dot == std::string_view::npos) {
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Number);
            return true;
        }

        const std::string_view levelPart = token.substr(0, dot);
        const std::string_view indexPart = token.substr(dot + 1);
        std::uint64_t ignored = 0;

        analyzeScalarValueAt(doc, lineIndex, valueStart, levelPart, variables, ValueType::UInt32, ignored);
        addToken(doc, lineIndex, valueStart + dot, valueStart + dot + 1, TokenType::Operator);
        analyzeScalarValueAt(doc, lineIndex, valueStart + dot + 1, indexPart, variables, ValueType::UInt32, ignored);
        return true;
    }

    static void markStructuredFieldAssigned(StructuredCompletionState& state, const std::size_t fieldIndex) {
        if (fieldIndex >= state.assigned.size()) {
            return;
        }

        state.assigned[fieldIndex] = true;
        while (state.nextPositionalIndex < state.assigned.size() && state.assigned[state.nextPositionalIndex]) {
            ++state.nextPositionalIndex;
        }
    }

    bool resolveNodeIdToken(
        const VariableMap& variables,
        const std::string_view token,
        std::uint64_t& resolvedValue,
        std::string& resolvedText,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        const std::size_t dot = token.find('.');
        if (dot != std::string_view::npos) {
            const std::string_view levelPart = token.substr(0, dot);
            const std::string_view indexPart = token.substr(dot + 1);
            std::uint64_t level = 0;
            std::uint64_t index = 0;
            std::string partError;
            if (!resolveValueToken(variables, levelPart, ValueType::UInt32, level, partError)) {
                errorMessage = "Invalid KgNodeId level: " + partError;
                return false;
            }
            if (!resolveValueToken(variables, indexPart, ValueType::UInt32, index, partError)) {
                errorMessage = "Invalid KgNodeId index: " + partError;
                return false;
            }

            resolvedValue = (level << 32) | index;
            resolvedText = formatKgNodeId(static_cast<std::uint32_t>(level), static_cast<std::uint32_t>(index));
            if (resolvedVariable != nullptr) {
                *resolvedVariable = nullptr;
            }
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected `KgNodeId` literal or variable name.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgNodeId) {
            errorMessage = "Variable `" + variable.name + "` has incompatible type.";
            return false;
        }

        resolvedValue = variable.scalarValue;
        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) {
            *resolvedVariable = &variable;
        }
        return true;
    }

    bool parseHostExpression(
        const VariableMap& variables,
        const std::string_view expression,
        std::uint32_t& hostValue,
        std::string& errorMessage
    ) {
        std::array<std::string_view, 4> octets{};
        std::size_t octetIndex = 0;
        std::size_t start = 0;

        while (start <= expression.size()) {
            const std::size_t dot = expression.find('.', start);
            const std::size_t end = dot == std::string_view::npos ? expression.size() : dot;
            if (octetIndex >= octets.size() || end == start) {
                errorMessage = "Host must contain exactly four octets.";
                return false;
            }

            octets[octetIndex++] = expression.substr(start, end - start);
            if (dot == std::string_view::npos) {
                break;
            }
            start = dot + 1;
        }

        if (octetIndex != octets.size()) {
            errorMessage = "Host must contain exactly four octets.";
            return false;
        }

        hostValue = 0;
        for (const auto octet : octets) {
            std::uint64_t value = 0;
            std::string octetError;
            if (!resolveValueToken(variables, octet, ValueType::UInt8, value, octetError)) {
                errorMessage = "Invalid host octet `" + std::string(octet) + "`: " + octetError;
                return false;
            }

            hostValue = (hostValue << 8) | static_cast<std::uint32_t>(value);
        }

        return true;
    }

    bool resolveKgIpValueToken(
        const VariableMap& variables,
        const std::string_view token,
        std::string& resolvedText,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        if (!token.empty() && token.front() == '<' && token.back() == '>') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto fields = splitWords(body);

            bool positionalBlocked = false;
            std::array<bool, 2> assigned{false, false};
            std::uint32_t hostValue = 0;
            std::uint64_t portValue = 0;
            std::size_t nextPositionalIndex = 0;

            for (const auto& field : fields) {
                const std::size_t equals = field.text.find('=');
                const bool isNamed = equals != std::string_view::npos;
                std::size_t parameterIndex = 0;
                std::string_view parameterValue = field.text;

                if (isNamed) {
                    const std::string_view fieldName = field.text.substr(0, equals);
                    parameterValue = field.text.substr(equals + 1);

                    if (fieldName == CoreTypes::Param::Host) {
                        parameterIndex = 0;
                    } else if (fieldName == CoreTypes::Param::Port) {
                        parameterIndex = 1;
                    } else {
                        errorMessage = "Unknown KgIP field `" + std::string(fieldName) + "`.";
                        return false;
                    }

                    if (assigned[parameterIndex]) {
                        errorMessage = "Field `" + std::string(fieldName) + "` is specified more than once.";
                        return false;
                    }

                    if (parameterIndex > nextPositionalIndex) {
                        positionalBlocked = true;
                    }
                } else {
                    if (positionalBlocked) {
                        errorMessage = "Positional KgIP field cannot appear after out-of-order named fields.";
                        return false;
                    }

                    while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                        ++nextPositionalIndex;
                    }

                    if (nextPositionalIndex >= assigned.size()) {
                        errorMessage = "Too many positional fields for KgIP.";
                        return false;
                    }

                    parameterIndex = nextPositionalIndex;
                }

                if (parameterIndex == 0) {
                    if (!parseHostExpression(variables, parameterValue, hostValue, errorMessage)) {
                        return false;
                    }
                } else {
                    std::string portError;
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt16, portValue, portError)) {
                        errorMessage = "Invalid port value: " + portError;
                        return false;
                    }
                }

                assigned[parameterIndex] = true;
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }
            }

            resolvedText = formatKgIp(hostValue, portValue);
            if (resolvedVariable != nullptr) {
                *resolvedVariable = nullptr;
            }
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected KgIP variable or `<host=... port=...>` literal.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgIP) {
            errorMessage = "Variable `" + variable.name + "` has incompatible type.";
            return false;
        }

        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) {
            *resolvedVariable = &variable;
        }
        return true;
    }

    bool analyzeHostExpression(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t expressionStart,
        const std::string_view expression,
        const VariableMap& variables,
        std::uint32_t& hostValue
    ) {
        std::array<std::pair<std::size_t, std::size_t>, 4> octetRanges{};
        std::array<std::string_view, 4> octets{};
        std::size_t octetIndex = 0;
        std::size_t start = 0;

        while (start <= expression.size()) {
            const std::size_t dot = expression.find('.', start);
            const std::size_t end = dot == std::string_view::npos ? expression.size() : dot;
            if (octetIndex >= octets.size() || end == start) {
                addDiagnostic(doc, lineIndex, expressionStart, expressionStart + expression.size(), "Host must contain exactly four octets.");
                return false;
            }

            octets[octetIndex] = expression.substr(start, end - start);
            octetRanges[octetIndex] = {start, end};
            ++octetIndex;

            if (dot == std::string_view::npos) {
                break;
            }
            start = dot + 1;
        }

        if (octetIndex != octets.size()) {
            addDiagnostic(doc, lineIndex, expressionStart, expressionStart + expression.size(), "Host must contain exactly four octets.");
            return false;
        }

        hostValue = 0;
        for (std::size_t i = 0; i < octets.size(); ++i) {
            std::uint64_t value = 0;
            std::string octetError;
            const VariableInfo* resolvedVariable = nullptr;
            const auto [localStart, localEnd] = octetRanges[i];
            if (!resolveValueToken(variables, octets[i], ValueType::UInt8, value, octetError, &resolvedVariable)) {
                addDiagnostic(doc, lineIndex, expressionStart + localStart, expressionStart + localEnd, octetError);
                addToken(doc, lineIndex, expressionStart + localStart, expressionStart + localEnd, TokenType::Error);
                return false;
            }

            if (resolvedVariable != nullptr) {
                addHover(doc, lineIndex, expressionStart + localStart, expressionStart + localEnd, makeVariableHoverMarkdown(*resolvedVariable));
            }

            addScalarValueToken(doc, lineIndex, expressionStart + localStart, expressionStart + localEnd, ValueType::UInt8, octets[i], resolvedVariable);

            if (i + 1 < octets.size()) {
                addToken(doc, lineIndex, expressionStart + localEnd, expressionStart + localEnd + 1, TokenType::Operator);
            }

            hostValue = (hostValue << 8) | static_cast<std::uint32_t>(value);
        }

        return true;
    }

    bool analyzeKgIpInlineLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        std::string& resolvedText
    ) {
        if (token.empty() || token.front() != '<') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected KgIp variable or `<host=... port=...>` literal.");
            return false;
        }

        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);

        if (token.back() != '>') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `>` for KgIp literal.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }

        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto fields = splitWords(body);

        bool positionalBlocked = false;
        std::array<bool, 2> assigned{false, false};
        std::uint32_t hostValue = 0;
        std::uint64_t portValue = 0;
        std::size_t nextPositionalIndex = 0;

        for (const auto& field : fields) {
            const std::size_t fieldStart = valueStart + 1 + field.start;
            const std::size_t fieldEnd = valueStart + 1 + field.end;
            const std::size_t equals = field.text.find('=');
            const bool isNamed = equals != std::string_view::npos;
            std::size_t parameterIndex = 0;
            std::string_view parameterValue = field.text;
            std::size_t parameterValueStart = fieldStart;

            if (isNamed) {
                const std::string_view fieldName = field.text.substr(0, equals);
                parameterValue = field.text.substr(equals + 1);
                parameterValueStart = fieldStart + equals + 1;

                if (fieldName == CoreTypes::Param::Host) {
                    parameterIndex = 0;
                } else if (fieldName == CoreTypes::Param::Port) {
                    parameterIndex = 1;
                } else {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Unknown KgIp field `" + std::string(fieldName) + "`.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }

                addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Property);
                addToken(doc, lineIndex, fieldStart + equals, fieldStart + equals + 1, TokenType::Operator);

                if (assigned[parameterIndex]) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Field `" + std::string(fieldName) + "` is specified more than once.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }

                if (parameterIndex > nextPositionalIndex) {
                    positionalBlocked = true;
                }
            } else {
                if (positionalBlocked) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Positional KgIp field cannot appear after out-of-order named fields.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }

                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }

                if (nextPositionalIndex >= assigned.size()) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Too many positional fields for KgIp.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }

                parameterIndex = nextPositionalIndex;
            }

            if (parameterIndex == 0) {
                if (!analyzeHostExpression(doc, lineIndex, parameterValueStart, parameterValue, variables, hostValue)) {
                    return false;
                }
            } else {
                std::uint64_t value = 0;
                std::string portError;
                const VariableInfo* resolvedVariable = nullptr;
                if (!resolveValueToken(variables, parameterValue, ValueType::UInt16, value, portError, &resolvedVariable)) {
                    addDiagnostic(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), portError);
                    addToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), TokenType::Error);
                    return false;
                }

                if (resolvedVariable != nullptr) {
                    addHover(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), makeVariableHoverMarkdown(*resolvedVariable));
                }

                addScalarValueToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), ValueType::UInt16, parameterValue, resolvedVariable);

                portValue = value;
            }

            assigned[parameterIndex] = true;
            while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                ++nextPositionalIndex;
            }
        }

        resolvedText = formatKgIp(hostValue, portValue);
        return true;
    }

    bool resolveKgRunValueToken(
        const VariableMap& variables,
        const std::string_view token,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        if (!token.empty() && token.front() == '<' && token.back() == '>') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto fields = splitWords(body);

            bool positionalBlocked = false;
            std::array<bool, 2> assigned{false, false};
            std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
            std::uint64_t toRunValue = 0;
            std::size_t nextPositionalIndex = 0;

            for (const auto& field : fields) {
                const std::size_t equals = field.text.find('=');
                const bool isNamed = equals != std::string_view::npos;
                std::size_t parameterIndex = 0;
                std::string_view parameterValue = field.text;

                if (isNamed) {
                    const std::string_view fieldName = field.text.substr(0, equals);
                    parameterValue = field.text.substr(equals + 1);

                    if (fieldName == CoreTypes::Param::Device) {
                        parameterIndex = 0;
                    } else if (fieldName == CoreTypes::Param::ToRun) {
                        parameterIndex = 1;
                    } else {
                        errorMessage = "Unknown KgRun field `" + std::string(fieldName) + "`.";
                        return false;
                    }

                    if (assigned[parameterIndex]) {
                        errorMessage = "Field `" + std::string(fieldName) + "` is specified more than once.";
                        return false;
                    }

                    if (parameterIndex > nextPositionalIndex) {
                        positionalBlocked = true;
                    }
                } else {
                    if (positionalBlocked) {
                        errorMessage = "Positional KgRun field cannot appear after out-of-order named fields.";
                        return false;
                    }

                    while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                        ++nextPositionalIndex;
                    }

                    if (nextPositionalIndex >= assigned.size()) {
                        errorMessage = "Too many positional fields for KgRun.";
                        return false;
                    }

                    parameterIndex = nextPositionalIndex;
                }

                std::string fieldError;
                if (parameterIndex == 0) {
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, deviceIdValue, fieldError)) {
                        errorMessage = fieldError;
                        return false;
                    }
                } else {
                    if (!resolveValueToken(variables, parameterValue, ValueType::Bool, toRunValue, fieldError)) {
                        errorMessage = fieldError;
                        return false;
                    }
                }

                assigned[parameterIndex] = true;
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }
            }

            if (!assigned[0] && !defaultDeviceId.has_value()) {
                errorMessage = "Default `deviceId` is unavailable until `device` command is specified.";
                return false;
            }

            resolvedText = formatKgRun(static_cast<std::uint32_t>(deviceIdValue), toRunValue != 0);
            if (resolvedVariable != nullptr) {
                *resolvedVariable = nullptr;
            }
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected KgRun variable or `<deviceId=... toRun=...>` literal.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgRun) {
            errorMessage = "Variable `" + variable.name + "` has incompatible type.";
            return false;
        }

        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) {
            *resolvedVariable = &variable;
        }
        return true;
    }

    bool analyzeKgRunInlineLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText
    ) {
        if (token.empty() || token.front() != '<') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected KgRun variable or `<deviceId=... toRun=...>` literal.");
            return false;
        }

        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);

        if (token.back() != '>') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `>` for KgRun literal.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }

        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto fields = splitWords(body);

        bool positionalBlocked = false;
        std::array<bool, 2> assigned{false, false};
        std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
        std::uint64_t toRunValue = 0;
        std::size_t nextPositionalIndex = 0;

        for (const auto& field : fields) {
            const std::size_t fieldStart = valueStart + 1 + field.start;
            const std::size_t fieldEnd = valueStart + 1 + field.end;
            const std::size_t equals = field.text.find('=');
            const bool isNamed = equals != std::string_view::npos;
            std::size_t parameterIndex = 0;
            std::string_view parameterValue = field.text;
            std::size_t parameterValueStart = fieldStart;

            if (isNamed) {
                const std::string_view fieldName = field.text.substr(0, equals);
                parameterValue = field.text.substr(equals + 1);
                parameterValueStart = fieldStart + equals + 1;

                if (fieldName == CoreTypes::Param::Device) {
                    parameterIndex = 0;
                } else if (fieldName == CoreTypes::Param::ToRun) {
                    parameterIndex = 1;
                } else {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Unknown KgRun field `" + std::string(fieldName) + "`.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }

                addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Property);
                addToken(doc, lineIndex, fieldStart + equals, fieldStart + equals + 1, TokenType::Operator);

                if (assigned[parameterIndex]) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Field `" + std::string(fieldName) + "` is specified more than once.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }

                if (parameterIndex > nextPositionalIndex) {
                    positionalBlocked = true;
                }
            } else {
                if (positionalBlocked) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Positional KgRun field cannot appear after out-of-order named fields.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }

                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }

                if (nextPositionalIndex >= assigned.size()) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Too many positional fields for KgRun.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }

                parameterIndex = nextPositionalIndex;
            }

            std::uint64_t resolvedValue = 0;
            std::string valueError;
            const VariableInfo* resolvedVariable = nullptr;
            const ValueType expectedType = parameterIndex == 0 ? ValueType::UInt32 : ValueType::Bool;
            if (!resolveValueToken(variables, parameterValue, expectedType, resolvedValue, valueError, &resolvedVariable)) {
                addDiagnostic(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), valueError);
                addToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), TokenType::Error);
                return false;
            }

            if (resolvedVariable != nullptr) {
                addHover(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), makeVariableHoverMarkdown(*resolvedVariable));
            }

            addScalarValueToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), expectedType, parameterValue, resolvedVariable);

            if (parameterIndex == 0) {
                deviceIdValue = resolvedValue;
            } else {
                toRunValue = resolvedValue;
            }

            assigned[parameterIndex] = true;
            while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                ++nextPositionalIndex;
            }
        }

        if (!assigned[0] && !defaultDeviceId.has_value()) {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Default `deviceId` is unavailable until `device` command is specified.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }

        resolvedText = formatKgRun(static_cast<std::uint32_t>(deviceIdValue), toRunValue != 0);
        return true;
    }

    bool resolveKgOutputValueToken(
        const VariableMap& variables,
        const std::string_view token,
        std::string& resolvedText,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        if (!token.empty() && token.front() == '<' && token.back() == '>') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto fields = splitWords(body);
            bool positionalBlocked = false;
            std::array<bool, 2> assigned{false, false};
            std::uint64_t cValue = 0;
            std::uint64_t aValue = 0;
            std::size_t nextPositionalIndex = 0;

            for (const auto& field : fields) {
                const std::size_t equals = field.text.find('=');
                const bool isNamed = equals != std::string_view::npos;
                std::size_t parameterIndex = 0;
                std::string_view parameterValue = field.text;

                if (isNamed) {
                    const std::string_view fieldName = field.text.substr(0, equals);
                    parameterValue = field.text.substr(equals + 1);
                    if (fieldName == CoreTypes::Param::OutC) {
                        parameterIndex = 0;
                    } else if (fieldName == CoreTypes::Param::OutA) {
                        parameterIndex = 1;
                    } else {
                        errorMessage = "Unknown KgOutput field `" + std::string(fieldName) + "`.";
                        return false;
                    }
                    if (assigned[parameterIndex]) {
                        errorMessage = "Field `" + std::string(fieldName) + "` is specified more than once.";
                        return false;
                    }
                    if (parameterIndex > nextPositionalIndex) {
                        positionalBlocked = true;
                    }
                } else {
                    if (positionalBlocked) {
                        errorMessage = "Positional KgOutput field cannot appear after out-of-order named fields.";
                        return false;
                    }
                    while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                        ++nextPositionalIndex;
                    }
                    if (nextPositionalIndex >= assigned.size()) {
                        errorMessage = "Too many positional fields for KgOutput.";
                        return false;
                    }
                    parameterIndex = nextPositionalIndex;
                }

                std::uint64_t value = 0;
                const ValueType expectedType = parameterIndex == 0 ? ValueType::Direction : ValueType::Double01;
                if (!resolveValueToken(variables, parameterValue, expectedType, value, errorMessage)) {
                    return false;
                }

                if (parameterIndex == 0) {
                    cValue = value;
                } else {
                    aValue = value;
                }

                assigned[parameterIndex] = true;
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }
            }

            resolvedText = formatKgOutput(cValue, aValue);
            if (resolvedVariable != nullptr) {
                *resolvedVariable = nullptr;
            }
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected KgOutput variable or `<c=... a=...>` literal.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgOutput) {
            errorMessage = "Variable `" + variable.name + "` has incompatible type.";
            return false;
        }

        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) {
            *resolvedVariable = &variable;
        }
        return true;
    }

    bool analyzeKgOutputInlineLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        std::string& resolvedText
    ) {
        if (token.empty() || token.front() != '<') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected KgOutput literal.");
            return false;
        }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != '>') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `>` for KgOutput literal.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto fields = splitWords(body);
        bool positionalBlocked = false;
        std::array<bool, 2> assigned{false, false};
        std::uint64_t cValue = 0;
        std::uint64_t aValue = 0;
        std::size_t nextPositionalIndex = 0;

        for (const auto& field : fields) {
            const std::size_t fieldStart = valueStart + 1 + field.start;
            const std::size_t fieldEnd = valueStart + 1 + field.end;
            const std::size_t equals = field.text.find('=');
            const bool isNamed = equals != std::string_view::npos;
            std::size_t parameterIndex = 0;
            std::string_view parameterValue = field.text;
            std::size_t parameterValueStart = fieldStart;

            if (isNamed) {
                const std::string_view fieldName = field.text.substr(0, equals);
                parameterValue = field.text.substr(equals + 1);
                parameterValueStart = fieldStart + equals + 1;
                if (fieldName == CoreTypes::Param::OutC) {
                    parameterIndex = 0;
                } else if (fieldName == CoreTypes::Param::OutA) {
                    parameterIndex = 1;
                } else {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Unknown KgOutput field `" + std::string(fieldName) + "`.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }
                addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Property);
                addToken(doc, lineIndex, fieldStart + equals, fieldStart + equals + 1, TokenType::Operator);
                if (assigned[parameterIndex]) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Field `" + std::string(fieldName) + "` is specified more than once.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }
                if (parameterIndex > nextPositionalIndex) {
                    positionalBlocked = true;
                }
            } else {
                if (positionalBlocked) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Positional KgOutput field cannot appear after out-of-order named fields.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }
                if (nextPositionalIndex >= assigned.size()) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Too many positional fields for KgOutput.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }
                parameterIndex = nextPositionalIndex;
            }

            std::uint64_t value = 0;
            const ValueType expectedType = parameterIndex == 0 ? ValueType::Direction : ValueType::Double01;
            if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, expectedType, value)) {
                return false;
            }

            if (parameterIndex == 0) {
                cValue = value;
            } else {
                aValue = value;
            }

            assigned[parameterIndex] = true;
            while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                ++nextPositionalIndex;
            }
        }

        resolvedText = formatKgOutput(cValue, aValue);
        return true;
    }

    bool resolveKgInputValueToken(
        const VariableMap& variables,
        const std::string_view token,
        std::string& resolvedText,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        if (!token.empty() && token.front() == '<' && token.back() == '>') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto fields = splitWords(body);
            bool positionalBlocked = false;
            std::array<bool, 5> assigned{false, false, false, false, false};
            std::array<std::uint64_t, 5> values{0, 0, 0, 0, 0};
            std::size_t nextPositionalIndex = 0;
            const std::array<std::string_view, 5> fieldNames{
                CoreTypes::Param::InC, CoreTypes::Param::InA, CoreTypes::Param::InB, CoreTypes::Param::InV, CoreTypes::Param::InG
            };
            const std::array<ValueType, 5> fieldTypes{
                ValueType::Direction, ValueType::Double01, ValueType::UInt32, ValueType::Double01, ValueType::Double01
            };

            for (const auto& field : fields) {
                const std::size_t equals = field.text.find('=');
                const bool isNamed = equals != std::string_view::npos;
                std::size_t parameterIndex = 0;
                std::string_view parameterValue = field.text;
                if (isNamed) {
                    const std::string_view fieldName = field.text.substr(0, equals);
                    parameterValue = field.text.substr(equals + 1);
                    bool found = false;
                    for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                        if (fieldNames[index] == fieldName) {
                            parameterIndex = index;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        errorMessage = "Unknown KgInput field `" + std::string(fieldName) + "`.";
                        return false;
                    }
                    if (assigned[parameterIndex]) {
                        errorMessage = "Field `" + std::string(fieldName) + "` is specified more than once.";
                        return false;
                    }
                    if (parameterIndex > nextPositionalIndex) {
                        positionalBlocked = true;
                    }
                } else {
                    if (positionalBlocked) {
                        errorMessage = "Positional KgInput field cannot appear after out-of-order named fields.";
                        return false;
                    }
                    while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                        ++nextPositionalIndex;
                    }
                    if (nextPositionalIndex >= assigned.size()) {
                        errorMessage = "Too many positional fields for KgInput.";
                        return false;
                    }
                    parameterIndex = nextPositionalIndex;
                }

                if (!resolveValueToken(variables, parameterValue, fieldTypes[parameterIndex], values[parameterIndex], errorMessage)) {
                    return false;
                }

                assigned[parameterIndex] = true;
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }
            }

            resolvedText = formatKgInput(values[0], values[1], values[2], values[3], values[4]);
            if (resolvedVariable != nullptr) {
                *resolvedVariable = nullptr;
            }
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected KgInput variable or `<...>` literal.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgInput) {
            errorMessage = "Variable `" + variable.name + "` has incompatible type.";
            return false;
        }

        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) {
            *resolvedVariable = &variable;
        }
        return true;
    }

    bool analyzeKgInputInlineLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        std::string& resolvedText
    ) {
        if (token.empty() || token.front() != '<') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected KgInput literal.");
            return false;
        }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != '>') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `>` for KgInput literal.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto fields = splitWords(body);
        bool positionalBlocked = false;
        std::array<bool, 5> assigned{false, false, false, false, false};
        std::array<std::uint64_t, 5> values{0, 0, 0, 0, 0};
        std::size_t nextPositionalIndex = 0;
        const std::array<std::string_view, 5> fieldNames{
            CoreTypes::Param::InC, CoreTypes::Param::InA, CoreTypes::Param::InB, CoreTypes::Param::InV, CoreTypes::Param::InG
        };
        const std::array<ValueType, 5> fieldTypes{
            ValueType::Direction, ValueType::Double01, ValueType::UInt32, ValueType::Double01, ValueType::Double01
        };

        for (const auto& field : fields) {
            const std::size_t fieldStart = valueStart + 1 + field.start;
            const std::size_t fieldEnd = valueStart + 1 + field.end;
            const std::size_t equals = field.text.find('=');
            const bool isNamed = equals != std::string_view::npos;
            std::size_t parameterIndex = 0;
            std::string_view parameterValue = field.text;
            std::size_t parameterValueStart = fieldStart;

            if (isNamed) {
                const std::string_view fieldName = field.text.substr(0, equals);
                parameterValue = field.text.substr(equals + 1);
                parameterValueStart = fieldStart + equals + 1;
                bool found = false;
                for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                    if (fieldNames[index] == fieldName) {
                        parameterIndex = index;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Unknown KgInput field `" + std::string(fieldName) + "`.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }
                addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Property);
                addToken(doc, lineIndex, fieldStart + equals, fieldStart + equals + 1, TokenType::Operator);
                if (assigned[parameterIndex]) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Field `" + std::string(fieldName) + "` is specified more than once.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }
                if (parameterIndex > nextPositionalIndex) {
                    positionalBlocked = true;
                }
            } else {
                if (positionalBlocked) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Positional KgInput field cannot appear after out-of-order named fields.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }
                if (nextPositionalIndex >= assigned.size()) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Too many positional fields for KgInput.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }
                parameterIndex = nextPositionalIndex;
            }

            if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, fieldTypes[parameterIndex], values[parameterIndex])) {
                return false;
            }

            assigned[parameterIndex] = true;
            while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                ++nextPositionalIndex;
            }
        }

        resolvedText = formatKgInput(values[0], values[1], values[2], values[3], values[4]);
        return true;
    }

    bool resolveListKgInputValueToken(
        const VariableMap& variables,
        const std::string_view token,
        std::string& resolvedText,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr,
        std::size_t* outCount = nullptr
    ) {
        if (!token.empty() && token.front() == '[' && token.back() == ']') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto items = splitWords(body);
            if (outCount) *outCount = items.size();
            std::ostringstream stream;
            stream << '[';
            bool first = true;
            for (const auto& item : items) {
                std::string itemText;
                if (!resolveKgInputValueToken(variables, item.text, itemText, errorMessage)) {
                    return false;
                }
                if (!first) {
                    stream << ' ';
                }
                first = false;
                stream << itemText;
            }
            stream << ']';
            resolvedText = stream.str();
            if (resolvedVariable != nullptr) {
                *resolvedVariable = nullptr;
            }
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected `[]KgInput` variable or `[...]` literal.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::ListKgInput) {
            errorMessage = "Variable `" + variable.name + "` has incompatible type.";
            return false;
        }

        if (outCount) *outCount = variable.listCount;
        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) {
            *resolvedVariable = &variable;
        }
        return true;
    }

    bool analyzeListKgInputLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        std::string& resolvedText,
        std::size_t* outCount = nullptr
    ) {
        if (token.empty() || token.front() != '[') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected list literal `[ ... ]`.");
            return false;
        }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != ']') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `]` for list literal.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto items = splitWords(body);
        if (outCount) *outCount = items.size();
        std::ostringstream stream;
        stream << '[';
        bool first = true;
        for (const auto& item : items) {
            const std::size_t itemStart = valueStart + 1 + item.start;
            std::string itemText;
            const VariableInfo* resolvedVariable = nullptr;
            if (!item.text.empty() && item.text.front() == '<') {
                if (!analyzeKgInputInlineLiteral(doc, lineIndex, itemStart, item.text, variables, itemText)) {
                    return false;
                }
            } else {
                std::string errorMessage;
                if (!resolveKgInputValueToken(variables, item.text, itemText, errorMessage, &resolvedVariable)) {
                    addDiagnostic(doc, lineIndex, itemStart, itemStart + item.text.size(), errorMessage);
                    addToken(doc, lineIndex, itemStart, itemStart + item.text.size(), TokenType::Error);
                    return false;
                }
                if (resolvedVariable != nullptr) {
                    addHover(doc, lineIndex, itemStart, itemStart + item.text.size(), makeVariableHoverMarkdown(*resolvedVariable));
                    addToken(doc, lineIndex, itemStart, itemStart + item.text.size(), TokenType::Variable);
                }
            }

            if (!first) {
                stream << ' ';
            }
            first = false;
            stream << itemText;
        }
        stream << ']';
        resolvedText = stream.str();
        return true;
    }

    bool resolveListUInt32ValueToken(
        const VariableMap& variables,
        const std::string_view token,
        std::string& resolvedText,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr,
        std::size_t* outCount = nullptr
    ) {
        if (!token.empty() && token.front() == '[' && token.back() == ']') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto items = splitWords(body);
            if (outCount) *outCount = items.size();
            std::vector<std::uint64_t> values;
            values.reserve(items.size());

            for (const auto& item : items) {
                std::uint64_t value = 0;
                if (!resolveValueToken(variables, item.text, ValueType::UInt32, value, errorMessage)) {
                    return false;
                }
                values.push_back(value);
            }

            resolvedText = formatListUInt32(values);
            if (resolvedVariable != nullptr) {
                *resolvedVariable = nullptr;
            }
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected `[]uint32` variable or `[ ... ]` literal.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::ListUInt32) {
            errorMessage = "Variable `" + variable.name + "` has incompatible type.";
            return false;
        }

        if (outCount) *outCount = variable.listCount;
        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) {
            *resolvedVariable = &variable;
        }
        return true;
    }

    bool analyzeListUInt32Literal(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        std::string& resolvedText,
        std::size_t* outCount = nullptr
    ) {
        if (token.empty() || token.front() != '[') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected list literal `[ ... ]`.");
            return false;
        }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != ']') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `]` for list literal.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto items = splitWords(body);
        if (outCount) *outCount = items.size();
        std::vector<std::uint64_t> values;
        values.reserve(items.size());

        for (const auto& item : items) {
            const std::size_t itemStart = valueStart + 1 + item.start;
            std::uint64_t value = 0;
            if (!analyzeScalarValueAt(doc, lineIndex, itemStart, item.text, variables, ValueType::UInt32, value)) {
                return false;
            }
            values.push_back(value);
        }

        resolvedText = formatListUInt32(values);
        return true;
    }

    bool resolveKgNodeValueToken(
        const VariableMap& variables,
        const std::string_view token,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::uint32_t& deviceIdOut,
        std::uint64_t& nodeIdOut,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr,
        std::size_t* outInputsCount = nullptr
    ) {
        if (!token.empty() && token.front() == '<' && token.back() == '>') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto fields = splitWords(body);
            bool positionalBlocked = false;
            std::array<bool, 5> assigned{false, false, false, false, false};
            std::size_t nextPositionalIndex = 0;
            std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
            std::uint64_t parsedNodeIdValue = 0;
            std::uint64_t algoId = 0;
            std::string outputText = formatKgOutput(0, 0);
            std::string inputsText = "[]";
            const std::array<std::string_view, 5> fieldNames{
                CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::Algo, CoreTypes::Param::Output, CoreTypes::Param::Inputs
            };

            for (const auto& field : fields) {
                const std::size_t equals = field.text.find('=');
                const bool isNamed = equals != std::string_view::npos;
                std::size_t parameterIndex = 0;
                std::string_view parameterValue = field.text;

                if (isNamed) {
                    const std::string_view fieldName = field.text.substr(0, equals);
                    parameterValue = field.text.substr(equals + 1);
                    bool found = false;
                    for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                        if (fieldNames[index] == fieldName) {
                            parameterIndex = index;
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        errorMessage = "Unknown KgNode field `" + std::string(fieldName) + "`.";
                        return false;
                    }

                    if (assigned[parameterIndex]) {
                        errorMessage = "Field `" + std::string(fieldName) + "` is specified more than once.";
                        return false;
                    }

                    if (parameterIndex > nextPositionalIndex) {
                        positionalBlocked = true;
                    }
                } else {
                    if (positionalBlocked) {
                        errorMessage = "Positional KgNode field cannot appear after out-of-order named fields.";
                        return false;
                    }

                    while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                        ++nextPositionalIndex;
                    }

                    if (nextPositionalIndex >= assigned.size()) {
                        errorMessage = "Too many positional fields for KgNode.";
                        return false;
                    }

                    parameterIndex = nextPositionalIndex;
                }

                switch (parameterIndex) {
                    case 0:
                        if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, deviceIdValue, errorMessage)) {
                            errorMessage = "Invalid deviceId: " + errorMessage;
                            return false;
                        }
                        break;
                    case 1: {
                        std::string nodeIdText;
                        if (!resolveNodeIdToken(variables, parameterValue, parsedNodeIdValue, nodeIdText, errorMessage)) {
                            errorMessage = "Invalid node id: " + errorMessage;
                            return false;
                        }
                        break;
                    }
                    case 2:
                        if (!resolveValueToken(variables, parameterValue, ValueType::AlgoType, algoId, errorMessage)) {
                            errorMessage = "Invalid algorithm: " + errorMessage;
                            return false;
                        }
                        break;
                    case 3:
                        if (!resolveKgOutputValueToken(variables, parameterValue, outputText, errorMessage)) {
                            errorMessage = "Invalid output: " + errorMessage;
                            return false;
                        }
                        break;
                    case 4:
                        if (!resolveListKgInputValueToken(variables, parameterValue, inputsText, errorMessage, nullptr, outInputsCount)) {
                            errorMessage = "Invalid inputs: " + errorMessage;
                            return false;
                        }
                        break;
                    default:
                        break;
                }

                assigned[parameterIndex] = true;
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }
            }

            if (!assigned[0] && !defaultDeviceId.has_value()) {
                errorMessage = "Default `deviceId` is unavailable until `device` command is specified.";
                return false;
            }

            deviceIdOut = static_cast<std::uint32_t>(deviceIdValue);
            nodeIdOut = parsedNodeIdValue;
            resolvedText = formatKgNode(
                deviceIdOut,
                nodeIdOut,
                algoId,
                outputText,
                inputsText
            );
            if (resolvedVariable != nullptr) {
                *resolvedVariable = nullptr;
            }
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected KgNode variable or `<...>` literal.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgNode) {
            errorMessage = "Variable `" + variable.name + "` has incompatible type.";
            return false;
        }

        deviceIdOut = variable.deviceIdValue;
        nodeIdOut = variable.scalarValue;
        if (outInputsCount) *outInputsCount = variable.listCount;
        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) {
            *resolvedVariable = &variable;
        }
        return true;
    }

    bool analyzeKgNodeInlineLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::uint32_t& deviceIdOut,
        std::uint64_t& nodeIdOut,
        std::pair<std::size_t, std::size_t>* idValueRange = nullptr,
        std::size_t* outInputsCount = nullptr
    ) {
        if (token.empty() || token.front() != '<') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected KgNode literal.");
            return false;
        }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != '>') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `>` for KgNode literal.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto fields = splitWords(body);
        bool positionalBlocked = false;
        std::array<bool, 5> assigned{false, false, false, false, false};
        std::size_t nextPositionalIndex = 0;
        std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
        std::uint64_t parsedNodeIdValue = 0;
        std::uint64_t algoId = 0;
        std::string outputText = formatKgOutput(0, 0);
        std::string inputsText = "[]";
        const std::array<std::string_view, 5> fieldNames{
            CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::Algo, CoreTypes::Param::Output, CoreTypes::Param::Inputs
        };

        for (const auto& field : fields) {
            const std::size_t fieldStart = valueStart + 1 + field.start;
            const std::size_t fieldEnd = valueStart + 1 + field.end;
            const std::size_t equals = field.text.find('=');
            const bool isNamed = equals != std::string_view::npos;
            std::size_t parameterIndex = 0;
            std::string_view parameterValue = field.text;
            std::size_t parameterValueStart = fieldStart;

            if (isNamed) {
                const std::string_view fieldName = field.text.substr(0, equals);
                parameterValue = field.text.substr(equals + 1);
                parameterValueStart = fieldStart + equals + 1;

                bool found = false;
                for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                    if (fieldNames[index] == fieldName) {
                        parameterIndex = index;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Unknown KgNode field `" + std::string(fieldName) + "`.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }

                addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Property);
                addToken(doc, lineIndex, fieldStart + equals, fieldStart + equals + 1, TokenType::Operator);

                if (assigned[parameterIndex]) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Field `" + std::string(fieldName) + "` is specified more than once.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }

                if (parameterIndex > nextPositionalIndex) {
                    positionalBlocked = true;
                }
            } else {
                if (positionalBlocked) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Positional KgNode field cannot appear after out-of-order named fields.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }

                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }

                if (nextPositionalIndex >= assigned.size()) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Too many positional fields for KgNode.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }

                parameterIndex = nextPositionalIndex;
            }

            switch (parameterIndex) {
                case 0:
                    if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::UInt32, deviceIdValue)) {
                        return false;
                    }
                    break;
                case 1: {
                    std::string nodeIdText;
                    if (!analyzeNodeIdAt(doc, lineIndex, parameterValueStart, parameterValue, variables, parsedNodeIdValue, nodeIdText, idValueRange)) {
                        return false;
                    }
                    break;
                }
                case 2:
                    if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::AlgoType, algoId)) {
                        return false;
                    }
                    break;
                case 3:
                    if (!parameterValue.empty() && parameterValue.front() == '<') {
                        if (!analyzeKgOutputInlineLiteral(doc, lineIndex, parameterValueStart, parameterValue, variables, outputText)) {
                            return false;
                        }
                    } else {
                        std::string errorMessage;
                        const VariableInfo* resolvedVariable = nullptr;
                        if (!resolveKgOutputValueToken(variables, parameterValue, outputText, errorMessage, &resolvedVariable)) {
                            addDiagnostic(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), errorMessage);
                            addToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), TokenType::Error);
                            return false;
                        }
                        if (resolvedVariable != nullptr) {
                            addHover(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), makeVariableHoverMarkdown(*resolvedVariable));
                            addToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), TokenType::Variable);
                        }
                    }
                    break;
                case 4:
                    if (!parameterValue.empty() && parameterValue.front() == '[') {
                        if (!analyzeListKgInputLiteral(doc, lineIndex, parameterValueStart, parameterValue, variables, inputsText, outInputsCount)) {
                            return false;
                        }
                    } else {
                        std::string errorMessage;
                        const VariableInfo* resolvedVariable = nullptr;
                        if (!resolveListKgInputValueToken(variables, parameterValue, inputsText, errorMessage, &resolvedVariable, outInputsCount)) {
                            addDiagnostic(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), errorMessage);
                            addToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), TokenType::Error);
                            return false;
                        }
                        if (resolvedVariable != nullptr) {
                            addHover(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), makeVariableHoverMarkdown(*resolvedVariable));
                            addToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), TokenType::Variable);
                        }
                    }
                    break;
                default:
                    break;
            }

            assigned[parameterIndex] = true;
            while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                ++nextPositionalIndex;
            }
        }

        if (!assigned[0] && !defaultDeviceId.has_value()) {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Default `deviceId` is unavailable until `device` command is specified.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }

        deviceIdOut = static_cast<std::uint32_t>(deviceIdValue);
        nodeIdOut = parsedNodeIdValue;
        resolvedText = formatKgNode(
            deviceIdOut,
            nodeIdOut,
            algoId,
            outputText,
            inputsText
        );
        return true;
    }

    bool resolveKgSensorValueToken(
        const VariableMap& variables,
        const std::string_view token,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::uint32_t& deviceIdOut,
        std::uint32_t& idOut,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        if (!token.empty() && token.front() == '<' && token.back() == '>') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto fields = splitWords(body);
            bool positionalBlocked = false;
            std::array<bool, 4> assigned{false, false, false, false};
            std::size_t nextPositionalIndex = 0;
            std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
            std::uint64_t idValue = 0;
            std::uint64_t periodValue = 200;      // Дефолтне значення period
            std::uint64_t toEmitValue = 500000;   // Дефолтне значення toEmit (0.5 * 1000000)
            const std::array<std::string_view, 4> fieldNames{"deviceId", "id", "period", "toEmit"};

            for (const auto& field : fields) {
                const std::size_t equals = field.text.find('=');
                const bool isNamed = equals != std::string_view::npos;
                std::size_t parameterIndex = 0;
                std::string_view parameterValue = field.text;

                if (isNamed) {
                    const std::string_view fieldName = field.text.substr(0, equals);
                    parameterValue = field.text.substr(equals + 1);

                    bool found = false;
                    for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                        if (fieldNames[index] == fieldName) {
                            parameterIndex = index;
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        errorMessage = "Unknown KgSensor field `" + std::string(fieldName) + "`.";
                        return false;
                    }

                    if (assigned[parameterIndex]) {
                        errorMessage = "Field `" + std::string(fieldName) + "` is specified more than once.";
                        return false;
                    }

                    if (parameterIndex > nextPositionalIndex) {
                        positionalBlocked = true;
                    }
                } else {
                    if (positionalBlocked) {
                        errorMessage = "Positional KgSensor field cannot appear after out-of-order named fields.";
                        return false;
                    }

                    while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                        ++nextPositionalIndex;
                    }

                    if (nextPositionalIndex >= assigned.size()) {
                        errorMessage = "Too many positional fields for KgSensor.";
                        return false;
                    }

                    parameterIndex = nextPositionalIndex;
                }

                if (parameterIndex == 0) {
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, deviceIdValue, errorMessage)) return false;
                } else if (parameterIndex == 1) {
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, idValue, errorMessage)) return false;
                } else if (parameterIndex == 2) {
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, periodValue, errorMessage)) return false;
                } else if (parameterIndex == 3) {
                    if (!resolveValueToken(variables, parameterValue, ValueType::Double01, toEmitValue, errorMessage)) return false;
                }

                assigned[parameterIndex] = true;
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }
            }

            if (!assigned[0] && !defaultDeviceId.has_value()) {
                errorMessage = "Default `deviceId` is unavailable until `device` command is specified.";
                return false;
            }

            deviceIdOut = static_cast<std::uint32_t>(deviceIdValue);
            idOut = static_cast<std::uint32_t>(idValue);
            resolvedText = formatKgSensor(deviceIdOut, idOut, periodValue, toEmitValue);
            if (resolvedVariable != nullptr) *resolvedVariable = nullptr;
            return true;
        }

        if (!isIdentifier(token)) {
            errorMessage = "Expected KgSensor variable or `<deviceId=... id=... period=... toEmit=...>` literal.";
            return false;
        }

        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) {
            errorMessage = "Unknown variable `" + std::string(token) + "`.";
            return false;
        }

        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgSensor) {
            errorMessage = "Variable `" + variable.name + "` has incompatible type.";
            return false;
        }

        deviceIdOut = variable.deviceIdValue;
        idOut = static_cast<std::uint32_t>(variable.scalarValue);
        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) *resolvedVariable = &variable;
        return true;
    }

    bool analyzeKgSensorInlineLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::uint32_t& deviceIdOut,
        std::uint32_t& idOut,
        std::pair<std::size_t, std::size_t>* idValueRange = nullptr
    ) {
        if (token.empty() || token.front() != '<') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected KgSensor literal.");
            return false;
        }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != '>') {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `>` for KgSensor literal.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto fields = splitWords(body);
        bool positionalBlocked = false;
        std::array<bool, 4> assigned{false, false, false, false};
        std::size_t nextPositionalIndex = 0;
        std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
        std::uint64_t idValue = 0;
        std::uint64_t periodValue = 200;      // Дефолтне значення
        std::uint64_t toEmitValue = 500000;   // Дефолтне значення (0.5)
        const std::array<std::string_view, 4> fieldNames{"deviceId", "id", "period", "toEmit"};

        for (const auto& field : fields) {
            const std::size_t fieldStart = valueStart + 1 + field.start;
            const std::size_t fieldEnd = valueStart + 1 + field.end;
            const std::size_t equals = field.text.find('=');
            const bool isNamed = equals != std::string_view::npos;
            std::size_t parameterIndex = 0;
            std::string_view parameterValue = field.text;
            std::size_t parameterValueStart = fieldStart;

            if (isNamed) {
                const std::string_view fieldName = field.text.substr(0, equals);
                parameterValue = field.text.substr(equals + 1);
                parameterValueStart = fieldStart + equals + 1;

                bool found = false;
                for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                    if (fieldNames[index] == fieldName) {
                        parameterIndex = index;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Unknown KgSensor field `" + std::string(fieldName) + "`.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }

                addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Property);
                addToken(doc, lineIndex, fieldStart + equals, fieldStart + equals + 1, TokenType::Operator);

                if (assigned[parameterIndex]) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Field `" + std::string(fieldName) + "` is specified more than once.");
                    addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error);
                    return false;
                }

                if (parameterIndex > nextPositionalIndex) {
                    positionalBlocked = true;
                }
            } else {
                if (positionalBlocked) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Positional KgSensor field cannot appear after out-of-order named fields.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }

                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                    ++nextPositionalIndex;
                }

                if (nextPositionalIndex >= assigned.size()) {
                    addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Too many positional fields for KgSensor.");
                    addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error);
                    return false;
                }

                parameterIndex = nextPositionalIndex;
            }

            if (parameterIndex == 0) {
                if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::UInt32, deviceIdValue)) return false;
            } else if (parameterIndex == 1) {
                if (idValueRange != nullptr) *idValueRange = {parameterValueStart, parameterValueStart + parameterValue.size()};
                if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::UInt32, idValue)) return false;
            } else if (parameterIndex == 2) {
                if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::UInt32, periodValue)) return false;
            } else if (parameterIndex == 3) {
                if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::Double01, toEmitValue)) return false;
            }

            assigned[parameterIndex] = true;
            while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) {
                ++nextPositionalIndex;
            }
        }

        if (!assigned[0] && !defaultDeviceId.has_value()) {
            addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Default `deviceId` is unavailable until `device` command is specified.");
            addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error);
            return false;
        }

        deviceIdOut = static_cast<std::uint32_t>(deviceIdValue);
        idOut = static_cast<std::uint32_t>(idValue);
        resolvedText = formatKgSensor(deviceIdOut, idOut, periodValue, toEmitValue);
        return true;
    }

    // KgBindExternal parsers
    bool resolveKgBindExternalValueToken(
        const VariableMap& variables,
        const std::string_view token,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::uint32_t& deviceIdOut,
        std::uint64_t& nodeIdOut,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        if (!token.empty() && token.front() == '<' && token.back() == '>') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto fields = splitWords(body);
            bool positionalBlocked = false;
            std::array<bool, 3> assigned{false, false, false};
            std::size_t nextPositionalIndex = 0;
            std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
            std::uint64_t idValue = 0;
            std::uint64_t toValue = 0;
            const std::array<std::string_view, 3> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::To};

            for (const auto& field : fields) {
                const std::size_t equals = field.text.find('=');
                const bool isNamed = equals != std::string_view::npos;
                std::size_t parameterIndex = 0;
                std::string_view parameterValue = field.text;

                if (isNamed) {
                    const std::string_view fieldName = field.text.substr(0, equals);
                    parameterValue = field.text.substr(equals + 1);
                    bool found = false;
                    for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                        if (fieldNames[index] == fieldName) {
                            parameterIndex = index;
                            found = true;
                            break;
                        }
                    }
                    if (!found) { errorMessage = "Unknown KgBindExternal field `" + std::string(fieldName) + "`."; return false; }
                    if (assigned[parameterIndex]) { errorMessage = "Field `" + std::string(fieldName) + "` is specified more than once."; return false; }
                    if (parameterIndex > nextPositionalIndex) positionalBlocked = true;
                } else {
                    if (positionalBlocked) { errorMessage = "Positional KgBindExternal field cannot appear after out-of-order named fields."; return false; }
                    while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
                    if (nextPositionalIndex >= assigned.size()) { errorMessage = "Too many positional fields for KgBindExternal."; return false; }
                    parameterIndex = nextPositionalIndex;
                }

                if (parameterIndex == 0) {
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, deviceIdValue, errorMessage)) return false;
                } else if (parameterIndex == 1) {
                    std::string tmpStr;
                    if (!resolveNodeIdToken(variables, parameterValue, idValue, tmpStr, errorMessage)) return false;
                } else {
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, toValue, errorMessage)) return false;
                }

                assigned[parameterIndex] = true;
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
            }

            if (!assigned[0] && !defaultDeviceId.has_value()) { errorMessage = "Default `deviceId` is unavailable."; return false; }

            deviceIdOut = static_cast<std::uint32_t>(deviceIdValue);
            nodeIdOut = idValue;
            resolvedText = formatKgBindExternal(deviceIdOut, idValue, static_cast<std::uint32_t>(toValue));
            if (resolvedVariable != nullptr) *resolvedVariable = nullptr;
            return true;
        }

        if (!isIdentifier(token)) { errorMessage = "Expected KgBindExternal variable or `<...>` literal."; return false; }
        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) { errorMessage = "Unknown variable `" + std::string(token) + "`."; return false; }
        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgBindExternal) { errorMessage = "Variable `" + variable.name + "` has incompatible type."; return false; }

        deviceIdOut = variable.deviceIdValue;
        nodeIdOut = variable.scalarValue;
        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) *resolvedVariable = &variable;
        return true;
    }

    bool analyzeKgBindExternalInlineLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::uint32_t& deviceIdOut,
        std::uint64_t& nodeIdOut,
        std::pair<std::size_t, std::size_t>* idValueRange = nullptr
    ) {
        if (token.empty() || token.front() != '<') { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected KgBindExternal literal."); return false; }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != '>') { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `>`."); addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error); return false; }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto fields = splitWords(body);
        bool positionalBlocked = false;
        std::array<bool, 3> assigned{false, false, false};
        std::size_t nextPositionalIndex = 0;
        std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
        std::uint64_t idValue = 0;
        std::uint64_t toValue = 0;
        const std::array<std::string_view, 3> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::To};

        for (const auto& field : fields) {
            const std::size_t fieldStart = valueStart + 1 + field.start;
            const std::size_t fieldEnd = valueStart + 1 + field.end;
            const std::size_t equals = field.text.find('=');
            const bool isNamed = equals != std::string_view::npos;
            std::size_t parameterIndex = 0;
            std::string_view parameterValue = field.text;
            std::size_t parameterValueStart = fieldStart;

            if (isNamed) {
                const std::string_view fieldName = field.text.substr(0, equals);
                parameterValue = field.text.substr(equals + 1);
                parameterValueStart = fieldStart + equals + 1;
                bool found = false;
                for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                    if (fieldNames[index] == fieldName) { parameterIndex = index; found = true; break; }
                }
                if (!found) { addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Unknown KgBindExternal field."); addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error); return false; }
                addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Property);
                addToken(doc, lineIndex, fieldStart + equals, fieldStart + equals + 1, TokenType::Operator);
                if (assigned[parameterIndex]) { addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Field specified more than once."); addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error); return false; }
                if (parameterIndex > nextPositionalIndex) positionalBlocked = true;
            } else {
                if (positionalBlocked) { addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Positional field cannot appear after named fields."); addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error); return false; }
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
                if (nextPositionalIndex >= assigned.size()) { addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Too many positional fields."); addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error); return false; }
                parameterIndex = nextPositionalIndex;
            }

            if (parameterIndex == 0) {
                if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::UInt32, deviceIdValue)) return false;
            } else if (parameterIndex == 1) {
                std::string tmpStr;
                if (!analyzeNodeIdAt(doc, lineIndex, parameterValueStart, parameterValue, variables, idValue, tmpStr, idValueRange)) return false;
            } else {
                if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::UInt32, toValue)) return false;
            }

            assigned[parameterIndex] = true;
            while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
        }

        if (!assigned[0] && !defaultDeviceId.has_value()) { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Default `deviceId` is unavailable."); addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error); return false; }

        deviceIdOut = static_cast<std::uint32_t>(deviceIdValue);
        nodeIdOut = idValue;
        resolvedText = formatKgBindExternal(deviceIdOut, idValue, static_cast<std::uint32_t>(toValue));
        return true;
    }

    // KgBindInput parsers
    bool resolveKgBindInputValueToken(
        const VariableMap& variables,
        const std::string_view token,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        if (!token.empty() && token.front() == '<' && token.back() == '>') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto fields = splitWords(body);
            bool positionalBlocked = false;
            std::array<bool, 3> assigned{false, false, false};
            std::size_t nextPositionalIndex = 0;
            std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
            std::uint64_t idValue = 0;
            std::uint64_t inValue = 0;
            const std::array<std::string_view, 3> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::In};

            for (const auto& field : fields) {
                const std::size_t equals = field.text.find('=');
                const bool isNamed = equals != std::string_view::npos;
                std::size_t parameterIndex = 0;
                std::string_view parameterValue = field.text;

                if (isNamed) {
                    const std::string_view fieldName = field.text.substr(0, equals);
                    parameterValue = field.text.substr(equals + 1);
                    bool found = false;
                    for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                        if (fieldNames[index] == fieldName) { parameterIndex = index; found = true; break; }
                    }
                    if (!found) { errorMessage = "Unknown KgBindInput field `" + std::string(fieldName) + "`."; return false; }
                    if (assigned[parameterIndex]) { errorMessage = "Field specified more than once."; return false; }
                    if (parameterIndex > nextPositionalIndex) positionalBlocked = true;
                } else {
                    if (positionalBlocked) { errorMessage = "Positional field out of order."; return false; }
                    while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
                    if (nextPositionalIndex >= assigned.size()) { errorMessage = "Too many positional fields."; return false; }
                    parameterIndex = nextPositionalIndex;
                }

                if (parameterIndex == 0) {
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, deviceIdValue, errorMessage)) return false;
                } else if (parameterIndex == 1) {
                    std::string tmpStr;
                    if (!resolveNodeIdToken(variables, parameterValue, idValue, tmpStr, errorMessage)) return false;
                } else {
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, inValue, errorMessage)) return false;
                }

                assigned[parameterIndex] = true;
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
            }

            if (!assigned[0] && !defaultDeviceId.has_value()) { errorMessage = "Default `deviceId` is unavailable."; return false; }
            resolvedText = formatKgBindInput(static_cast<std::uint32_t>(deviceIdValue), idValue, static_cast<std::uint32_t>(inValue));
            if (resolvedVariable != nullptr) *resolvedVariable = nullptr;
            return true;
        }
        if (!isIdentifier(token)) { errorMessage = "Expected KgBindInput variable or `<...>` literal."; return false; }
        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) { errorMessage = "Unknown variable `" + std::string(token) + "`."; return false; }
        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgBindInput) { errorMessage = "Variable `" + variable.name + "` has incompatible type."; return false; }
        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) *resolvedVariable = &variable;
        return true;
    }

    bool analyzeKgBindInputInlineLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText
    ) {
        if (token.empty() || token.front() != '<') { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected KgBindInput literal."); return false; }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != '>') { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `>`."); addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error); return false; }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto fields = splitWords(body);
        bool positionalBlocked = false;
        std::array<bool, 3> assigned{false, false, false};
        std::size_t nextPositionalIndex = 0;
        std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
        std::uint64_t idValue = 0;
        std::uint64_t inValue = 0;
        std::pair<std::size_t, std::size_t> idValueRange{valueStart, valueStart + token.size()};
        std::pair<std::size_t, std::size_t> inValueRange{valueStart, valueStart + token.size()};
        const std::array<std::string_view, 3> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::In};

        for (const auto& field : fields) {
            const std::size_t fieldStart = valueStart + 1 + field.start;
            const std::size_t fieldEnd = valueStart + 1 + field.end;
            const std::size_t equals = field.text.find('=');
            const bool isNamed = equals != std::string_view::npos;
            std::size_t parameterIndex = 0;
            std::string_view parameterValue = field.text;
            std::size_t parameterValueStart = fieldStart;

            if (isNamed) {
                const std::string_view fieldName = field.text.substr(0, equals);
                parameterValue = field.text.substr(equals + 1);
                parameterValueStart = fieldStart + equals + 1;
                bool found = false;
                for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                    if (fieldNames[index] == fieldName) { parameterIndex = index; found = true; break; }
                }
                if (!found) { addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Unknown KgBindInput field."); addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error); return false; }
                addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Property);
                addToken(doc, lineIndex, fieldStart + equals, fieldStart + equals + 1, TokenType::Operator);
                if (assigned[parameterIndex]) { addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Field specified more than once."); addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error); return false; }
                if (parameterIndex > nextPositionalIndex) positionalBlocked = true;
            } else {
                if (positionalBlocked) { addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Positional field out of order."); addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error); return false; }
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
                if (nextPositionalIndex >= assigned.size()) { addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Too many positional fields."); addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error); return false; }
                parameterIndex = nextPositionalIndex;
            }

            if (parameterIndex == 0) {
                if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::UInt32, deviceIdValue)) return false;
            } else if (parameterIndex == 1) {
                std::string tmpStr;
                if (!analyzeNodeIdAt(doc, lineIndex, parameterValueStart, parameterValue, variables, idValue, tmpStr, &idValueRange)) return false;
            } else {
                inValueRange = {parameterValueStart, parameterValueStart + parameterValue.size()};
                if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::UInt32, inValue)) return false;
            }

            assigned[parameterIndex] = true;
            while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
        }

        if (!assigned[0] && !defaultDeviceId.has_value()) { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Default `deviceId` is unavailable."); addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error); return false; }

        if (assigned[0] && assigned[1]) {
            auto key = std::make_pair(static_cast<std::uint32_t>(deviceIdValue), idValue);
            auto it = doc.declaredNodes.find(key);
            if (it == doc.declaredNodes.end()) {
                addDiagnostic(doc, lineIndex, idValueRange.first, idValueRange.second, "Target node not found.", "warning");
            } else if (assigned[2] && inValue >= it->second) {
                addDiagnostic(doc, lineIndex, inValueRange.first, inValueRange.second, "Input index out of bounds for the specified node.", "warning");
            }
        }

        resolvedText = formatKgBindInput(static_cast<std::uint32_t>(deviceIdValue), idValue, static_cast<std::uint32_t>(inValue));
        return true;
    }

    bool resolveListKgBindInputValueToken(
        const VariableMap& variables,
        const std::string_view token,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr,
        std::size_t* outCount = nullptr
    ) {
        if (!token.empty() && token.front() == '[' && token.back() == ']') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto items = splitWords(body);
            if (outCount) *outCount = items.size();
            std::ostringstream stream;
            stream << '[';
            bool first = true;
            for (const auto& item : items) {
                std::string itemText;
                if (!resolveKgBindInputValueToken(variables, item.text, defaultDeviceId, itemText, errorMessage)) return false;
                if (!first) stream << ' ';
                first = false;
                stream << itemText;
            }
            stream << ']';
            resolvedText = stream.str();
            if (resolvedVariable != nullptr) *resolvedVariable = nullptr;
            return true;
        }
        if (!isIdentifier(token)) { errorMessage = "Expected `[]KgBindInput` variable or `[...]` literal."; return false; }
        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) { errorMessage = "Unknown variable `" + std::string(token) + "`."; return false; }
        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::ListKgBindInput) { errorMessage = "Variable `" + variable.name + "` has incompatible type."; return false; }
        if (outCount) *outCount = variable.listCount;
        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) *resolvedVariable = &variable;
        return true;
    }

    bool analyzeListKgBindInputLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::size_t* outCount = nullptr
    ) {
        if (token.empty() || token.front() != '[') { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected list literal `[ ... ]`."); return false; }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != ']') { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `]`."); addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error); return false; }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto items = splitWords(body);
        if (outCount) *outCount = items.size();
        std::ostringstream stream;
        stream << '[';
        bool first = true;
        for (const auto& item : items) {
            const std::size_t itemStart = valueStart + 1 + item.start;
            std::string itemText;
            const VariableInfo* resolvedVariable = nullptr;
            if (!item.text.empty() && item.text.front() == '<') {
                if (!analyzeKgBindInputInlineLiteral(doc, lineIndex, itemStart, item.text, variables, defaultDeviceId, itemText)) return false;
            } else {
                std::string errorMessage;
                if (!resolveKgBindInputValueToken(variables, item.text, defaultDeviceId, itemText, errorMessage, &resolvedVariable)) {
                    addDiagnostic(doc, lineIndex, itemStart, itemStart + item.text.size(), errorMessage);
                    addToken(doc, lineIndex, itemStart, itemStart + item.text.size(), TokenType::Error);
                    return false;
                }
                if (resolvedVariable != nullptr) {
                    addHover(doc, lineIndex, itemStart, itemStart + item.text.size(), makeVariableHoverMarkdown(*resolvedVariable));
                    addToken(doc, lineIndex, itemStart, itemStart + item.text.size(), TokenType::Variable);
                }
            }
            if (!first) stream << ' ';
            first = false;
            stream << itemText;
        }
        stream << ']';
        resolvedText = stream.str();
        return true;
    }

    // KgBind parsers
    bool resolveKgBindValueToken(
        const VariableMap& variables,
        const std::string_view token,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::uint32_t& deviceIdOut,
        std::uint64_t& nodeIdOut,
        std::string& errorMessage,
        const VariableInfo** resolvedVariable = nullptr
    ) {
        if (!token.empty() && token.front() == '<' && token.back() == '>') {
            const std::string_view body = token.substr(1, token.size() - 2);
            const auto fields = splitWords(body);
            bool positionalBlocked = false;
            std::array<bool, 3> assigned{false, false, false};
            std::size_t nextPositionalIndex = 0;
            std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
            std::uint64_t idValue = 0;
            std::string toText = "[]";
            const std::array<std::string_view, 3> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::To};

            for (const auto& field : fields) {
                const std::size_t equals = field.text.find('=');
                const bool isNamed = equals != std::string_view::npos;
                std::size_t parameterIndex = 0;
                std::string_view parameterValue = field.text;

                if (isNamed) {
                    const std::string_view fieldName = field.text.substr(0, equals);
                    parameterValue = field.text.substr(equals + 1);
                    bool found = false;
                    for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                        if (fieldNames[index] == fieldName) { parameterIndex = index; found = true; break; }
                    }
                    if (!found) { errorMessage = "Unknown KgBind field `" + std::string(fieldName) + "`."; return false; }
                    if (assigned[parameterIndex]) { errorMessage = "Field specified more than once."; return false; }
                    if (parameterIndex > nextPositionalIndex) positionalBlocked = true;
                } else {
                    if (positionalBlocked) { errorMessage = "Positional field out of order."; return false; }
                    while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
                    if (nextPositionalIndex >= assigned.size()) { errorMessage = "Too many positional fields."; return false; }
                    parameterIndex = nextPositionalIndex;
                }

                if (parameterIndex == 0) {
                    if (!resolveValueToken(variables, parameterValue, ValueType::UInt32, deviceIdValue, errorMessage)) return false;
                } else if (parameterIndex == 1) {
                    std::string tmpStr;
                    if (!resolveNodeIdToken(variables, parameterValue, idValue, tmpStr, errorMessage)) return false;
                } else {
                    if (!resolveListKgBindInputValueToken(variables, parameterValue, defaultDeviceId, toText, errorMessage)) return false;
                }

                assigned[parameterIndex] = true;
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
            }

            if (!assigned[0] && !defaultDeviceId.has_value()) { errorMessage = "Default `deviceId` is unavailable."; return false; }
            deviceIdOut = static_cast<std::uint32_t>(deviceIdValue);
            nodeIdOut = idValue;
            resolvedText = formatKgBind(deviceIdOut, idValue, toText);
            if (resolvedVariable != nullptr) *resolvedVariable = nullptr;
            return true;
        }

        if (!isIdentifier(token)) { errorMessage = "Expected KgBind variable or `<...>` literal."; return false; }
        const auto variableIt = variables.find(token);
        if (variableIt == variables.end()) { errorMessage = "Unknown variable `" + std::string(token) + "`."; return false; }
        const VariableInfo& variable = variableIt->second;
        if (variable.type != ValueType::KgBind) { errorMessage = "Variable `" + variable.name + "` has incompatible type."; return false; }
        deviceIdOut = variable.deviceIdValue;
        nodeIdOut = variable.scalarValue;
        resolvedText = variable.resolvedText;
        if (resolvedVariable != nullptr) *resolvedVariable = &variable;
        return true;
    }

    bool analyzeKgBindInlineLiteral(
        DocumentState& doc,
        const std::size_t lineIndex,
        const std::size_t valueStart,
        const std::string_view token,
        const VariableMap& variables,
        const std::optional<std::uint32_t> defaultDeviceId,
        std::string& resolvedText,
        std::uint32_t& deviceIdOut,
        std::uint64_t& nodeIdOut,
        std::pair<std::size_t, std::size_t>* idValueRange = nullptr
    ) {
        if (token.empty() || token.front() != '<') { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected KgBind literal."); return false; }
        addToken(doc, lineIndex, valueStart, valueStart + 1, TokenType::Operator);
        if (token.back() != '>') { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Expected closing `>`."); addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error); return false; }
        addToken(doc, lineIndex, valueStart + token.size() - 1, valueStart + token.size(), TokenType::Operator);

        const std::string_view body = token.substr(1, token.size() - 2);
        const auto fields = splitWords(body);
        bool positionalBlocked = false;
        std::array<bool, 3> assigned{false, false, false};
        std::size_t nextPositionalIndex = 0;
        std::uint64_t deviceIdValue = defaultDeviceId.value_or(0);
        std::uint64_t idValue = 0;
        std::string toText = "[]";
        const std::array<std::string_view, 3> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::To};

        for (const auto& field : fields) {
            const std::size_t fieldStart = valueStart + 1 + field.start;
            const std::size_t fieldEnd = valueStart + 1 + field.end;
            const std::size_t equals = field.text.find('=');
            const bool isNamed = equals != std::string_view::npos;
            std::size_t parameterIndex = 0;
            std::string_view parameterValue = field.text;
            std::size_t parameterValueStart = fieldStart;

            if (isNamed) {
                const std::string_view fieldName = field.text.substr(0, equals);
                parameterValue = field.text.substr(equals + 1);
                parameterValueStart = fieldStart + equals + 1;
                bool found = false;
                for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                    if (fieldNames[index] == fieldName) { parameterIndex = index; found = true; break; }
                }
                if (!found) { addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Unknown KgBind field."); addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error); return false; }
                addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Property);
                addToken(doc, lineIndex, fieldStart + equals, fieldStart + equals + 1, TokenType::Operator);
                if (assigned[parameterIndex]) { addDiagnostic(doc, lineIndex, fieldStart, fieldStart + equals, "Field specified more than once."); addToken(doc, lineIndex, fieldStart, fieldStart + equals, TokenType::Error); return false; }
                if (parameterIndex > nextPositionalIndex) positionalBlocked = true;
            } else {
                if (positionalBlocked) { addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Positional field out of order."); addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error); return false; }
                while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
                if (nextPositionalIndex >= assigned.size()) { addDiagnostic(doc, lineIndex, fieldStart, fieldEnd, "Too many positional fields."); addToken(doc, lineIndex, fieldStart, fieldEnd, TokenType::Error); return false; }
                parameterIndex = nextPositionalIndex;
            }

            if (parameterIndex == 0) {
                if (!analyzeScalarValueAt(doc, lineIndex, parameterValueStart, parameterValue, variables, ValueType::UInt32, deviceIdValue)) return false;
            } else if (parameterIndex == 1) {
                std::string tmpStr;
                if (!analyzeNodeIdAt(doc, lineIndex, parameterValueStart, parameterValue, variables, idValue, tmpStr, idValueRange)) return false;
            } else {
                if (!parameterValue.empty() && parameterValue.front() == '[') {
                    if (!analyzeListKgBindInputLiteral(doc, lineIndex, parameterValueStart, parameterValue, variables, defaultDeviceId, toText)) return false;
                } else {
                    std::string errorMessage;
                    const VariableInfo* resolvedVariable = nullptr;
                    if (!resolveListKgBindInputValueToken(variables, parameterValue, defaultDeviceId, toText, errorMessage, &resolvedVariable)) {
                        addDiagnostic(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), errorMessage);
                        addToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), TokenType::Error);
                        return false;
                    }
                    if (resolvedVariable != nullptr) {
                        addHover(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), makeVariableHoverMarkdown(*resolvedVariable));
                        addToken(doc, lineIndex, parameterValueStart, parameterValueStart + parameterValue.size(), TokenType::Variable);
                    }
                }
            }

            assigned[parameterIndex] = true;
            while (nextPositionalIndex < assigned.size() && assigned[nextPositionalIndex]) ++nextPositionalIndex;
        }

        if (!assigned[0] && !defaultDeviceId.has_value()) { addDiagnostic(doc, lineIndex, valueStart, valueStart + token.size(), "Default `deviceId` is unavailable."); addToken(doc, lineIndex, valueStart, valueStart + token.size(), TokenType::Error); return false; }

        deviceIdOut = static_cast<std::uint32_t>(deviceIdValue);
        nodeIdOut = idValue;
        resolvedText = formatKgBind(deviceIdOut, idValue, toText);
        return true;
    }

    void addToken(DocumentState& doc, std::size_t line, std::size_t start, std::size_t end, TokenType type) {
        if (end <= start) {
            return;
        }

        if (activeLogicalStatement == nullptr) {
            doc.tokens.push_back(SemanticToken{
                static_cast<int>(line),
                static_cast<int>(start),
                static_cast<int>(end - start),
                type
            });
            return;
        }

        int currentLine = -1;
        int segmentStart = -1;
        int previousCharacter = -1;

        const std::size_t safeEnd = std::min(end, activeLogicalStatement->text.size());
        for (std::size_t offset = start; offset < safeEnd; ++offset) {
            if (activeLogicalStatement->text[offset] == '\n') {
                if (segmentStart >= 0) {
                    doc.tokens.push_back(SemanticToken{
                        currentLine,
                        segmentStart,
                        previousCharacter - segmentStart + 1,
                        type
                    });
                }
                currentLine = -1;
                segmentStart = -1;
                previousCharacter = -1;
                continue;
            }

            const auto [mappedLine, mappedCharacter] = activeLogicalStatement->offsetMap[offset];
            if (segmentStart >= 0 && mappedLine == currentLine && mappedCharacter == previousCharacter + 1) {
                previousCharacter = mappedCharacter;
                continue;
            }

            if (segmentStart >= 0) {
                doc.tokens.push_back(SemanticToken{
                    currentLine,
                    segmentStart,
                    previousCharacter - segmentStart + 1,
                    type
                });
            }

            currentLine = mappedLine;
            segmentStart = mappedCharacter;
            previousCharacter = mappedCharacter;
        }

        if (segmentStart >= 0) {
            doc.tokens.push_back(SemanticToken{
                currentLine,
                segmentStart,
                previousCharacter - segmentStart + 1,
                type
            });
        }
    }

    void addDiagnostic(
        DocumentState& doc,
        std::size_t line,
        std::size_t start,
        std::size_t end,
        std::string message,
        std::string severity = "error"
    ) {
        if (activeLogicalStatement == nullptr) {
            doc.diagnostics.push_back(Diagnostic{
                static_cast<int>(line),
                static_cast<int>(start),
                static_cast<int>(line),
                static_cast<int>(end),
                std::move(message),
                std::move(severity)
            });
            return;
        }

        const auto [startLine, startCharacter] = mapLogicalBoundary(start, start, false);
        const auto [endLine, endCharacter] = mapLogicalBoundary(end, start, true);
        doc.diagnostics.push_back(Diagnostic{
            startLine,
            startCharacter,
            endLine,
            endCharacter,
            std::move(message),
            std::move(severity)
        });
    }

    void addHover(DocumentState& doc, std::size_t line, std::size_t start, std::size_t end, std::string markdown) {
        if (activeLogicalStatement == nullptr) {
            doc.hovers.push_back(HoverInfo{
                static_cast<int>(line),
                static_cast<int>(start),
                static_cast<int>(line),
                static_cast<int>(end),
                std::move(markdown)
            });
            return;
        }

        const auto [startLine, startCharacter] = mapLogicalBoundary(start, start, false);
        const auto [endLine, endCharacter] = mapLogicalBoundary(end, start, true);
        doc.hovers.push_back(HoverInfo{
            startLine,
            startCharacter,
            endLine,
            endCharacter,
            std::move(markdown)
        });
    }

    void analyzeKnownCommand(
        DocumentState& doc,
        const std::string& line,
        const std::vector<LineWord>& words,
        std::size_t lineIndex,
        const CommandInfo& command,
        const VariableMap& variables,
        std::optional<std::uint32_t>& defaultDeviceId,
        std::set<std::pair<std::uint32_t, std::uint64_t>>& usedEntities
    ) {
        const LineWord& keyword = words.front();
        addToken(doc, lineIndex, keyword.start, keyword.end, TokenType::Keyword);

        const TypeInfo* typeInfo = findTypeInfoByValueType(command.valueType);
        std::ostringstream hover;
        hover << "```rpconfig\n" << command.keyword << " <" << command.parameterName << ">\n```\n"
              << command.description << "\n\n"
              << "Allowed type: `" << (typeInfo != nullptr ? typeInfo->keyword : "unknown") << "`.";
        addHover(doc, lineIndex, keyword.start, keyword.end, hover.str());

        if (words.size() == 1) {
            addDiagnostic(
                doc,
                lineIndex,
                keyword.start,
                keyword.end,
                "Expected argument <" + std::string(command.parameterName) + ">."
            );
            return;
        }

        if (command.valueType == ValueType::KgIP || command.valueType == ValueType::KgRun ||
            command.valueType == ValueType::KgNode || command.valueType == ValueType::KgSensor ||
            command.valueType == ValueType::KgBindExternal || command.valueType == ValueType::KgBind) {

            std::size_t argumentStart = words[1].start;
            while (argumentStart < line.size() && isWhitespace(line[argumentStart])) {
                ++argumentStart;
            }

            const std::string_view argumentText = trimView(std::string_view(line.data() + argumentStart, line.size() - argumentStart));
            std::string resolvedText;
            const VariableInfo* resolvedVariable = nullptr;
            std::string errorMessage;

            if (command.valueType == ValueType::KgIP) {
                if (!argumentText.empty() && argumentText.front() == '<') {
                    if (!analyzeKgIpInlineLiteral(doc, lineIndex, argumentStart, argumentText, variables, resolvedText)) {
                        return;
                    }
                } else if (!resolveKgIpValueToken(variables, argumentText, resolvedText, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, argumentStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, argumentStart, line.size(), errorMessage);
                    return;
                }
            } else if (command.valueType == ValueType::KgRun) {
                if (!argumentText.empty() && argumentText.front() == '<') {
                    if (!analyzeKgRunInlineLiteral(doc, lineIndex, argumentStart, argumentText, variables, defaultDeviceId, resolvedText)) {
                        return;
                    }
                } else if (!resolveKgRunValueToken(variables, argumentText, defaultDeviceId, resolvedText, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, argumentStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, argumentStart, line.size(), errorMessage);
                    return;
                }
            } else {
                std::uint32_t parsedDeviceId = 0;
                std::uint64_t parsedNodeId = 0;
                std::size_t inputsCount = 0;
                std::pair<std::size_t, std::size_t> idRange{argumentStart, argumentStart + argumentText.size()};

                if (command.valueType == ValueType::KgNode) {
                    if (!argumentText.empty() && argumentText.front() == '<') {
                        if (!analyzeKgNodeInlineLiteral(doc, lineIndex, argumentStart, argumentText, variables, defaultDeviceId, resolvedText, parsedDeviceId, parsedNodeId, &idRange, &inputsCount)) {
                            return;
                        }
                    } else if (!resolveKgNodeValueToken(variables, argumentText, defaultDeviceId, resolvedText, parsedDeviceId, parsedNodeId, errorMessage, &resolvedVariable, &inputsCount)) {
                        addToken(doc, lineIndex, argumentStart, line.size(), TokenType::Error);
                        addDiagnostic(doc, lineIndex, argumentStart, line.size(), errorMessage);
                        return;
                    }
                    if (usedEntities.contains({parsedDeviceId, parsedNodeId})) {
                        addDiagnostic(doc, lineIndex, idRange.first, idRange.second, "Node with deviceId `" + std::to_string(parsedDeviceId) + "` and id `" + formatScalarValue(ValueType::KgNodeId, parsedNodeId) + "` already exists.");
                        addToken(doc, lineIndex, idRange.first, idRange.second, TokenType::Error);
                        return;
                    }
                    usedEntities.insert({parsedDeviceId, parsedNodeId});
                    doc.declaredNodes[{parsedDeviceId, parsedNodeId}] = inputsCount;
                } else if (command.valueType == ValueType::KgSensor) {
                    std::uint32_t sensorId = 0;
                    if (!argumentText.empty() && argumentText.front() == '<') {
                        if (!analyzeKgSensorInlineLiteral(doc, lineIndex, argumentStart, argumentText, variables, defaultDeviceId, resolvedText, parsedDeviceId, sensorId, &idRange)) {
                            return;
                        }
                    } else if (!resolveKgSensorValueToken(variables, argumentText, defaultDeviceId, resolvedText, parsedDeviceId, sensorId, errorMessage, &resolvedVariable)) {
                        addToken(doc, lineIndex, argumentStart, line.size(), TokenType::Error);
                        addDiagnostic(doc, lineIndex, argumentStart, line.size(), errorMessage);
                        return;
                    }
                    parsedNodeId = sensorId;
                    if (usedEntities.contains({parsedDeviceId, parsedNodeId})) {
                        addDiagnostic(doc, lineIndex, idRange.first, idRange.second, "Sensor with deviceId `" + std::to_string(parsedDeviceId) + "` and id `" + formatScalarValue(ValueType::KgNodeId, parsedNodeId) + "` already exists.");
                        addToken(doc, lineIndex, idRange.first, idRange.second, TokenType::Error);
                        return;
                    }
                    usedEntities.insert({parsedDeviceId, parsedNodeId});
                    doc.declaredNodes[{parsedDeviceId, parsedNodeId}] = 0;
                } else if (command.valueType == ValueType::KgBindExternal) {
                    if (!argumentText.empty() && argumentText.front() == '<') {
                        if (!analyzeKgBindExternalInlineLiteral(doc, lineIndex, argumentStart, argumentText, variables, defaultDeviceId, resolvedText, parsedDeviceId, parsedNodeId, &idRange)) return;
                    } else if (!resolveKgBindExternalValueToken(variables, argumentText, defaultDeviceId, resolvedText, parsedDeviceId, parsedNodeId, errorMessage, &resolvedVariable)) {
                        addToken(doc, lineIndex, argumentStart, line.size(), TokenType::Error);
                        addDiagnostic(doc, lineIndex, argumentStart, line.size(), errorMessage);
                        return;
                    }
                    if (!doc.declaredNodes.contains({parsedDeviceId, parsedNodeId})) {
                        addDiagnostic(doc, lineIndex, idRange.first, idRange.second, "Source node for bindE not found.", "warning");
                    }
                } else if (command.valueType == ValueType::KgBind) {
                    if (!argumentText.empty() && argumentText.front() == '<') {
                        if (!analyzeKgBindInlineLiteral(doc, lineIndex, argumentStart, argumentText, variables, defaultDeviceId, resolvedText, parsedDeviceId, parsedNodeId, &idRange)) return;
                    } else if (!resolveKgBindValueToken(variables, argumentText, defaultDeviceId, resolvedText, parsedDeviceId, parsedNodeId, errorMessage, &resolvedVariable)) {
                        addToken(doc, lineIndex, argumentStart, line.size(), TokenType::Error);
                        addDiagnostic(doc, lineIndex, argumentStart, line.size(), errorMessage);
                        return;
                    }
                    if (!doc.declaredNodes.contains({parsedDeviceId, parsedNodeId})) {
                        addDiagnostic(doc, lineIndex, idRange.first, idRange.second, "Source node for bind not found.", "warning");
                    }
                }
            }

            std::ostringstream argHover;
            argHover << "Parameter `" << command.parameterName << "`.\n\n";
            if (resolvedVariable != nullptr) {
                argHover << makeVariableHoverMarkdown(*resolvedVariable) << "\n\n";
            }
            argHover << "Resolved value: `" << resolvedText << "`.";
            addHover(doc, lineIndex, argumentStart, line.size(), argHover.str());
            return;
        }

        const LineWord& argument = words[1];
        std::uint64_t parsedValue = 0;
        std::string errorMessage;
        const VariableInfo* resolvedVariable = nullptr;
        if (!resolveValueToken(variables, argument.text, command.valueType, parsedValue, errorMessage, &resolvedVariable)) {
            addToken(doc, lineIndex, argument.start, argument.end, TokenType::Error);
            addDiagnostic(doc, lineIndex, argument.start, argument.end, errorMessage);
        } else {
            addScalarValueToken(doc, lineIndex, argument.start, argument.end, command.valueType, argument.text, resolvedVariable);

            std::ostringstream argHover;
            argHover << "Parameter `" << command.parameterName << "`.\n\n";
            if (resolvedVariable != nullptr) {
                argHover << makeVariableHoverMarkdown(*resolvedVariable) << "\n\n";
            }
            argHover << "Resolved value: `" << formatScalarValue(command.valueType, parsedValue) << "`.";
            addHover(doc, lineIndex, argument.start, argument.end, argHover.str());

            if (command.keyword == CoreTypes::Keyword::Device) {
                defaultDeviceId = static_cast<std::uint32_t>(parsedValue);
            }
        }

        if (words.size() > 2) {
            for (std::size_t i = 2; i < words.size(); ++i) {
                addToken(doc, lineIndex, words[i].start, words[i].end, TokenType::Error);
            }

            addDiagnostic(
                doc,
                lineIndex,
                words[2].start,
                words.back().end,
                "Only one argument is allowed for `" + std::string(command.keyword) + "`."
            );
        }
    }

    void analyzeLetDeclaration(
        DocumentState& doc,
        const std::string& line,
        const std::vector<LineWord>& words,
        std::size_t lineIndex,
        VariableMap& variables,
        const std::optional<std::uint32_t> defaultDeviceId
    ) {
        const LineWord& letWord = words.front();
        addToken(doc, lineIndex, letWord.start, letWord.end, TokenType::Keyword);
        addHover(
            doc,
            lineIndex,
            letWord.start,
            letWord.end,
            "```rpconfig\nlet <type> <name> = <value>\n```\nDeclares a named helper value."
        );

        if (words.size() < 2) {
            addDiagnostic(doc, lineIndex, letWord.start, letWord.end, "Expected type after `let`.");
            return;
        }

        const LineWord& typeWord = words[1];
        const TypeInfo* declaredType = findTypeInfoByKeyword(typeWord.text);
        if (declaredType == nullptr) {
            addToken(doc, lineIndex, typeWord.start, typeWord.end, TokenType::Error);
            addDiagnostic(doc, lineIndex, typeWord.start, typeWord.end, "Unknown variable type `" + std::string(typeWord.text) + "`.");
            return;
        }

        addToken(doc, lineIndex, typeWord.start, typeWord.end, TokenType::Type);
        std::string typeHover = "Type `" + std::string(declaredType->keyword) + "`.";
        switch (declaredType->valueType) {
            case ValueType::KgIP: typeHover += "\n\nFields: `host`, `port`."; break;
            case ValueType::KgRun: typeHover += "\n\nFields: `deviceId`, `toRun`."; break;
            case ValueType::KgOutput: typeHover += "\n\nFields: `c`, `a`."; break;
            case ValueType::KgInput: typeHover += "\n\nFields: `c_i`, `a_i`, `b_i`, `v_i`, `g_i`."; break;
            case ValueType::KgNode: typeHover += "\n\nFields: `deviceId`, `id`, `algo`, `output`, `inputs`."; break;
            case ValueType::KgSensor: typeHover += "\n\nFields: `deviceId`, `id`, `period`, `toEmit`."; break;
            case ValueType::KgBindExternal: typeHover += "\n\nFields: `deviceId`, `id`, `to`."; break;
            case ValueType::KgBind: typeHover += "\n\nFields: `deviceId`, `id`, `to`."; break;
            case ValueType::KgBindInput: typeHover += "\n\nFields: `deviceId`, `id`, `in`."; break;
            case ValueType::KgNodeId: typeHover += "\n\nFormat: `level.index`."; break;
            case ValueType::AlgoType: typeHover += "\n\nAllowed values come from `Algoholic` and are written in quotes."; break;
            case ValueType::Direction: typeHover += "\n\nAllowed values: `\"up\"`, `\"down\"`."; break;
            case ValueType::ListUInt32:
            case ValueType::ListKgInput:
            case ValueType::ListKgBindInput:
                typeHover += "\n\nList literal syntax: `[item1 item2 ...]`."; break;
            default: typeHover += "\n\nAllowed range: `0.." + std::to_string(declaredType->maxValue) + "`."; break;
        }
        addHover(doc, lineIndex, typeWord.start, typeWord.end, typeHover);

        if (words.size() < 3) {
            addDiagnostic(doc, lineIndex, typeWord.start, typeWord.end, "Expected variable name.");
            return;
        }

        const LineWord& nameWord = words[2];
        if (!isIdentifier(nameWord.text)) {
            addToken(doc, lineIndex, nameWord.start, nameWord.end, TokenType::Error);
            addDiagnostic(doc, lineIndex, nameWord.start, nameWord.end, "Invalid variable name `" + std::string(nameWord.text) + "`.");
            return;
        }

        if (words.size() < 4) {
            addDiagnostic(doc, lineIndex, nameWord.start, nameWord.end, "Expected `=` after variable name.");
            return;
        }

        const LineWord& equalsWord = words[3];
        if (equalsWord.text != "=") {
            addToken(doc, lineIndex, equalsWord.start, equalsWord.end, TokenType::Error);
            addDiagnostic(doc, lineIndex, equalsWord.start, equalsWord.end, "Expected `=` before variable value.");
            return;
        }
        addToken(doc, lineIndex, equalsWord.start, equalsWord.end, TokenType::Operator);

        if (words.size() < 5) {
            addDiagnostic(doc, lineIndex, nameWord.start, nameWord.end, "Expected value after `=`.");
            return;
        }

        if (variables.contains(nameWord.text)) {
            addToken(doc, lineIndex, nameWord.start, nameWord.end, TokenType::Error);
            addDiagnostic(doc, lineIndex, nameWord.start, nameWord.end, "Variable `" + std::string(nameWord.text) + "` is already declared.");
            return;
        }

        VariableInfo variable;
        variable.name = std::string(nameWord.text);
        variable.type = declaredType->valueType;
        variable.declarationLine = static_cast<int>(lineIndex);
        variable.nameStart = static_cast<int>(nameWord.start);
        variable.nameEnd = static_cast<int>(nameWord.end);

        if (declaredType->valueType == ValueType::KgIP
            || declaredType->valueType == ValueType::KgRun
            || declaredType->valueType == ValueType::KgOutput
            || declaredType->valueType == ValueType::KgInput
            || declaredType->valueType == ValueType::ListUInt32
            || declaredType->valueType == ValueType::ListKgInput
            || declaredType->valueType == ValueType::KgNode
            || declaredType->valueType == ValueType::KgSensor
            || declaredType->valueType == ValueType::KgBindExternal
            || declaredType->valueType == ValueType::KgBind
            || declaredType->valueType == ValueType::KgBindInput
            || declaredType->valueType == ValueType::ListKgBindInput) {
            std::size_t valueStart = equalsWord.end;
            while (valueStart < line.size() && isWhitespace(line[valueStart])) {
                ++valueStart;
            }

            if (valueStart >= line.size()) {
                addDiagnostic(doc, lineIndex, nameWord.start, nameWord.end, "Expected value after `=`.");
                return;
            }

            const std::string_view valueText = trimView(std::string_view(line.data() + valueStart, line.size() - valueStart));
            const VariableInfo* resolvedVariable = nullptr;
            std::string errorMessage;
            if (declaredType->valueType == ValueType::KgIP) {
                if (!valueText.empty() && valueText.front() == '<') {
                    if (!analyzeKgIpInlineLiteral(doc, lineIndex, valueStart, valueText, variables, variable.resolvedText)) return;
                } else if (!resolveKgIpValueToken(variables, valueText, variable.resolvedText, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
            } else if (declaredType->valueType == ValueType::KgRun) {
                if (!valueText.empty() && valueText.front() == '<') {
                    if (!analyzeKgRunInlineLiteral(doc, lineIndex, valueStart, valueText, variables, defaultDeviceId, variable.resolvedText)) return;
                } else if (!resolveKgRunValueToken(variables, valueText, defaultDeviceId, variable.resolvedText, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
            } else if (declaredType->valueType == ValueType::KgOutput) {
                if (!valueText.empty() && valueText.front() == '<') {
                    if (!analyzeKgOutputInlineLiteral(doc, lineIndex, valueStart, valueText, variables, variable.resolvedText)) return;
                } else if (!resolveKgOutputValueToken(variables, valueText, variable.resolvedText, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
            } else if (declaredType->valueType == ValueType::KgInput) {
                if (!valueText.empty() && valueText.front() == '<') {
                    if (!analyzeKgInputInlineLiteral(doc, lineIndex, valueStart, valueText, variables, variable.resolvedText)) return;
                } else if (!resolveKgInputValueToken(variables, valueText, variable.resolvedText, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
            } else if (declaredType->valueType == ValueType::ListUInt32) {
                std::size_t itemsCount = 0;
                if (!valueText.empty() && valueText.front() == '[') {
                    if (!analyzeListUInt32Literal(doc, lineIndex, valueStart, valueText, variables, variable.resolvedText, &itemsCount)) return;
                } else if (!resolveListUInt32ValueToken(variables, valueText, variable.resolvedText, errorMessage, &resolvedVariable, &itemsCount)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
                variable.listCount = itemsCount;
            } else if (declaredType->valueType == ValueType::ListKgInput) {
                std::size_t itemsCount = 0;
                if (!valueText.empty() && valueText.front() == '[') {
                    if (!analyzeListKgInputLiteral(doc, lineIndex, valueStart, valueText, variables, variable.resolvedText, &itemsCount)) return;
                } else if (!resolveListKgInputValueToken(variables, valueText, variable.resolvedText, errorMessage, &resolvedVariable, &itemsCount)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
                variable.listCount = itemsCount;
            } else if (declaredType->valueType == ValueType::KgNode) {
                std::uint32_t parsedDeviceId = 0;
                std::uint64_t nodeIdValue = 0;
                std::size_t inputsCount = 0;
                if (!valueText.empty() && valueText.front() == '<') {
                    if (!analyzeKgNodeInlineLiteral(doc, lineIndex, valueStart, valueText, variables, defaultDeviceId, variable.resolvedText, parsedDeviceId, nodeIdValue, nullptr, &inputsCount)) return;
                } else if (!resolveKgNodeValueToken(variables, valueText, defaultDeviceId, variable.resolvedText, parsedDeviceId, nodeIdValue, errorMessage, &resolvedVariable, &inputsCount)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
                variable.scalarValue = nodeIdValue;
                variable.deviceIdValue = parsedDeviceId;
                variable.listCount = inputsCount;
                doc.declaredNodes[{parsedDeviceId, nodeIdValue}] = inputsCount;
            } else if (declaredType->valueType == ValueType::KgSensor) {
                std::uint32_t parsedDeviceId = 0;
                std::uint32_t sensorId = 0;
                if (!valueText.empty() && valueText.front() == '<') {
                    if (!analyzeKgSensorInlineLiteral(doc, lineIndex, valueStart, valueText, variables, defaultDeviceId, variable.resolvedText, parsedDeviceId, sensorId)) return;
                } else if (!resolveKgSensorValueToken(variables, valueText, defaultDeviceId, variable.resolvedText, parsedDeviceId, sensorId, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
                variable.scalarValue = sensorId;
                variable.deviceIdValue = parsedDeviceId;
            } else if (declaredType->valueType == ValueType::KgBindExternal) {
                std::uint32_t parsedDeviceId = 0;
                std::uint64_t nodeIdValue = 0;
                if (!valueText.empty() && valueText.front() == '<') {
                    if (!analyzeKgBindExternalInlineLiteral(doc, lineIndex, valueStart, valueText, variables, defaultDeviceId, variable.resolvedText, parsedDeviceId, nodeIdValue)) return;
                } else if (!resolveKgBindExternalValueToken(variables, valueText, defaultDeviceId, variable.resolvedText, parsedDeviceId, nodeIdValue, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
                variable.scalarValue = nodeIdValue;
                variable.deviceIdValue = parsedDeviceId;
            } else if (declaredType->valueType == ValueType::KgBindInput) {
                if (!valueText.empty() && valueText.front() == '<') {
                    if (!analyzeKgBindInputInlineLiteral(doc, lineIndex, valueStart, valueText, variables, defaultDeviceId, variable.resolvedText)) return;
                } else if (!resolveKgBindInputValueToken(variables, valueText, defaultDeviceId, variable.resolvedText, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
            } else if (declaredType->valueType == ValueType::ListKgBindInput) {
                std::size_t itemsCount = 0;
                if (!valueText.empty() && valueText.front() == '[') {
                    if (!analyzeListKgBindInputLiteral(doc, lineIndex, valueStart, valueText, variables, defaultDeviceId, variable.resolvedText, &itemsCount)) return;
                } else if (!resolveListKgBindInputValueToken(variables, valueText, defaultDeviceId, variable.resolvedText, errorMessage, &resolvedVariable, &itemsCount)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
                variable.listCount = itemsCount;
            } else if (declaredType->valueType == ValueType::KgBind) {
                std::uint32_t parsedDeviceId = 0;
                std::uint64_t nodeIdValue = 0;
                if (!valueText.empty() && valueText.front() == '<') {
                    if (!analyzeKgBindInlineLiteral(doc, lineIndex, valueStart, valueText, variables, defaultDeviceId, variable.resolvedText, parsedDeviceId, nodeIdValue)) return;
                } else if (!resolveKgBindValueToken(variables, valueText, defaultDeviceId, variable.resolvedText, parsedDeviceId, nodeIdValue, errorMessage, &resolvedVariable)) {
                    addToken(doc, lineIndex, valueStart, line.size(), TokenType::Error);
                    addDiagnostic(doc, lineIndex, valueStart, line.size(), errorMessage);
                    return;
                }
                variable.scalarValue = nodeIdValue;
                variable.deviceIdValue = parsedDeviceId;
            }

            if (resolvedVariable != nullptr) {
                addHover(doc, lineIndex, valueStart, line.size(), makeVariableHoverMarkdown(*resolvedVariable));
            }
        } else {
            const LineWord& valueWord = words[4];
            std::uint64_t resolvedValue = 0;
            std::string errorMessage;
            const VariableInfo* resolvedVariable = nullptr;

            if (words.size() > 5) {
                for (std::size_t i = 5; i < words.size(); ++i) {
                    addToken(doc, lineIndex, words[i].start, words[i].end, TokenType::Error);
                }

                addDiagnostic(doc, lineIndex, words[5].start, words.back().end, "Unexpected extra tokens after variable value.");
                return;
            }

            if (declaredType->valueType == ValueType::KgNodeId) {
                if (!analyzeNodeIdAt(doc, lineIndex, valueWord.start, valueWord.text, variables, resolvedValue, variable.resolvedText)) {
                    return;
                }
            } else if (!resolveValueToken(variables, valueWord.text, declaredType->valueType, resolvedValue, errorMessage, &resolvedVariable)) {
                addToken(doc, lineIndex, valueWord.start, valueWord.end, TokenType::Error);
                addDiagnostic(doc, lineIndex, valueWord.start, valueWord.end, errorMessage);
                return;
            } else {
                variable.resolvedText = formatScalarValue(declaredType->valueType, resolvedValue);
                if (resolvedVariable != nullptr) {
                    addHover(doc, lineIndex, valueWord.start, valueWord.end, makeVariableHoverMarkdown(*resolvedVariable));
                }
                addScalarValueToken(doc, lineIndex, valueWord.start, valueWord.end, declaredType->valueType, valueWord.text, resolvedVariable);
            }

            variable.scalarValue = resolvedValue;
        }

        variables.emplace(variable.name, variable);
        doc.variables.emplace(variable.name, variable);
        addHover(doc, lineIndex, nameWord.start, nameWord.end, makeVariableHoverMarkdown(variable));
    }

    void analyzeDocument(DocumentState& doc) {
        doc.tokens.clear();
        doc.diagnostics.clear();
        doc.hovers.clear();
        doc.variables.clear();
        doc.declaredNodes.clear();
        doc.lines = splitLines(doc.content);

        doc.statements.clear();
        doc.statements.reserve(doc.lines.size());

        VariableMap availableVariables;
        std::optional<std::uint32_t> defaultDeviceId;
        std::set<std::pair<std::uint32_t, std::uint64_t>> usedEntities;

        for (std::size_t lineIndex = 0; lineIndex < doc.lines.size(); ++lineIndex) {
            std::string_view lineView = doc.lines[lineIndex];
            std::size_t firstNonSpace = 0;
            while (firstNonSpace < lineView.size() && isWhitespace(lineView[firstNonSpace])) {
                firstNonSpace++;
            }
            if (firstNonSpace < lineView.size() && lineView[firstNonSpace] == '#') {
                addToken(doc, lineIndex, firstNonSpace, lineView.size(), TokenType::Comment);
                continue;
            }
            doc.statements.push_back(buildLogicalStatement(doc.lines, lineIndex));
            const LogicalStatement& statement = doc.statements.back();
            const std::string& line = statement.text;
            const auto words = splitWords(line);

            if (words.empty()) {
                lineIndex = statement.endLine;
                continue;
            }

            activeLogicalStatement = &statement;

            const LineWord& keyword = words.front();
            if (keyword.start != 0) {
                addDiagnostic(
                    doc,
                    lineIndex,
                    keyword.start,
                    keyword.end,
                    "Commands must start at column 0."
                );
            }

            if (keyword.text == CoreTypes::Keyword::Let) {
                analyzeLetDeclaration(doc, line, words, lineIndex, availableVariables, defaultDeviceId);
                activeLogicalStatement = nullptr;
                lineIndex = statement.endLine;
                continue;
            }

            const CommandInfo* commandInfo = findCommandInfo(keyword.text);
            if (commandInfo == nullptr) {
                addToken(doc, lineIndex, keyword.start, keyword.end, TokenType::Error);
                addDiagnostic(
                    doc,
                    lineIndex,
                    keyword.start,
                    keyword.end,
                    "Unknown top-level command `" + std::string(keyword.text) + "`."
                );
                activeLogicalStatement = nullptr;
                lineIndex = statement.endLine;
                continue;
            }

            analyzeKnownCommand(doc, line, words, lineIndex, *commandInfo, availableVariables, defaultDeviceId, usedEntities);
            activeLogicalStatement = nullptr;
            lineIndex = statement.endLine;
        }

        activeLogicalStatement = nullptr;
    }

    void addVariableCompletionItems(
        json& items,
        const DocumentState& doc,
        const ValueType expectedType,
        const int line,
        const std::string_view typedPrefix
    ) {
        for (const auto&[name, variable] : doc.variables) {
            if (variable.type != expectedType || variable.declarationLine >= line) {
                continue;
            }
            if (!typedPrefix.empty() && name.rfind(std::string(typedPrefix), 0) != 0) {
                continue;
            }

            items.push_back({
                {"label", name},
                {"kind", 6},
                {"detail", makeVariableHoverMarkdown(variable)},
                {"insertText", name + " "},
                {"command", makeRetriggerSuggestCommand()}
            });
        }
    }

    void addNodeIdCompletionItems(
        json& items,
        const DocumentState& doc,
        const int line,
        const std::optional<std::uint32_t> deviceIdFilter,
        const std::string_view typedPrefix
    ) {
        for (const auto& [key, count] : doc.declaredNodes) {
            if (deviceIdFilter.has_value() && key.first != deviceIdFilter.value()) {
                continue;
            }
            std::string literal = formatKgNodeId(static_cast<std::uint32_t>(key.second >> 32), static_cast<std::uint32_t>(key.second & 0xFFFFFFFF));
            if (!typedPrefix.empty() && literal.substr(0, typedPrefix.size()) != typedPrefix) {
                continue;
            }
            items.push_back({
                {"label", literal},
                {"kind", 12},
                {"detail", "Declared Node"},
                {"insertText", literal + " "},
                {"command", makeRetriggerSuggestCommand()}
            });
        }
        addVariableCompletionItems(items, doc, ValueType::KgNodeId, line, typedPrefix);
    }

    void addInputIndexCompletionItems(
        json& items,
        const DocumentState& doc,
        const int line,
        const std::optional<std::uint32_t> deviceIdFilter,
        const std::optional<std::uint64_t> nodeIdFilter,
        const std::string_view typedPrefix
    ) {
        if (nodeIdFilter.has_value()) {
            std::size_t maxInputs = 0;
            bool found = false;
            for (const auto& [key, count] : doc.declaredNodes) {
                if (key.second == nodeIdFilter.value()) {
                    if (!deviceIdFilter.has_value() || key.first == deviceIdFilter.value()) {
                        maxInputs = std::max(maxInputs, count);
                        found = true;
                    }
                }
            }

            if (found) {
                for (std::size_t i = 0; i < maxInputs; ++i) {
                    std::string literal = std::to_string(i);
                    if (!typedPrefix.empty() && literal.substr(0, typedPrefix.size()) != typedPrefix) {
                        continue;
                    }
                    items.push_back({
                        {"label", literal},
                        {"kind", 12}, // 12 = Value
                        {"detail", "Node Input Index"},
                        {"insertText", literal + " "},
                        {"command", makeRetriggerSuggestCommand()}
                    });
                }
            }
        }
        addVariableCompletionItems(items, doc, ValueType::UInt32, line, typedPrefix);
    }

    template<std::size_t N>
    void parsePartialFields(
        const std::string_view priorBody,
        const std::array<std::string_view, N>& fieldNames,
        const std::array<ValueType, N>& fieldTypes,
        const VariableMap& variables,
        std::array<std::uint64_t, N>& outValues,
        std::array<bool, N>& outAssigned
    ) {
        const auto fields = splitWords(priorBody);
        std::size_t nextPos = 0;
        bool blocked = false;
        outAssigned.fill(false);
        outValues.fill(0);

        for (const auto& field : fields) {
            const std::size_t equals = field.text.find('=');
            std::size_t pIndex = 0;
            std::string_view pVal = field.text;

            if (equals != std::string_view::npos) {
                const std::string_view name = field.text.substr(0, equals);
                pVal = field.text.substr(equals + 1);
                bool found = false;
                for(std::size_t i=0; i<N; ++i) {
                    if(fieldNames[i] == name) {
                        pIndex = i;
                        found = true;
                        break;
                    }
                }
                if(!found) continue;
                if(pIndex > nextPos) blocked = true;
            } else {
                if(blocked) continue;
                while(nextPos < N && outAssigned[nextPos]) ++nextPos;
                if(nextPos >= N) continue;
                pIndex = nextPos;
            }

            std::uint64_t val = 0;
            std::string err;
            if (resolveValueToken(variables, pVal, fieldTypes[pIndex], val, err)) {
                outValues[pIndex] = val;
            }
            outAssigned[pIndex] = true;
            while(nextPos < N && outAssigned[nextPos]) ++nextPos;
        }
    }

    static bool hasExactTypedVariable(
        const DocumentState& doc,
        const ValueType expectedType,
        const int line,
        const std::string_view token
    ) {
        if (!isIdentifier(token)) {
            return false;
        }

        const auto variableIt = doc.variables.find(token);
        if (variableIt == doc.variables.end()) {
            return false;
        }

        return variableIt->second.type == expectedType && variableIt->second.declarationLine < line;
    }

    static bool isClosedCompositeToken(const std::string_view token) {
        const std::string_view trimmed = trimView(token);
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

    static bool isUnclosedCompositeToken(const std::string_view token) {
        const std::string_view trimmed = trimView(token);
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

    static bool hasUnclosedCompositeValue(const std::string_view token) {
        const std::size_t equals = token.find('=');
        if (equals == std::string_view::npos) {
            return false;
        }

        return isUnclosedCompositeToken(token.substr(equals + 1));
    }

    static std::string_view getCurrentListItemPrefix(const std::string_view body) {
        const auto itemsInBody = splitWords(body);
        if (itemsInBody.empty()) {
            return std::string_view{};
        }

        const std::string_view rawLastItem = itemsInBody.back().text;
        const std::string_view trimmedLastItem = trimView(rawLastItem);
        if (isUnclosedCompositeToken(rawLastItem)) {
            return rawLastItem;
        }

        const bool trailingWhitespace = !body.empty() && isWhitespace(body.back());
        return trailingWhitespace ? std::string_view{} : trimmedLastItem;
    }

    static json makeRetriggerSuggestCommand() {
        return {
            {"title", "Trigger Suggest"},
            {"command", "editor.action.triggerSuggest"}
        };
    }

    static bool matchesQuotedLiteralPrefix(const std::string_view literal, const std::string_view typedPrefix) {
        if (typedPrefix.empty()) {
            return true;
        }

        if (literal.size() >= typedPrefix.size() && literal.substr(0, typedPrefix.size()) == typedPrefix) {
            return true;
        }

        if (typedPrefix.front() == '"' || literal.empty() || literal.front() != '"') {
            return false;
        }

        const std::string_view unquotedLiteral = literal.substr(1);
        return unquotedLiteral.size() >= typedPrefix.size() && unquotedLiteral.substr(0, typedPrefix.size()) == typedPrefix;
    }

void addKgIpTemplateCompletion(json& items) {
        items.push_back({
            {"label", "<...>"},
            {"kind", 15},
            {"detail", "Inline KgIp literal"},
            {"insertText", "<$1>"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    StructuredCompletionState buildStructuredCompletionState(
        const std::string_view priorBody,
        const std::array<std::string_view, 2>& fieldNames
    ) {
        StructuredCompletionState state;
        const auto fields = splitWords(priorBody);

        for (const auto& field : fields) {
            const std::size_t equals = field.text.find('=');
            if (equals != std::string_view::npos) {
                const std::string_view fieldName = field.text.substr(0, equals);
                for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                    if (fieldNames[index] == fieldName) {
                        if (index > state.nextPositionalIndex) {
                            state.positionalBlocked = true;
                        }
                        markStructuredFieldAssigned(state, index);
                        break;
                    }
                }
                continue;
            }

            if (state.nextPositionalIndex < state.assigned.size()) {
                markStructuredFieldAssigned(state, state.nextPositionalIndex);
            }
        }

        return state;
    }

    template<std::size_t N>
    StructuredCompletionStateN<N> buildStructuredCompletionStateN(
        const std::string_view priorBody,
        const std::array<std::string_view, N>& fieldNames
    ) {
        StructuredCompletionStateN<N> state;
        const auto fields = splitWords(priorBody);

        for (const auto& field : fields) {
            const std::size_t equals = field.text.find('=');
            if (equals != std::string_view::npos) {
                const std::string_view fieldName = field.text.substr(0, equals);
                for (std::size_t index = 0; index < fieldNames.size(); ++index) {
                    if (fieldNames[index] == fieldName) {
                        if (index > state.nextPositionalIndex) {
                            state.positionalBlocked = true;
                        }
                        state.assigned[index] = true;
                        while (state.nextPositionalIndex < state.assigned.size() && state.assigned[state.nextPositionalIndex]) {
                            ++state.nextPositionalIndex;
                        }
                        break;
                    }
                }
                continue;
            }

            if (state.nextPositionalIndex < state.assigned.size()) {
                state.assigned[state.nextPositionalIndex] = true;
                while (state.nextPositionalIndex < state.assigned.size() && state.assigned[state.nextPositionalIndex]) {
                    ++state.nextPositionalIndex;
                }
            }
        }

        return state;
    }

    template<std::size_t N>
    void addStructuredFieldNameCompletions(
        json& items,
        const std::string_view prefix,
        const std::array<std::string_view, N>& fieldNames,
        const std::array<bool, N>& assignedFields,
        const std::string_view detail
    ) {
        for (std::size_t index = 0; index < fieldNames.size(); ++index) {
            if (assignedFields[index]) {
                continue;
            }

            const auto fieldName = fieldNames[index];
            if (!prefix.empty() && fieldName.substr(0, prefix.size()) != prefix) {
                continue;
            }

            items.push_back({
                {"label", std::string(fieldName)},
                {"kind", kCompletionKindKeyword},
                {"detail", std::string(detail)},
                {"insertText", std::string(fieldName) + "="},
                {"command", makeRetriggerSuggestCommand()}
            });
        }
    }

    void addBoolCompletionItems(json& items, const DocumentState& doc, const int line, const std::string_view typedPrefix) {
        const std::array<std::string_view, 2> boolLiterals{"true", "false"};
        for (const auto literal : boolLiterals) {
            if (!typedPrefix.empty() && literal.substr(0, typedPrefix.size()) != typedPrefix) {
                continue;
            }

            items.push_back({
                {"label", std::string(literal)},
                {"kind", kCompletionKindKeyword},
                {"detail", "bool literal"},
                {"insertText", std::string(literal) + " "},
                {"command", makeRetriggerSuggestCommand()}
            });
        }

        addVariableCompletionItems(items, doc, ValueType::Bool, line, typedPrefix);
    }

    void addDirectionCompletionItems(json& items, const DocumentState& doc, const int line, const std::string_view typedPrefix) {
        const std::array<std::string, 2> literals{"\"up\"", "\"down\""};
        for (const auto& literal : literals) {
            if (!matchesQuotedLiteralPrefix(literal, typedPrefix)) {
                continue;
            }

            items.push_back({
                {"label", literal},
                {"kind", kCompletionKindKeyword},
                {"detail", "direction literal"},
                {"insertText", literal + " "},
                {"command", makeRetriggerSuggestCommand()}
            });
        }

        addVariableCompletionItems(items, doc, ValueType::Direction, line, typedPrefix);
    }

    void addAlgoCompletionItems(json& items, const DocumentState& doc, const int line, const std::string_view typedPrefix) {
        for (const auto algoName : getAlgoNames()) {
            const std::string literal = "\"" + std::string(algoName) + "\"";
            if (!matchesQuotedLiteralPrefix(literal, typedPrefix)) {
                continue;
            }

            items.push_back({
                {"label", literal},
                {"kind", 12},
                {"detail", "AlgoType literal"},
                {"insertText", literal + " "},
                {"command", makeRetriggerSuggestCommand()}
            });
        }

        addVariableCompletionItems(items, doc, ValueType::AlgoType, line, typedPrefix);
    }

    void addKgIpFieldNameCompletions(json& items, const std::string_view prefix, const std::array<bool, 2>& assignedFields) {
        const std::array<std::string_view, 2> fieldNames{CoreTypes::Param::Host, CoreTypes::Param::Port};
        addStructuredFieldNameCompletions(items, prefix, fieldNames, assignedFields, "KgIp field");
    }

    void addKgRunTemplateCompletion(json& items) {
        items.push_back({
            {"label", "<...>"},
            {"kind", 15},
            {"detail", "Inline KgRun literal"},
            {"insertText", "<$1>"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addKgRunFieldNameCompletions(json& items, const std::string_view prefix, const std::array<bool, 2>& assignedFields) {
        const std::array<std::string_view, 2> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::ToRun};
        addStructuredFieldNameCompletions(items, prefix, fieldNames, assignedFields, "KgRun field");
    }

    void addKgOutputTemplateCompletion(json& items) {
        items.push_back({
            {"label", "<...>"},
            {"kind", 15},
            {"detail", "Inline KgOutput literal"},
            {"insertText", "<$1>"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addKgInputTemplateCompletion(json& items) {
        items.push_back({
            {"label", "<...>"},
            {"kind", 15},
            {"detail", "Inline KgInput literal"},
            {"insertText", "<$1>"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addListUInt32TemplateCompletion(json& items) {
        items.push_back({
            {"label", "[...]"},
            {"kind", 15},
            {"detail", "Inline []uint32 literal"},
            {"insertText", "[$1]"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addListKgInputTemplateCompletion(json& items) {
        items.push_back({
            {"label", "[...]"},
            {"kind", 15},
            {"detail", "Inline[]KgInput literal"},
            {"insertText", "[$1]"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addKgNodeTemplateCompletion(json& items) {
        items.push_back({
            {"label", "<...>"},
            {"kind", 15},
            {"detail", "Inline KgNode literal"},
            {"insertText", "<$1>"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addKgSensorTemplateCompletion(json& items) {
        items.push_back({
            {"label", "<...>"},
            {"kind", 15},
            {"detail", "Inline KgSensor literal"},
            {"insertText", "<$1>"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addKgBindExternalTemplateCompletion(json& items) {
        items.push_back({
            {"label", "<...>"},
            {"kind", 15},
            {"detail", "Inline KgBindExternal literal"},
            {"insertText", "<$1>"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addKgBindInputTemplateCompletion(json& items) {
        items.push_back({
            {"label", "<...>"},
            {"kind", 15},
            {"detail", "Inline KgBindInput literal"},
            {"insertText", "<$1>"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addListKgBindInputTemplateCompletion(json& items) {
        items.push_back({
            {"label", "[...]"},
            {"kind", 15},
            {"detail", "Inline[]KgBindInput literal"},
            {"insertText", "[$1]"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    void addKgBindTemplateCompletion(json& items) {
        items.push_back({
            {"label", "<...>"},
            {"kind", 15},
            {"detail", "Inline KgBind literal"},
            {"insertText", "<$1>"},
            {"insertTextFormat", 2},
            {"command", makeRetriggerSuggestCommand()}
        });
    }

    static void splitStructuredCompletionPrefix(
        const std::string_view body,
        std::string_view& priorBody,
        std::string_view& currentField
    ) {
        const auto fields = splitWords(body);
        if (body.empty()) {
            priorBody = std::string_view{};
            currentField = std::string_view{};
            return;
        }

        const bool trailingWhitespace = isWhitespace(body.back());
        if (fields.empty()) {
            priorBody = body;
            currentField = std::string_view{};
            return;
        }

        const auto& lastField = fields.back();
        if (trailingWhitespace) {
            if (isUnclosedCompositeToken(lastField.text) || hasUnclosedCompositeValue(lastField.text)) {
                priorBody = body.substr(0, lastField.start);
                currentField = lastField.text;
                return;
            }

            priorBody = body;
            currentField = std::string_view{};
            return;
        }

        priorBody = body.substr(0, lastField.start);
        currentField = lastField.text;
    }

    void addKgIpValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addKgIpTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgIP, line, "");
            return;
        }

        if (prefix.front() != '<') {
            addKgIpTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgIP, line, prefix);
            return;
        }

        const std::string_view body = prefix.substr(1);
        std::string_view priorBody;
        std::string_view currentField;
        splitStructuredCompletionPrefix(body, priorBody, currentField);
        const StructuredCompletionState state = buildStructuredCompletionState(
            priorBody,
            std::array<std::string_view, 2>{CoreTypes::Param::Host, CoreTypes::Param::Port}
        );

        if (currentField.empty()) {
            addKgIpFieldNameCompletions(items, "", state.assigned);
            if (!state.positionalBlocked && state.nextPositionalIndex < 2) {
                addVariableCompletionItems(items, doc, state.nextPositionalIndex == 0 ? ValueType::UInt8 : ValueType::UInt16, line, "");
            }
            return;
        }

        const std::size_t equals = currentField.find('=');
        if (equals != std::string_view::npos) {
            const std::string_view fieldName = currentField.substr(0, equals);
            const std::string_view valuePrefix = currentField.substr(equals + 1);

            if (fieldName == CoreTypes::Param::Host) {
                const std::size_t dot = valuePrefix.find_last_of('.');
                const std::string_view octetPrefix = dot == std::string_view::npos ? valuePrefix : valuePrefix.substr(dot + 1);
                addVariableCompletionItems(items, doc, ValueType::UInt8, line, octetPrefix);
            } else if (fieldName == CoreTypes::Param::Port) {
                addVariableCompletionItems(items, doc, ValueType::UInt16, line, valuePrefix);
            } else {
                addKgIpFieldNameCompletions(items, fieldName, state.assigned);
            }
            return;
        }

        if (currentField.find('.') != std::string_view::npos) {
            const std::size_t dot = currentField.find_last_of('.');
            const std::string_view octetPrefix = currentField.substr(dot + 1);
            addVariableCompletionItems(items, doc, ValueType::UInt8, line, octetPrefix);
            return;
        }

        addKgIpFieldNameCompletions(items, currentField, state.assigned);
        if (!state.positionalBlocked && state.nextPositionalIndex < 2) {
            addVariableCompletionItems(items, doc, state.nextPositionalIndex == 0 ? ValueType::UInt8 : ValueType::UInt16, line, currentField);
        }
    }

    void addKgRunValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addKgRunTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgRun, line, "");
            return;
        }

        if (prefix.front() != '<') {
            addKgRunTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgRun, line, prefix);
            return;
        }

        const std::string_view body = prefix.substr(1);
        std::string_view priorBody;
        std::string_view currentField;
        splitStructuredCompletionPrefix(body, priorBody, currentField);
        const StructuredCompletionState state = buildStructuredCompletionState(
            priorBody,
            std::array<std::string_view, 2>{CoreTypes::Param::Device, CoreTypes::Param::ToRun}
        );

        if (currentField.empty()) {
            addKgRunFieldNameCompletions(items, "", state.assigned);
            if (!state.positionalBlocked && state.nextPositionalIndex < 2) {
                if (state.nextPositionalIndex == 0) {
                    addVariableCompletionItems(items, doc, ValueType::UInt32, line, "");
                } else {
                    addBoolCompletionItems(items, doc, line, "");
                }
            }
            return;
        }

        const std::size_t equals = currentField.find('=');
        if (equals != std::string_view::npos) {
            const std::string_view fieldName = currentField.substr(0, equals);
            const std::string_view valuePrefix = currentField.substr(equals + 1);

            if (fieldName == CoreTypes::Param::Device) {
                addVariableCompletionItems(items, doc, ValueType::UInt32, line, valuePrefix);
            } else if (fieldName == CoreTypes::Param::ToRun) {
                addBoolCompletionItems(items, doc, line, valuePrefix);
            } else {
                addKgRunFieldNameCompletions(items, fieldName, state.assigned);
            }
            return;
        }

        addKgRunFieldNameCompletions(items, currentField, state.assigned);
        if (!state.positionalBlocked && state.nextPositionalIndex < 2) {
            if (state.nextPositionalIndex == 0) {
                addVariableCompletionItems(items, doc, ValueType::UInt32, line, currentField);
            } else {
                addBoolCompletionItems(items, doc, line, currentField);
            }
        }
    }

    void addKgOutputFieldNameCompletions(json& items, const std::string_view prefix, const std::array<bool, 2>& assignedFields) {
        const std::array<std::string_view, 2> fieldNames{CoreTypes::Param::OutC, CoreTypes::Param::OutA};
        addStructuredFieldNameCompletions(items, prefix, fieldNames, assignedFields, "KgOutput field");
    }

    void addKgOutputValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addKgOutputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgOutput, line, "");
            return;
        }

        if (prefix.front() != '<') {
            addKgOutputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgOutput, line, prefix);
            return;
        }

        const std::string_view body = prefix.substr(1);
        std::string_view priorBody;
        std::string_view currentField;
        splitStructuredCompletionPrefix(body, priorBody, currentField);
        const StructuredCompletionState state = buildStructuredCompletionState(
            priorBody,
            std::array<std::string_view, 2>{CoreTypes::Param::OutC, CoreTypes::Param::OutA}
        );

        if (currentField.empty()) {
            addKgOutputFieldNameCompletions(items, "", state.assigned);
            if (!state.positionalBlocked && state.nextPositionalIndex < 2) {
                if (state.nextPositionalIndex == 0) {
                    addDirectionCompletionItems(items, doc, line, "");
                } else {
                    addVariableCompletionItems(items, doc, ValueType::Double01, line, "");
                }
            }
            return;
        }

        const std::size_t equals = currentField.find('=');
        if (equals != std::string_view::npos) {
            const std::string_view fieldName = currentField.substr(0, equals);
            const std::string_view valuePrefix = currentField.substr(equals + 1);

            if (fieldName == CoreTypes::Param::OutC) {
                addDirectionCompletionItems(items, doc, line, valuePrefix);
            } else if (fieldName == CoreTypes::Param::OutA) {
                addVariableCompletionItems(items, doc, ValueType::Double01, line, valuePrefix);
            } else {
                addKgOutputFieldNameCompletions(items, fieldName, state.assigned);
            }
            return;
        }

        addKgOutputFieldNameCompletions(items, currentField, state.assigned);
        if (!state.positionalBlocked && state.nextPositionalIndex < 2) {
            if (state.nextPositionalIndex == 0) {
                addDirectionCompletionItems(items, doc, line, currentField);
            } else {
                addVariableCompletionItems(items, doc, ValueType::Double01, line, currentField);
            }
        }
    }

    void addKgInputFieldNameCompletions(json& items, const std::string_view prefix, const std::array<bool, 5>& assignedFields) {
        const std::array<std::string_view, 5> fieldNames{
            CoreTypes::Param::InC, CoreTypes::Param::InA, CoreTypes::Param::InB, CoreTypes::Param::InV, CoreTypes::Param::InG
        };
        addStructuredFieldNameCompletions(items, prefix, fieldNames, assignedFields, "KgInput field");
    }

    void addKgInputValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addKgInputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgInput, line, "");
            return;
        }

        if (prefix.front() != '<') {
            addKgInputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgInput, line, prefix);
            return;
        }

        const std::string_view body = prefix.substr(1);
        std::string_view priorBody;
        std::string_view currentField;
        splitStructuredCompletionPrefix(body, priorBody, currentField);

        const std::array<std::string_view, 5> fieldNames{
            CoreTypes::Param::InC, CoreTypes::Param::InA, CoreTypes::Param::InB, CoreTypes::Param::InV, CoreTypes::Param::InG
        };
        const StructuredCompletionStateN<5> state = buildStructuredCompletionStateN(priorBody, fieldNames);

        auto completeForIndex = [&](const std::size_t index, const std::string_view valuePrefix) {
            if (index == 0) {
                addDirectionCompletionItems(items, doc, line, valuePrefix);
            } else if (index == 1 || index == 3 || index == 4) {
                addVariableCompletionItems(items, doc, ValueType::Double01, line, valuePrefix);
            } else if (index == 2) {
                addVariableCompletionItems(items, doc, ValueType::UInt32, line, valuePrefix);
            }
        };

        if (currentField.empty()) {
            addKgInputFieldNameCompletions(items, "", state.assigned);
            if (!state.positionalBlocked && state.nextPositionalIndex < 5) {
                completeForIndex(state.nextPositionalIndex, "");
            }
            return;
        }

        const std::size_t equals = currentField.find('=');
        if (equals != std::string_view::npos) {
            const std::string_view fieldName = currentField.substr(0, equals);
            const std::string_view valuePrefix = currentField.substr(equals + 1);

            bool found = false;
            for (std::size_t i = 0; i < fieldNames.size(); ++i) {
                if (fieldNames[i] == fieldName) {
                    completeForIndex(i, valuePrefix);
                    found = true;
                    break;
                }
            }

            if (!found) {
                addKgInputFieldNameCompletions(items, fieldName, state.assigned);
            }
            return;
        }

        addKgInputFieldNameCompletions(items, currentField, state.assigned);
        if (!state.positionalBlocked && state.nextPositionalIndex < 5) {
            completeForIndex(state.nextPositionalIndex, currentField);
        }
    }

    void addListUInt32ValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addListUInt32TemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::ListUInt32, line, "");
            return;
        }

        if (prefix.front() != '[') {
            addListUInt32TemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::ListUInt32, line, prefix);
            return;
        }

        const std::string_view itemPrefix = getCurrentListItemPrefix(prefix.substr(1));
        addVariableCompletionItems(items, doc, ValueType::UInt32, line, itemPrefix);
    }

    void addListKgInputValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addListKgInputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::ListKgInput, line, "");
            return;
        }

        if (prefix.front() != '[') {
            addListKgInputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::ListKgInput, line, prefix);
            return;
        }

        const std::string_view itemPrefix = getCurrentListItemPrefix(prefix.substr(1));
        addKgInputValueCompletions(items, doc, line, itemPrefix);
    }

    void addKgNodeFieldNameCompletions(json& items, const std::string_view prefix, const std::array<bool, 5>& assignedFields) {
        const std::array<std::string_view, 5> fieldNames{
            CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::Algo, CoreTypes::Param::Output, CoreTypes::Param::Inputs
        };
        addStructuredFieldNameCompletions(items, prefix, fieldNames, assignedFields, "KgNode field");
    }

    void addKgNodeValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addKgNodeTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgNode, line, "");
            return;
        }

        if (prefix.front() != '<') {
            addKgNodeTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgNode, line, prefix);
            return;
        }

        const std::string_view body = prefix.substr(1);
        std::string_view priorBody;
        std::string_view currentField;
        splitStructuredCompletionPrefix(body, priorBody, currentField);

        const std::array<std::string_view, 5> fieldNames{
            CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::Algo, CoreTypes::Param::Output, CoreTypes::Param::Inputs
        };
        const StructuredCompletionStateN<5> state = buildStructuredCompletionStateN(priorBody, fieldNames);

        auto completeForIndex = [&](const std::size_t index, const std::string_view valuePrefix) {
            if (index == 0) {
                addVariableCompletionItems(items, doc, ValueType::UInt32, line, valuePrefix);
            } else if (index == 1) {
                addVariableCompletionItems(items, doc, ValueType::KgNodeId, line, valuePrefix);
            } else if (index == 2) {
                addAlgoCompletionItems(items, doc, line, valuePrefix);
            } else if (index == 3) {
                addKgOutputValueCompletions(items, doc, line, valuePrefix);
            } else if (index == 4) {
                addListKgInputValueCompletions(items, doc, line, valuePrefix);
            }
        };

        if (currentField.empty()) {
            addKgNodeFieldNameCompletions(items, "", state.assigned);
            if (!state.positionalBlocked && state.nextPositionalIndex < 5) {
                completeForIndex(state.nextPositionalIndex, "");
            }
            return;
        }

        const std::size_t equals = currentField.find('=');
        if (equals != std::string_view::npos) {
            const std::string_view fieldName = currentField.substr(0, equals);
            const std::string_view valuePrefix = currentField.substr(equals + 1);

            bool found = false;
            for (std::size_t i = 0; i < fieldNames.size(); ++i) {
                if (fieldNames[i] == fieldName) {
                    completeForIndex(i, valuePrefix);
                    found = true;
                    break;
                }
            }

            if (!found) {
                addKgNodeFieldNameCompletions(items, fieldName, state.assigned);
            }
            return;
        }

        addKgNodeFieldNameCompletions(items, currentField, state.assigned);
        if (!state.positionalBlocked && state.nextPositionalIndex < 5) {
            completeForIndex(state.nextPositionalIndex, currentField);
        }
    }

    void addKgSensorFieldNameCompletions(json& items, const std::string_view prefix, const std::array<bool, 4>& assignedFields) {
        const std::array<std::string_view, 4> fieldNames{"deviceId", "id", "period", "toEmit"};
        addStructuredFieldNameCompletions(items, prefix, fieldNames, assignedFields, "KgSensor field");
    }

    void addKgSensorValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addKgSensorTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgSensor, line, "");
            return;
        }

        if (prefix.front() != '<') {
            addKgSensorTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgSensor, line, prefix);
            return;
        }

        const std::string_view body = prefix.substr(1);
        std::string_view priorBody;
        std::string_view currentField;
        splitStructuredCompletionPrefix(body, priorBody, currentField);
        const std::array<std::string_view, 4> fieldNames{"deviceId", "id", "period", "toEmit"};
        const StructuredCompletionStateN<4> state = buildStructuredCompletionStateN<4>(priorBody, fieldNames);

        auto completeForIndex = [&](const std::size_t index, const std::string_view valuePrefix) {
            if (index == 0 || index == 1 || index == 2) {
                addVariableCompletionItems(items, doc, ValueType::UInt32, line, valuePrefix);
            } else if (index == 3) {
                addVariableCompletionItems(items, doc, ValueType::Double01, line, valuePrefix);
            }
        };

        if (currentField.empty()) {
            addKgSensorFieldNameCompletions(items, "", state.assigned);
            if (!state.positionalBlocked && state.nextPositionalIndex < 4) {
                completeForIndex(state.nextPositionalIndex, "");
            }
            return;
        }

        const std::size_t equals = currentField.find('=');
        if (equals != std::string_view::npos) {
            const std::string_view fieldName = currentField.substr(0, equals);
            const std::string_view valuePrefix = currentField.substr(equals + 1);

            bool found = false;
            for (std::size_t i = 0; i < fieldNames.size(); ++i) {
                if (fieldNames[i] == fieldName) {
                    completeForIndex(i, valuePrefix);
                    found = true;
                    break;
                }
            }

            if (!found) {
                addKgSensorFieldNameCompletions(items, fieldName, state.assigned);
            }
            return;
        }

        addKgSensorFieldNameCompletions(items, currentField, state.assigned);
        if (!state.positionalBlocked && state.nextPositionalIndex < 4) {
            completeForIndex(state.nextPositionalIndex, currentField);
        }
    }

    void addKgBindExternalValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addKgBindExternalTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgBindExternal, line, "");
            return;
        }

        if (prefix.front() != '<') {
            addKgBindExternalTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgBindExternal, line, prefix);
            return;
        }

        const std::string_view body = prefix.substr(1);
        std::string_view priorBody;
        std::string_view currentField;
        splitStructuredCompletionPrefix(body, priorBody, currentField);

        const std::array<std::string_view, 3> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::To};
        const std::array<ValueType, 3> fieldTypes{ValueType::UInt32, ValueType::KgNodeId, ValueType::UInt32};
        std::array<std::uint64_t, 3> parsedValues{0, 0, 0};
        std::array<bool, 3> parsedAssigned{false, false, false};

        parsePartialFields<3>(priorBody, fieldNames, fieldTypes, doc.variables, parsedValues, parsedAssigned);
        const StructuredCompletionStateN<3> state = buildStructuredCompletionStateN<3>(priorBody, fieldNames);

        auto completeForIndex = [&](const std::size_t index, const std::string_view valuePrefix) {
            if (index == 0) {
                addVariableCompletionItems(items, doc, ValueType::UInt32, line, valuePrefix);
            } else if (index == 1) {
                std::optional<std::uint32_t> devIdFilter;
                if (parsedAssigned[0]) devIdFilter = static_cast<std::uint32_t>(parsedValues[0]);
                addNodeIdCompletionItems(items, doc, line, devIdFilter, valuePrefix);
            } else if (index == 2) {
                addVariableCompletionItems(items, doc, ValueType::UInt32, line, valuePrefix);
            }
        };

        if (currentField.empty()) {
            addStructuredFieldNameCompletions(items, "", fieldNames, state.assigned, "KgBindExternal field");
            if (!state.positionalBlocked && state.nextPositionalIndex < 3) {
                completeForIndex(state.nextPositionalIndex, "");
            }
            return;
        }

        const std::size_t equals = currentField.find('=');
        if (equals != std::string_view::npos) {
            const std::string_view fieldName = currentField.substr(0, equals);
            const std::string_view valuePrefix = currentField.substr(equals + 1);

            bool found = false;
            for (std::size_t i = 0; i < fieldNames.size(); ++i) {
                if (fieldNames[i] == fieldName) {
                    completeForIndex(i, valuePrefix);
                    found = true;
                    break;
                }
            }

            if (!found) {
                addStructuredFieldNameCompletions(items, fieldName, fieldNames, state.assigned, "KgBindExternal field");
            }
            return;
        }

        addStructuredFieldNameCompletions(items, currentField, fieldNames, state.assigned, "KgBindExternal field");
        if (!state.positionalBlocked && state.nextPositionalIndex < 3) {
            completeForIndex(state.nextPositionalIndex, currentField);
        }
    }

    void addKgBindInputValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addKgBindInputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgBindInput, line, "");
            return;
        }

        if (prefix.front() != '<') {
            addKgBindInputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgBindInput, line, prefix);
            return;
        }

        const std::string_view body = prefix.substr(1);
        std::string_view priorBody;
        std::string_view currentField;
        splitStructuredCompletionPrefix(body, priorBody, currentField);

        const std::array<std::string_view, 3> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::In};
        const std::array<ValueType, 3> fieldTypes{ValueType::UInt32, ValueType::KgNodeId, ValueType::UInt32};
        std::array<std::uint64_t, 3> parsedValues{0, 0, 0};
        std::array<bool, 3> parsedAssigned{false, false, false};

        parsePartialFields<3>(priorBody, fieldNames, fieldTypes, doc.variables, parsedValues, parsedAssigned);
        const StructuredCompletionStateN<3> state = buildStructuredCompletionStateN<3>(priorBody, fieldNames);

        auto completeForIndex = [&](const std::size_t index, const std::string_view valuePrefix) {
            if (index == 0) {
                addVariableCompletionItems(items, doc, ValueType::UInt32, line, valuePrefix);
            } else if (index == 1) {
                std::optional<std::uint32_t> devIdFilter;
                if (parsedAssigned[0]) devIdFilter = static_cast<std::uint32_t>(parsedValues[0]);
                addNodeIdCompletionItems(items, doc, line, devIdFilter, valuePrefix);
            } else if (index == 2) {
                std::optional<std::uint32_t> devIdFilter;
                std::optional<std::uint64_t> nodeIdFilter;
                if (parsedAssigned[0]) devIdFilter = static_cast<std::uint32_t>(parsedValues[0]);
                if (parsedAssigned[1]) nodeIdFilter = parsedValues[1];
                addInputIndexCompletionItems(items, doc, line, devIdFilter, nodeIdFilter, valuePrefix);
            }
        };

        if (currentField.empty()) {
            addStructuredFieldNameCompletions(items, "", fieldNames, state.assigned, "KgBindInput field");
            if (!state.positionalBlocked && state.nextPositionalIndex < 3) {
                completeForIndex(state.nextPositionalIndex, "");
            }
            return;
        }

        const std::size_t equals = currentField.find('=');
        if (equals != std::string_view::npos) {
            const std::string_view fieldName = currentField.substr(0, equals);
            const std::string_view valuePrefix = currentField.substr(equals + 1);

            bool found = false;
            for (std::size_t i = 0; i < fieldNames.size(); ++i) {
                if (fieldNames[i] == fieldName) {
                    completeForIndex(i, valuePrefix);
                    found = true;
                    break;
                }
            }

            if (!found) {
                addStructuredFieldNameCompletions(items, fieldName, fieldNames, state.assigned, "KgBindInput field");
            }
            return;
        }

        addStructuredFieldNameCompletions(items, currentField, fieldNames, state.assigned, "KgBindInput field");
        if (!state.positionalBlocked && state.nextPositionalIndex < 3) {
            completeForIndex(state.nextPositionalIndex, currentField);
        }
    }

    void addListKgBindInputValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addListKgBindInputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::ListKgBindInput, line, "");
            return;
        }

        if (prefix.front() != '[') {
            addListKgBindInputTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::ListKgBindInput, line, prefix);
            return;
        }

        const std::string_view itemPrefix = getCurrentListItemPrefix(prefix.substr(1));
        addKgBindInputValueCompletions(items, doc, line, itemPrefix);
    }

    void addKgBindValueCompletions(json& items, const DocumentState& doc, const int line, const std::string_view prefix) {
        if (prefix.empty()) {
            addKgBindTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgBind, line, "");
            return;
        }

        if (prefix.front() != '<') {
            addKgBindTemplateCompletion(items);
            addVariableCompletionItems(items, doc, ValueType::KgBind, line, prefix);
            return;
        }

        const std::string_view body = prefix.substr(1);
        std::string_view priorBody;
        std::string_view currentField;
        splitStructuredCompletionPrefix(body, priorBody, currentField);

        const std::array<std::string_view, 3> fieldNames{CoreTypes::Param::Device, CoreTypes::Param::Id, CoreTypes::Param::To};
        const std::array<ValueType, 3> fieldTypes{ValueType::UInt32, ValueType::KgNodeId, ValueType::ListKgBindInput};
        std::array<std::uint64_t, 3> parsedValues{0, 0, 0};
        std::array<bool, 3> parsedAssigned{false, false, false};

        parsePartialFields<3>(priorBody, fieldNames, fieldTypes, doc.variables, parsedValues, parsedAssigned);
        const StructuredCompletionStateN<3> state = buildStructuredCompletionStateN<3>(priorBody, fieldNames);

        auto completeForIndex = [&](const std::size_t index, const std::string_view valuePrefix) {
            if (index == 0) {
                addVariableCompletionItems(items, doc, ValueType::UInt32, line, valuePrefix);
            } else if (index == 1) {
                std::optional<std::uint32_t> devIdFilter;
                if (parsedAssigned[0]) devIdFilter = static_cast<std::uint32_t>(parsedValues[0]);
                addNodeIdCompletionItems(items, doc, line, devIdFilter, valuePrefix);
            } else if (index == 2) {
                addListKgBindInputValueCompletions(items, doc, line, valuePrefix);
            }
        };

        if (currentField.empty()) {
            addStructuredFieldNameCompletions(items, "", fieldNames, state.assigned, "KgBind field");
            if (!state.positionalBlocked && state.nextPositionalIndex < 3) {
                completeForIndex(state.nextPositionalIndex, "");
            }
            return;
        }

        const std::size_t equals = currentField.find('=');
        if (equals != std::string_view::npos) {
            const std::string_view fieldName = currentField.substr(0, equals);
            const std::string_view valuePrefix = currentField.substr(equals + 1);

            bool found = false;
            for (std::size_t i = 0; i < fieldNames.size(); ++i) {
                if (fieldNames[i] == fieldName) {
                    completeForIndex(i, valuePrefix);
                    found = true;
                    break;
                }
            }

            if (!found) {
                addStructuredFieldNameCompletions(items, fieldName, fieldNames, state.assigned, "KgBind field");
            }
            return;
        }

        addStructuredFieldNameCompletions(items, currentField, fieldNames, state.assigned, "KgBind field");
        if (!state.positionalBlocked && state.nextPositionalIndex < 3) {
            completeForIndex(state.nextPositionalIndex, currentField);
        }
    }

    void getCompletionsForType(
        json& items,
        const DocumentState& doc,
        const ValueType valueType,
        const int line,
        const std::string_view prefix
    ) {
        switch (valueType) {
            case ValueType::Bool:
                addBoolCompletionItems(items, doc, line, prefix);
                break;
            case ValueType::Direction:
                addDirectionCompletionItems(items, doc, line, prefix);
                break;
            case ValueType::AlgoType:
                addAlgoCompletionItems(items, doc, line, prefix);
                break;
            case ValueType::KgIP:
                addKgIpValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::KgRun:
                addKgRunValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::KgOutput:
                addKgOutputValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::KgInput:
                addKgInputValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::ListUInt32:
                addListUInt32ValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::ListKgInput:
                addListKgInputValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::KgNode:
                addKgNodeValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::KgSensor:
                addKgSensorValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::KgBindExternal:
                addKgBindExternalValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::KgBindInput:
                addKgBindInputValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::ListKgBindInput:
                addListKgBindInputValueCompletions(items, doc, line, prefix);
                break;
            case ValueType::KgBind:
                addKgBindValueCompletions(items, doc, line, prefix);
                break;
            default:
                addVariableCompletionItems(items, doc, valueType, line, prefix);
                break;
        }
    }

    void handleCompletion(const json& id, const json& params) {
        const std::string uri = params["textDocument"]["uri"];
        const int line = params["position"]["line"];
        const int character = params["position"]["character"];

        auto it = documents.find(uri);
        if (it == documents.end() || line < 0 || line >= static_cast<int>(it->second.lines.size())) {
            writeLSPMessage(id, json::array());
            return;
        }

        const DocumentState& doc = it->second;
        std::string_view textBeforeCursor;
        const LogicalStatement* statement = findLogicalStatementForLine(doc, line);

        if (statement != nullptr) {
            std::size_t logicalOffset = logicalOffsetForPosition(*statement, line, character);
            textBeforeCursor = std::string_view(statement->text.data(), logicalOffset);
        } else {
            const std::string& lineText = doc.lines[line];
            const int safeCharacter = std::min(character, static_cast<int>(lineText.size()));
            textBeforeCursor = std::string_view(lineText.data(), safeCharacter);
        }

        json items = json::array();
        const auto words = splitWords(textBeforeCursor);

        if (words.empty() || (words.size() == 1 && !isWhitespace(textBeforeCursor.back()))) {
            const std::string_view prefix = words.empty() ? "" : words[0].text;

            for (const auto& command : kAllCommands) {
                if (!prefix.empty() && command.keyword.substr(0, prefix.size()) != prefix) continue;
                items.push_back({
                    {"label", std::string(command.keyword)},
                    {"kind", kCompletionKindKeyword},
                    {"detail", std::string(command.description)},
                    {"insertText", std::string(command.keyword) + " "},
                    {"command", makeRetriggerSuggestCommand()}
                });
            }

            const std::string_view letKeyword = CoreTypes::Keyword::Let;
            if (prefix.empty() || letKeyword.substr(0, prefix.size()) == prefix) {
                items.push_back({
                    {"label", std::string(letKeyword)},
                    {"kind", kCompletionKindKeyword},
                    {"detail", "Declare a variable"},
                    {"insertText", std::string(letKeyword) + " "},
                    {"command", makeRetriggerSuggestCommand()}
                });
            }
        } else if (words.front().text == CoreTypes::Keyword::Let) {
            if (words.size() == 1 || (words.size() == 2 && !isWhitespace(textBeforeCursor.back()))) {
                const std::string_view prefix = words.size() == 2 ? words[1].text : "";
                for (const auto& type : kSupportedTypes) {
                    if (!prefix.empty() && type.keyword.substr(0, prefix.size()) != prefix) continue;
                    items.push_back({
                        {"label", std::string(type.keyword)},
                        {"kind", kCompletionKindKeyword},
                        {"detail", std::string(type.description)},
                        {"insertText", std::string(type.keyword) + " "},
                        {"command", makeRetriggerSuggestCommand()}
                    });
                }
            } else if (words.size() >= 4 && words[3].text == "=") {
                const std::string_view typeKeyword = words[1].text;
                const TypeInfo* declaredType = findTypeInfoByKeyword(typeKeyword);
                if (declaredType != nullptr) {
                    const std::size_t valueStart = words[3].end;
                    std::size_t offset = valueStart;
                    while (offset < textBeforeCursor.size() && isWhitespace(textBeforeCursor[offset])) {
                        ++offset;
                    }
                    const std::string_view prefix = textBeforeCursor.substr(offset);
                    getCompletionsForType(items, doc, declaredType->valueType, line, prefix);
                }
            }
        } else {
            const CommandInfo* command = findCommandInfo(words.front().text);
            if (command != nullptr) {
                const std::size_t valueStart = words[0].end;
                std::size_t offset = valueStart;
                while (offset < textBeforeCursor.size() && isWhitespace(textBeforeCursor[offset])) {
                    ++offset;
                }
                const std::string_view prefix = textBeforeCursor.substr(offset);
                getCompletionsForType(items, doc, command->valueType, line, prefix);
            }
        }

        writeLSPMessage(id, items);
    }

    void handleHover(const json& id, const json& params) {
        const std::string uri = params["textDocument"]["uri"];
        const int line = params["position"]["line"];
        const int character = params["position"]["character"];

        auto it = documents.find(uri);
        if (it == documents.end()) {
            writeLSPMessage(id, nullptr);
            return;
        }

        const DocumentState& doc = it->second;
        for (const auto& hover : doc.hovers) {
            if (line >= hover.startLine && line <= hover.endLine) {
                bool inRange = true;
                if (line == hover.startLine && character < hover.startChar) inRange = false;
                if (line == hover.endLine && character > hover.endChar) inRange = false;

                if (inRange) {
                    writeLSPMessage(id, {
                        {"contents", {
                            {"kind", "markdown"},
                            {"value", hover.markdown}
                        }},
                        {"range", {
                            {"start", {{"line", hover.startLine}, {"character", hover.startChar}}},
                            {"end", {{"line", hover.endLine}, {"character", hover.endChar}}}
                        }}
                    });
                    return;
                }
            }
        }

        writeLSPMessage(id, nullptr);
    }

    void handleDocumentDiagnostic(const json& id, const json& params) {
        const std::string uri = params["textDocument"]["uri"];

        auto it = documents.find(uri);
        if (it == documents.end()) {
            writeLSPMessage(id, {
                {"kind", "full"},
                {"items", json::array()}
            });
            return;
        }

        json items = json::array();
        for (const auto& diag : it->second.diagnostics) {
            int severity = diag.severity == "error" ? 1 : 2;
            items.push_back({
                {"range", {
                    {"start", {{"line", diag.startLine}, {"character", diag.startChar}}},
                    {"end", {{"line", diag.endLine}, {"character", diag.endChar}}}
                }},
                {"severity", severity},
                {"message", diag.message}
            });
        }

        writeLSPMessage(id, {
            {"kind", "full"},
            {"items", items}
        });
    }

    void handleSemanticTokens(const json& id, const json& params) {
        const std::string uri = params["textDocument"]["uri"];

        auto it = documents.find(uri);
        if (it == documents.end()) {
            writeLSPMessage(id, {{"data", json::array()}});
            return;
        }

        std::vector<int> data;
        int prevLine = 0;
        int prevChar = 0;

        auto sortedTokens = it->second.tokens;
        std::sort(sortedTokens.begin(), sortedTokens.end(),[](const SemanticToken& a, const SemanticToken& b) {
            if (a.line != b.line) return a.line < b.line;
            return a.startChar < b.startChar;
        });

        for (const auto& token : sortedTokens) {
            int deltaLine = token.line - prevLine;
            int deltaChar = deltaLine == 0 ? token.startChar - prevChar : token.startChar;

            data.push_back(deltaLine);
            data.push_back(deltaChar);
            data.push_back(token.length);
            data.push_back(static_cast<int>(token.type));
            data.push_back(0);

            prevLine = token.line;
            prevChar = token.startChar;
        }

        writeLSPMessage(id, {{"data", data}});
    }

    void handleDidOpen(const json& params) {
        const std::string uri = params["textDocument"]["uri"];
        const std::string text = params["textDocument"]["text"];
        int version = params["textDocument"].value("version", 0);

        DocumentState doc;
        doc.uri = uri;
        doc.content = text;
        doc.version = version;

        analyzeDocument(doc);
        documents[uri] = std::move(doc);
    }

    void handleDidChange(const json& params) {
        const std::string uri = params["textDocument"]["uri"];
        int version = params["textDocument"].value("version", 0);

        auto it = documents.find(uri);
        if (it != documents.end()) {
            const auto& changes = params["contentChanges"];
            if (!changes.empty()) {
                it->second.content = changes.back()["text"];
                it->second.version = version;
                analyzeDocument(it->second);
            }
        }
    }

    void handleDidClose(const json& params) {
        documents.erase(params["textDocument"]["uri"]);
    }

    void handleShutdown(const json& id) {
        writeLSPMessage(id, nullptr);
    }

    void writeLSPMessage(const json& id, const json& result) {
        json response = {
            {"jsonrpc", "2.0"},
            {"id", id},
            {"result", result}
        };
        std::string content = response.dump();
        std::cout << "Content-Length: " << content.length() << "\r\n\r\n" << content << std::flush;
    }

    std::optional<std::string> readLSPMessage() {
        std::string line;
        int contentLength = 0;

        while (std::getline(std::cin, line)) {
            if (line.empty() || line == "\r") break;

            if (line.rfind("Content-Length: ", 0) == 0) {
                contentLength = std::stoi(line.substr(16));
            }
        }

        if (contentLength > 0) {
            std::string content(contentLength, '\0');
            std::cin.read(&content[0], contentLength);
            return content;
        }
        return std::nullopt;
    }

public:
    void run() {
#ifdef _WIN32
        _setmode(_fileno(stdin), _O_BINARY);
        _setmode(_fileno(stdout), _O_BINARY);
#endif

        log("RPConfig LSP Server started");

        while (true) {
            auto content = readLSPMessage();
            if (!content) break;

            try {
                auto request = json::parse(*content);
                const std::string method = request.value("method", "");
                auto id = request.value("id", json(nullptr));
                auto params = request.value("params", json::object());

                if (method == "initialize") {
                    writeLSPMessage(id, {
                        {"capabilities", {
                            {"textDocumentSync", 1},
                            {"completionProvider", {
                                {"resolveProvider", false},
                                {"triggerCharacters", {".", "=", "<", "[", " "}}
                            }},
                            {"hoverProvider", true},
                            {"diagnosticProvider", {
                                {"interFileDependencies", false},
                                {"workspaceDiagnostics", false}
                            }},
                            {"semanticTokensProvider", {
                                {"legend", {
                                    {"tokenTypes", {
                                        "keyword", "type", "number", "string", "comment",
                                        "error", "operator", "property", "variable"
                                    }},
                                    {"tokenModifiers", json::array()}
                                }},
                                {"full", true}
                            }}
                        }}
                    });
                } else if (method == "initialized") {
                    log("Client initialized notification received");
                } else if (method == "textDocument/didOpen") {
                    handleDidOpen(params);
                } else if (method == "textDocument/didChange") {
                    handleDidChange(params);
                } else if (method == "textDocument/didClose") {
                    handleDidClose(params);
                } else if (method == "textDocument/completion") {
                    handleCompletion(id, params);
                } else if (method == "textDocument/hover") {
                    handleHover(id, params);
                } else if (method == "textDocument/diagnostic") {
                    handleDocumentDiagnostic(id, params);
                } else if (method == "textDocument/semanticTokens/full") {
                    handleSemanticTokens(id, params);
                } else if (method == "shutdown") {
                    handleShutdown(id);
                } else if (method == "exit") {
                    break;
                }
            } catch (const std::exception& error) {
                log("Error processing request: " + std::string(error.what()));
            }
        }

        log("RPConfig LSP Server stopped");
    }
};

int main() {
    static_assert(NodeSystem::Core::CommandList::count >= 2, "Command list must contain config commands.");

    RPConfigLSP server;
    server.run();
    return 0;
}