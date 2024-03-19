#pragma once
#include "./Source.h"
#include "./Token.h"

#include <Ty/ErrorOr.h>
#include <Ty/StringView.h>
#include <Ty/Vector.h>

struct LexError {
    constexpr LexError(Error error)
        : m_error(error)
    {
    }

    static LexError from_string_literal(Source source, u32 pos, c_string message, c_string func = __builtin_FUNCTION());

    operator Error() const;

private:
    constexpr LexError(Source source, u32 pos, c_string message, c_string func);

    Error m_error {};
    Source m_source {};
    c_string m_func { nullptr };
    c_string m_message { 0 };
    u32 m_pos { 0 };
};

ErrorOr<Vector<Token>, LexError> lex(Source source);

u32 relex_ident_size(StringView file, u32 pos);
u32 relex_number_size(StringView file, u32 pos);
u32 relex_string_size(StringView file, u32 pos);
