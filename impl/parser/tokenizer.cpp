// impl/parse/tokenizer.cpp
#include "Nova/parse/tokenizer.h"
#include "Nova/parse/keywords.h"
#include "Nova/parse/tokens.h"
#include "Nova/memory/arena_c_functions.h"
#include "Nova/utils/helpers.h"
#include <string_view>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdint>

namespace nova::detail {

struct LexError {
    const char* msg;      // ← agora é const char*, não string_view
    NovaInfo info;
};

using namespace nova::helpers;

// Helper: format small integers to string (base 10)
static void uint_to_str(char* buf, size_t buf_size, uint64_t value) {
    if (buf_size == 0) return;
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    char temp[21];
    size_t len = 0;
    while (value > 0 && len < sizeof(temp) - 1) {
        temp[len++] = static_cast<uint64_t>('0') + (value % 10);
        value /= 10;
    }
    size_t out_len = (len < buf_size) ? len : buf_size - 1;
    for (size_t i = 0; i < out_len; ++i) {
        buf[i] = temp[len - 1 - i];
    }
    buf[out_len] = '\0';
}

// Helper: create error message in Arena
static const char* make_error_msg(NovaArena* arena, const char* prefix, uint64_t line) {
    // Estimate max length: prefix + up to 20 digits + suffix
    char stack_buf[128];
    size_t pos = 0;
    const char* p = prefix;
    while (*p && pos < sizeof(stack_buf) - 1) {
        stack_buf[pos++] = *p++;
    }
    uint_to_str(stack_buf + pos, sizeof(stack_buf) - pos, line);
    size_t total_len = strlen(stack_buf);
    char* msg = static_cast<char*>(nova_arena_alloc(arena, total_len + 1));
    if (msg) {
        std::memcpy(msg, stack_buf, total_len + 1);
    }
    return msg;
}

static NovaToken make_token(NovaArena* arena, NovaTokenType type, std::string_view lexeme, NovaInfo info) {
    char* buf = static_cast<char*>(nova_arena_alloc(arena, lexeme.size()));
    if (buf && !lexeme.empty()) {
        std::memcpy(buf, lexeme.data(), lexeme.size());
    }
    return NovaToken{
        .value = buf,
        .value_len = lexeme.size(),
        .info = info,
        .token = type
    };
}

static bool isSpace(unsigned char c) { return ::isspace(c); }
static bool isAlpha(unsigned char c) { return ::isalpha(c); }
static bool isDigit(unsigned char c) { return ::isdigit(c); }
static bool isAlphaNum(unsigned char c) { return ::isalnum(c); }
static unsigned char toLower(unsigned char c) { return ::tolower(c); }

// Forward declarations
static void processIdentifier(const char*& current, const char* end,
                              std::vector<NovaToken>& tokens, NovaInfo& info,
                              NovaArena* arena);
static void processString(const char*& current, const char* end,
                          std::vector<NovaToken>& tokens, std::vector<LexError>& errors,
                          NovaInfo& info, NovaArena* arena);
static void processNumber(const char*& current, const char* end,
                          std::vector<NovaToken>& tokens, NovaInfo& info,
                          std::vector<LexError>& errors, NovaArena* arena);
static bool punctuation(const char*& current, const char* end,
                        std::vector<NovaToken>& tokens, NovaInfo& info,
                        NovaArena* arena);

static void skipSingleLine(NovaInfo& info, const char*& current, const char* end) {
    while (current < end && *current != '\n') {
        ++current;
        ++info.index;
    }
}

static void skipMultiLine(NovaInfo& info, const char*& current, const char* end,
                          std::vector<LexError>& errors, NovaArena* arena) {
    NovaInfo start = info;
    current += 2;
    info.index += 2;

    while (current < end) {
        if (*current == '*' && current + 1 < end && *(current + 1) == '/') {
            current += 2;
            info.index += 2;
            return;
        }
        if (*current == '\n') {
            NOVA_NEWLINE(&info);
        }
        ++current;
        ++info.index;
    }
    const char* msg = make_error_msg(arena, "Unterminated multi-line comment starting at line ", start.line);
    errors.push_back({msg, start});
}

static void addError(std::vector<LexError>& errors, const char* base_msg, char c, uint64_t line, NovaArena* arena) {
    char stack_buf[64];
    size_t pos = 0;
    const char* p = base_msg;
    while (*p && pos < sizeof(stack_buf) - 5) {
        stack_buf[pos++] = *p++;
    }
    stack_buf[pos++] = '\'';
    stack_buf[pos++] = c;
    stack_buf[pos++] = '\'';
    stack_buf[pos] = '\0';
    const char* prefix = stack_buf;
    const char* msg = make_error_msg(arena, prefix, line);
    errors.push_back({msg, NovaInfo{1, line}});
}

static void processIdentifier(const char*& current, const char* end,
                              std::vector<NovaToken>& tokens, NovaInfo& info,
                              NovaArena* arena) {
    const char* start = current;
    while (current < end && (isAlphaNum(*current) || *current == '_')) {
        ++current;
        ++info.index;
    }
    std::string_view lexeme(start, current - start);
    NovaTokenType type = nova_lookup_keyword(start, current - start);
    if (type == NOVA_TOKEN_IDENTIFIER) {
        type = NOVA_TOKEN_IDENTIFIER;
    }
    tokens.push_back(make_token(arena, type, lexeme, info));
}

static void processString(const char*& current, const char* end,
                          std::vector<NovaToken>& tokens, std::vector<LexError>& errors,
                          NovaInfo& info, NovaArena* arena) {
    NovaInfo startInfo = info;
    const char* start = current;
    ++current; ++info.index;

    while (current < end) {
        if (*current == '"') {
            ++current; ++info.index;
            tokens.push_back(make_token(arena, NOVA_TOKEN_STRING, std::string_view(start, current - start), startInfo));
            return;
        }
        if (*current == '\\') {
            ++current; ++info.index;
            if (current < end) {
                ++current; ++info.index;
                continue;
            }
        }
        if (*current == '\n') {
            NOVA_NEWLINE(&info);
        }
        ++current;
        ++info.index;
    }
    const char* msg = make_error_msg(arena, "Unterminated string at line ", info.line);
    errors.push_back({msg, info});
    tokens.push_back(make_token(arena, NOVA_TOKEN_STRING, std::string_view(start, current - start), startInfo));
}

static void processNumber(const char*& current, const char* end,
                          std::vector<NovaToken>& tokens, NovaInfo& info,
                          std::vector<LexError>& errors, NovaArena* arena) {
    const char* start = current;
    const NovaInfo startInfo = info;

    bool isHex = false, isBin = false, isFloat = false;

    if (*current == '0' && current + 1 < end) {
        if (const char next = static_cast<char>(toLower(*(current + 1))); next == 'x') { isHex = true; current += 2; info.index += 2; }
        else if (next == 'b') { isBin = true; current += 2; info.index += 2; }
    }

    while (current < end) {
        const char c = *current;
        if (c == '_') { ++current; ++info.index; continue; }

        if (isHex) {
            if (!::isxdigit(c)) break;
        } else if (isBin) {
            if (c != '0' && c != '1') break;
        } else {
            if (c == '.') {
                if (isFloat) break;
                if (!(current + 1 < end && isDigit(*(current + 1)))) break;
                isFloat = true;
            } else if (!isDigit(c)) {
                break;
            }
        }
        ++current; ++info.index;
    }

    NovaTokenType type = NOVA_TOKEN_NUMBER;
    if (isHex) type = NOVA_TOKEN_HEXADECIMAL;
    else if (isBin) type = NOVA_TOKEN_BINARY;
    else if (isFloat) type = NOVA_TOKEN_FLOAT;

    tokens.push_back(make_token(arena, type, std::string_view(start, current - start), startInfo));
}

static bool punctuation(const char*& current, const char* end,
                        std::vector<NovaToken>& tokens, NovaInfo& info,
                        NovaArena* arena) {
    for (const int len : {3, 2, 1}) {
        if (current + len > end) continue;
        if (const auto t = nova_lookup_keyword(current, len); t != NOVA_TOKEN_IDENTIFIER) {
            const std::string_view view(current, len);
            current += len;
            info.index += len;
            tokens.push_back(make_token(arena, t, view, info));
            return true;
        }
    }
    return false;
}

std::vector<NovaToken> tokenize(std::string_view src, NovaArena* arena, std::vector<LexError>& errors) {
    std::vector<NovaToken> tokens;
    tokens.reserve(src.size() / 2 + 1);

    NovaInfo info{};
    const char* current = src.data();
    const char* end = src.data() + src.size();

    while (current < end) {
        char c = *current;

        if (isSpace(c)) {
            if (c == '\n') NOVA_NEWLINE(&info);
            ++current; ++info.index;
            continue;
        }

        if (c == '/' && current + 1 < end) {
            if (*(current + 1) == '/') {
                skipSingleLine(info, current, end);
                continue;
            }
            if (*(current + 1) == '*') {
                skipMultiLine(info, current, end, errors, arena);
                continue;
            }
        }

        if (punctuation(current, end, tokens, info, arena))
            continue;

        if (isAlpha(c) || c == '_') {
            processIdentifier(current, end, tokens, info, arena);
            continue;
        }

        if (isDigit(c) || (c == '.' && current + 1 < end && isDigit(*(current + 1)))) {
            processNumber(current, end, tokens, info, errors, arena);
            continue;
        }

        if (c == '"') {
            processString(current, end, tokens, errors, info, arena);
            continue;
        }

        addError(errors, "Unknown character ", c, info.line, arena);
        tokens.push_back(make_token(arena, NOVA_TOKEN_UNKNOWN, std::string_view(current, 1), info));
        ++current; ++info.index;
    }

    tokens.push_back(make_token(arena, NOVA_TOKEN_END, std::string_view{}, info));
    return tokens;
}

} // namespace nova::detail

extern "C" NovaTokenSlice nova_tokenize(NovaArena* arena, const char* source, size_t source_len) {
    if (!arena || !source) {
        return {nullptr, 0};
    }

    std::string_view src(source, source_len);
    std::vector<nova::detail::LexError> errors;
    auto tokens = nova::detail::tokenize(src, arena, errors);

    if (!errors.empty()) {
        for (const auto&[msg, info] : errors) {
            std::cerr << "Lexical Error (line " << info.line << " & column " << info.index
                      << "): " << msg << '\n';
        }
        std::abort();
    }

    const size_t total_size = sizeof(NovaToken) * tokens.size();
    auto* out = static_cast<NovaToken*>(nova_arena_alloc(arena, total_size));
    if (!out) return {nullptr, 0};

    std::memcpy(out, tokens.data(), total_size);
    return NovaTokenSlice{.data = out, .len = tokens.size()};
}