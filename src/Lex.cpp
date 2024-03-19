#include "./Lex.h"

#include <Ty/Verify.h>
#include <Core/File.h>

struct FixedToken {
    Token::Kind kind;
    StringView name;
};

const FixedToken keywords_and_types[] = {
    { Token::kw_function,   "function"sv },
    { Token::kw_if,         "if"sv },
    { Token::kw_throw,      "throw"sv },
    { Token::kw_return,     "return"sv },
    { Token::kw_const,      "const"sv },

    { Token::type_boolean,  "boolean"sv },
    { Token::type_number,   "number"sv },
    { Token::type_void,     "void"sv },
};

const FixedToken symbols_and_ops[] = {
    { Token::sym_lparen,    "("sv },
    { Token::sym_rparen,    ")"sv },

    { Token::sym_lcurly,    "{"sv },
    { Token::sym_rcurly,    "}"sv },

    { Token::sym_fat_arrow, "=>"sv },
    { Token::sym_colon,     ":"sv },
    { Token::sym_semicolon, ";"sv },
    { Token::sym_dot,       "."sv },
    { Token::sym_comma,     ","sv },

    { Token::op_lt_eq,      "<="sv },
    { Token::op_minus,      "-"sv },
    { Token::op_plus,       "+"sv },
    { Token::op_assign,     "="sv },
    { Token::op_bang,       "!"sv },
    { Token::op_triple_eq,  "==="sv },
};

static bool is_keyword_delim(StringView c)
{
    if (c.is_empty())
        return false;
    switch (c[0]) {
    case 'a'...'z':
    case 'A'...'Z':
    case '0'...'9':
    case '_': case '$':
        return false;
    }
    return true;
}

ErrorOr<Vector<Token>, LexError> lex(Source source)
{
    auto tokens = Vector<Token>();

    auto file = source.file;
    u32 pos = 0;
    while(pos < file.size()) {
        switch (file[pos]) {
        case '\0': case ' ':
        case '\n': case '\r':
        case '\t': 
            pos++;
            goto next_token;
        }
        for (auto keyword_or_type : keywords_and_types) {
            auto part = file.sub_view(pos, keyword_or_type.name.size());
            if (part == keyword_or_type.name) {
                if (is_keyword_delim(file.sub_view(pos + part.size(), 1))) {
                    TRY(tokens.append(Token(keyword_or_type.kind, pos)));
                    pos += part.size();
                    goto next_token;
                }
            }
        }
        for (auto symbol_or_op : symbols_and_ops) {
            auto part = file.sub_view(pos, symbol_or_op.name.size());
            if (part == symbol_or_op.name) {
                TRY(tokens.append(Token(symbol_or_op.kind, pos)));
                pos += part.size();
                goto next_token;
            }
        }

        if (file[pos] == '"' || file[pos] == '\'' || file[pos] == '`') {
            auto token = Token(Token::lit_string, pos);
            TRY(tokens.append(token));
            pos += relex_string_size(file, pos) + 1;
            goto next_token;
        }

        switch (file[pos]) {
        case 'a'...'z':
        case 'A'...'Z':
        case '_': case '$': {
            auto token = Token(Token::lit_ident, pos);
            TRY(tokens.append(token));
            pos += relex_ident_size(file, pos);
            goto next_token;
        } break;

        case '0'...'9': {
            auto token = Token(Token::lit_number, pos);
            TRY(tokens.append(token));
            pos += relex_number_size(file, pos);
            goto next_token;
        } break;
        }

        if (file[pos] == '/') {
            if (pos + 1 < file.size()) {
                if (file[pos + 1] == '*') {
                    pos += 2;
                    for (;pos + 1 < file.size(); pos++) {
                        file.part(pos, pos + 1);
                        if (file.sub_view(pos, "*/"sv.size()) == "*/") {
                            pos += "*/"sv.size();
                            goto next_token;
                        }
                    }
                    return LexError::from_string_literal(source, pos, "expected end of block comment");
                }
                if (file[pos + 1] == '/') {
                    pos += 2;
                    while (pos < file.size() && file[pos] != '\n') {
                        pos++;
                    }
                    goto next_token;
                }
            }
        }

        return LexError::from_string_literal(source, pos, "unknown character");
next_token:;
    }

    return tokens;
}

u32 relex_ident_size(StringView file, u32 pos);
u32 relex_number_size(StringView file, u32 pos);
u32 relex_string_size(StringView file, u32 pos);

u32 relex_ident_size(StringView file, u32 pos)
{
    u32 size = 0;
    for (;pos < file.size(); pos++, size++) {
        switch (file[pos]) {
            case 'a'...'z':
            case 'A'...'Z':
            case '0'...'9':
            case '_': case '$':
                continue;
        }
        break;
    }
    return size;
}

u32 relex_number_size(StringView file, u32 pos)
{
    u32 size = 0;
    for (;pos < file.size(); pos++, size++) {
        switch (file[pos]) {
            case '0'...'9':
            case '.':
                continue;
        }
        break;
    }
    return size;
}

u32 relex_string_size(StringView file, u32 pos)
{
    auto string_kind = file[pos++];

    u32 size = 1;
    for (;pos < file.size(); pos++, size++) {
        if (file[pos] == string_kind) {
            break;
        }
    }
    return size;
}

LexError LexError::from_string_literal(Source source, u32 pos, c_string message, c_string func)
{
    return LexError(source, pos, message, func);
}

constexpr LexError::LexError(Source source, u32 pos, c_string message, c_string func)
    : m_source(source)
    , m_func(func)
    , m_message(message)
    , m_pos(pos)
{
}

LexError::operator Error() const
{
    if (m_message == nullptr) {
        return move(m_error);
    }
    auto file = m_source.file;
    auto path = m_source.path;

    u32 row = 1;
    u32 col = 1;
    for (u32 i = 0; i < m_pos; i++) {
        if (file[i] == '\n') {
            row++;
            col = 1;
        } else {
            col++;
        }
    }

    auto path_cstr = MUST(path.to_allocated_c_string());
    return Error::from_string_literal(m_message, m_func, path_cstr, row, col);
}
