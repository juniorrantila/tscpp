#pragma once
#include <Ty/Base.h>
#include <Ty/StringView.h>
#include <Ty/Verify.h>

struct Token {
    enum Kind : u8 {
        none,

        kw_function,
        kw_if,
        kw_throw,
        kw_return,
        kw_const,

        type_boolean,
        type_number,
        type_void,
        type__start = type_boolean,
        type__end = type_void,

        lit_ident,
        lit_string,
        lit_number,
        lit__start = lit_ident,
        lit__end = lit_number,

        sym_lparen,
        sym_rparen,

        sym_lcurly,
        sym_rcurly,

        sym_fat_arrow,
        sym_colon,
        sym_semicolon,
        sym_dot,
        sym_comma,

        op_lt_eq,
        op_minus,
        op_plus,
        op_triple_eq,
        op_assign,

        op_bang,
    };

    enum class Type {
        boolean,
        number,
        void_,
    };

    enum class Literal {
        ident,
        number,
        string,
    };

    constexpr Token() = default;

    constexpr Token(Kind kind, u32 position)
        : m_position(position)
        , m_kind(kind)
    {
    }

    constexpr bool operator==(Kind kind) const
    {
        return m_kind == kind;
    }

    constexpr operator Kind() const
    {
        return m_kind;
    }

    StringView kind_name() const;
    StringView view_in(StringView file) const;

    u32 position() const { return m_position; }

    bool is_literal() const;
    Literal as_literal() const;

    bool is_type() const;
    Type as_type() const;

private:

    u32 m_position : 24 { 0 };
    Kind m_kind { none };
};
