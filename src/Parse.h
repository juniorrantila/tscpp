#pragma once
#include "./Source.h"
#include "./Token.h"

#include <Ty/View.h>
#include <Ty/ErrorOr.h>
#include <Ty/Vector.h>

struct Parser;
struct ParseError {
    constexpr ParseError(Error error)
        : m_error(error)
    {
    }

    ErrorOr<void> show(Source, View<Token> tokens) const;
    StringBuffer message() const;

    template <usize Size>
    static ParseError expected_one_of(Parser const&, Token::Kind const(&one_of)[Size], c_string func = __builtin_FUNCTION());

    static ParseError expected(Parser const&, Token::Kind, c_string func = __builtin_FUNCTION());
    static ParseError expected_any(Parser const&, c_string func = __builtin_FUNCTION());

    operator Error() const;

private:
    enum Kind {
        kind_error,
        kind_expected,
        kind_expected_one_of,
        kind_expected_any,
    };

    ParseError(c_string func, Parser const& parser, View<Token::Kind const> expexted);
    ParseError(c_string func, Parser const& parser, Token::Kind expected);
    ParseError(c_string func, Parser const& parser);

    Error m_error {};
    View<Token::Kind const> m_expected {}; 
    Parser const* m_parser { nullptr };
    c_string m_func { nullptr };
    Kind m_kind { kind_error };
};

struct VarDecl;
struct Block;
struct FuncDecl;
struct FuncCall;

struct IfStmt;
struct ThrowStmt;
struct ReturnStmt;

struct UnaryExpr;
struct BinaryExpr;
struct RValue;
struct DotExpr;

struct Expr {
    enum Kind {
        none,

        block,

        var_decl,
        func_decl,
        func_call,

        if_stmt,
        throw_stmt,
        return_stmt,

        unary_expr,
        binary_expr,
        dot_expr,

        lvalue_expr,
        rvalue_expr,

        string_literal,
        number_literal,
    };

    explicit Expr() = default;

    static Expr lvalue(Token);
    static Expr string(Token);
    static Expr number(Token);

    Expr(Block&& value);
    Expr(FuncDecl&& value);
    Expr(FuncCall&& value);
    Expr(VarDecl&& value);
    Expr(IfStmt&& value);
    Expr(ThrowStmt&& value);
    Expr(ReturnStmt&& value);
    Expr(UnaryExpr&& value);
    Expr(BinaryExpr&& value);
    Expr(RValue&& value);
    Expr(DotExpr&& value);

    constexpr operator Kind() const { return kind; }

    union {
        Block* block;
        FuncDecl* func_decl;
        FuncCall* func_call;
        VarDecl* var_decl;
        IfStmt* if_stmt;
        ThrowStmt* throw_stmt;
        ReturnStmt* return_stmt;
        UnaryExpr* unary_expr;
        BinaryExpr* binary_expr;
        RValue* rvalue_expr;
        DotExpr* dot_expr;
        Token lvalue_expr;
        Token string_literal;
        Token number_literal;
    } as { nullptr };
    Kind kind { none };

private:
    Expr(Kind kind, Token token);
};

struct Type {
    enum Kind {
        none,

        boolean,
        number,
        void_,
    };

    static Type from_token(Token token);

    Type() = default;

    Type(Kind kind)
        : m_kind(kind)
    {
    }

    Kind kind() const { return m_kind; }
    operator Kind() const { return kind(); }

    ErrorOr<StringBuffer> to_string() const;

private:
    Kind m_kind { none };
};

struct RValue {
    Type type {};
    Expr value {};
};

struct VarDecl {
    Token name {};
    Type type {};
    Optional<RValue> default_value {};
};

struct Block {
    Vector<Expr> exprs {};
};

struct FuncDecl {
    Token name {};
    Vector<VarDecl> args {};
    Type return_type {};
    Block block {};
};

struct FuncCall {
    Token name;
    Vector<VarDecl> args;
    Type return_type;
};

struct IfStmt {
    RValue cond {};
    Expr then {};
    Expr else_ {};
};

struct UnaryExpr {
    Token op {};
    RValue value {};
};

struct BinaryExpr {
    Token op {};
    RValue lhs {};
    RValue rhs {};
};

struct DotExpr {
    RValue rhs {};
    Token lhs {};
};

struct ThrowStmt {
    RValue value {};
};

struct ReturnStmt {
    RValue value {};
};

struct ParseTree {
    Vector<Expr> expressions {};
};

ErrorOr<ParseTree, ParseError> parse(Source source, View<Token> tokens);
