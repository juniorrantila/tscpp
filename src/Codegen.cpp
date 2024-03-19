#include "./Parse.h"

#include <Ty/StringBuffer.h>

struct Codegen {
    Source source;
    ParseTree const& parse_tree;
};

static ErrorOr<u32> codegen_prelude(StringBuffer&, Codegen const&);
static ErrorOr<u32> codegen_types(StringBuffer&, Codegen const&);
static ErrorOr<u32> codegen_function_forwards(StringBuffer&, Codegen const&);
static ErrorOr<u32> codegen_main(StringBuffer&, Codegen const&);
static ErrorOr<u32> codegen_expr(StringBuffer&, Codegen const&, Expr const&);

static ErrorOr<u32> codegen_block(StringBuffer&, Codegen const&, Block const&);
static ErrorOr<u32> codegen_var_decl(StringBuffer&, Codegen const&, VarDecl const&);
static ErrorOr<u32> codegen_func_decl(StringBuffer&, Codegen const&, FuncDecl const&);
static ErrorOr<u32> codegen_func_call(StringBuffer&, Codegen const&, FuncCall const&);

static ErrorOr<u32> codegen_if_stmt(StringBuffer&, Codegen const&, IfStmt const&);
static ErrorOr<u32> codegen_throw_stmt(StringBuffer&, Codegen const&, ThrowStmt const&);
static ErrorOr<u32> codegen_return_stmt(StringBuffer&, Codegen const&, ReturnStmt const&);

static ErrorOr<u32> codegen_unary_expr(StringBuffer&, Codegen const&, UnaryExpr const&);
static ErrorOr<u32> codegen_binary_expr(StringBuffer&, Codegen const&, BinaryExpr const&);
static ErrorOr<u32> codegen_dot_expr(StringBuffer&, Codegen const&, DotExpr const&);

static ErrorOr<u32> codegen_lvalue_expr(StringBuffer&, Codegen const&, Token);
static ErrorOr<u32> codegen_rvalue_expr(StringBuffer&, Codegen const&, RValue const&);
static ErrorOr<u32> codegen_string_literal(StringBuffer&, Codegen const&, Token);
static ErrorOr<u32> codegen_number_literal(StringBuffer&, Codegen const&, Token);

namespace {
extern StringView const prelude;
}

ErrorOr<StringBuffer> codegen(Source source, ParseTree const& tree)
{
    auto out = TRY(StringBuffer::create());
    auto codegen = Codegen(source, tree);
    TRY(codegen_prelude(out, codegen));
    TRY(codegen_types(out, codegen));
    TRY(codegen_function_forwards(out, codegen));
    TRY(codegen_main(out, codegen));
    return out;
}

static ErrorOr<u32> codegen_prelude(StringBuffer& out, Codegen const&)
{
    return TRY(out.write(prelude));
}

static ErrorOr<u32> codegen_types(StringBuffer& out, Codegen const&)
{
    return TRY(out.writeln("\n// FIXME: implement codegen_types"sv));
}

static ErrorOr<u32> codegen_function_forwards(StringBuffer& out, Codegen const& gen)
{
    u32 size = 0;
    for (auto const& expr : gen.parse_tree.expressions) {
        if (expr.kind == Expr::func_decl) {
            auto func = expr.as.func_decl;
            auto return_type = TRY(func->return_type.to_string());
            auto name = func->name.view_in(gen.source.file);
            size += TRY(out.write("static ErrorOr<"sv, return_type.view(), ">"sv));
            size += TRY(out.write("(*"sv, name, ")("sv));
            for (usize i = 0; auto const& arg : func->args.view()) {
                auto arg_type = TRY(arg.type.to_string());
                size += TRY(out.write(arg_type.view()));
                if (i + 1 < func->args.size()) {
                    size += TRY(out.write(", "sv));
                }
                i++;
            }
            size += TRY(out.writeln(") = nullptr;"sv));
        }
    }
    return size;
}

static ErrorOr<u32> codegen_main(StringBuffer& out, Codegen const& gen)
{
    u32 size = 0;

    size += TRY(out.writeln("ErrorOr<int> Main::main(int, c_string[])"sv));
    size += TRY(out.writeln("{"sv));
    size += TRY(out.writeln("TRY([]() -> ErrorOr<void> {"sv));

    for (auto const& expr : gen.parse_tree.expressions) {
        size += TRY(codegen_expr(out, gen, expr));
        size += TRY(out.writeln(";"sv));
    }

    size += TRY(out.writeln("return {};"sv));
    size += TRY(out.writeln("}());"sv));
    size += TRY(out.writeln("return 0;"sv));
    size += TRY(out.writeln("}"sv));

    return size;
}

static ErrorOr<u32> codegen_expr(StringBuffer& out, Codegen const& codegen, Expr const& expr)
{
    switch(expr) {
    case Expr::none:
        return Error::from_string_literal("trying to codegen none");
    case Expr::block:
        return TRY(codegen_block(out, codegen, *expr.as.block));
    case Expr::var_decl:
        return TRY(codegen_var_decl(out, codegen, *expr.as.var_decl));
    case Expr::func_decl:
        return TRY(codegen_func_decl(out, codegen, *expr.as.func_decl));
    case Expr::func_call:
        return TRY(codegen_func_call(out, codegen, *expr.as.func_call));

    case Expr::if_stmt:
        return TRY(codegen_if_stmt(out, codegen, *expr.as.if_stmt));
    case Expr::throw_stmt:
        return TRY(codegen_throw_stmt(out, codegen, *expr.as.throw_stmt));
    case Expr::return_stmt:
        return TRY(codegen_return_stmt(out, codegen, *expr.as.return_stmt));

    case Expr::unary_expr:
        return TRY(codegen_unary_expr(out, codegen, *expr.as.unary_expr));
    case Expr::binary_expr:
        return TRY(codegen_binary_expr(out, codegen, *expr.as.binary_expr));
    case Expr::dot_expr:
        return TRY(codegen_dot_expr(out, codegen, *expr.as.dot_expr));

    case Expr::lvalue_expr:
        return TRY(codegen_lvalue_expr(out, codegen, expr.as.lvalue_expr));
    case Expr::rvalue_expr:
        return TRY(codegen_rvalue_expr(out, codegen, *expr.as.rvalue_expr));

    case Expr::string_literal:
        return TRY(codegen_string_literal(out, codegen, expr.as.string_literal));
    case Expr::number_literal:
        return TRY(codegen_number_literal(out, codegen, expr.as.number_literal));
    }
}

static ErrorOr<u32> codegen_block(StringBuffer& out, Codegen const& codegen, Block const& block)
{
    u32 size = 0;

    size += TRY(out.writeln("{"sv));
    for (auto const& expr : block.exprs.view()) {
        size += TRY(codegen_expr(out, codegen, expr));
    }
    size += TRY(out.writeln("}"sv));

    return size;
}

static ErrorOr<u32> codegen_var_decl(StringBuffer&, Codegen const&, VarDecl const&)
{
    return Error::unimplemented();
}

static ErrorOr<u32> codegen_func_decl(StringBuffer& out, Codegen const& gen, FuncDecl const& func)
{
    u32 size = 0;
    auto name = func.name.view_in(gen.source.file);
    size += TRY(out.write(name, " = []("sv));
    for (usize i = 0; auto const& arg : func.args.view()) {
        auto arg_type = TRY(arg.type.to_string());
        auto arg_name = arg.name.view_in(gen.source.file);
        size += TRY(out.write(arg_type.view(), " "sv, arg_name));
        if (i + 1 < func.args.size()) {
            size += TRY(out.write(", "sv));
        }
        i++;
    }
    size += TRY(out.write(") -> "sv));
    auto return_type = TRY(func.return_type.to_string());
    size += TRY(out.write("ErrorOr<"sv, return_type.view(), "> "sv));
    size += TRY(out.writeln("{"sv));
    size += TRY(codegen_block(out, gen, func.block));
    if (func.return_type.kind() == Type::void_) {
        size += TRY(out.write("return {};"sv));
    }
    size += TRY(out.writeln("};"sv));

    return size;
}

static ErrorOr<u32> codegen_func_call(StringBuffer& out, Codegen const& gen, FuncCall const& func_call)
{
    u32 size = 0;

    auto name = func_call.name.view_in(gen.source.file);
    size += TRY(out.write(name, "("sv));
    for (u32 i = 0; auto const& arg : func_call.args.view()) {
        if (!arg.default_value) {
            return Error::from_string_literal("expected some value for function parameter");
        }
        size += TRY(codegen_rvalue_expr(out, gen, *arg.default_value));
        if (i + 1 < func_call.args.size()) {
            size += TRY(out.write(", "sv));
        }
    }
    size += TRY(out.write(")"sv));

    return size;
}

static ErrorOr<u32> codegen_if_stmt(StringBuffer& out, Codegen const& gen, IfStmt const& stmt)
{
    u32 size = 0;

    size += TRY(out.write("if ("sv));
    size += TRY(codegen_expr(out, gen, stmt.cond.value));
    size += TRY(out.writeln(")"sv));
    size += TRY(codegen_expr(out, gen, stmt.then));

    return size;
}

static ErrorOr<u32> codegen_throw_stmt(StringBuffer& out, Codegen const& gen, ThrowStmt const& stmt)
{
    u32 size = 0;

    auto const& expr = stmt.value.value;
    size += TRY(out.write("return Error::from_string_literal("sv));
    size += TRY(codegen_expr(out, gen, expr));
    size += TRY(out.write(".data()"sv));
    size += TRY(out.writeln(");"sv));

    return size;
}

static ErrorOr<u32> codegen_return_stmt(StringBuffer&, Codegen const&, ReturnStmt const&)
{
    return Error::unimplemented();
}

static ErrorOr<u32> codegen_unary_expr(StringBuffer& out, Codegen const& gen, UnaryExpr const& expr)
{
    u32 size = 0;
    auto op = expr.op.view_in(gen.source.file);
    size += TRY(out.write(op));
    size += TRY(codegen_expr(out, gen, expr.value.value));
    return size;
}

static ErrorOr<u32> codegen_binary_expr(StringBuffer&, Codegen const&, BinaryExpr const&)
{
    return Error::unimplemented();
}

static ErrorOr<u32> codegen_dot_expr(StringBuffer& out, Codegen const& gen, DotExpr const& expr)
{
    u32 size = 0;

    size += TRY(out.write("("sv));
    size += TRY(out.write(expr.lhs.view_in(gen.source.file)));
    size += TRY(out.write("->"sv));
    size += TRY(codegen_rvalue_expr(out, gen, expr.rhs));
    size += TRY(out.write(")"sv));

    return size;
}

static ErrorOr<u32> codegen_lvalue_expr(StringBuffer& out, Codegen const& gen, Token token)
{
    return TRY(out.write(token.view_in(gen.source.file)));
}

static ErrorOr<u32> codegen_rvalue_expr(StringBuffer& out, Codegen const& gen, RValue const& rvalue)
{
    u32 size = 0;
    size += TRY(codegen_expr(out, gen, rvalue.value));
    return size;
}

static ErrorOr<u32> codegen_string_literal(StringBuffer& out, Codegen const& gen, Token token)
{
    // FIXME: Off by one
    return TRY(out.write(token.view_in(gen.source.file), "\"sv"sv));
}

static ErrorOr<u32> codegen_number_literal(StringBuffer&, Codegen const&, Token)
{
    return Error::unimplemented();
}

namespace {
StringView const prelude = R"(
#include <Main/Main.h>
#include <Core/File.h>

namespace JS {

struct Console {
    template <typename... Args>
    void log(Args... args) const
    {
        auto buf = StringBuffer();
        MUST(buffer(buf, args...));
    }

    Console const* operator->() const { return this; }

private:
    template <typename... Args>
        requires (sizeof...(Args) > 1)
    ErrorOr<usize> buffer(StringBuffer& buf, Args const&... args) const
    {
        ErrorOr<usize> results[] = {
            buffer(buf, args)...
        };
        usize size = 0;
        for (auto& result : results)
            size += TRY(result);
        return size;
    }

    template <typename T>
    ErrorOr<usize> buffer(StringBuffer& buffer, T const& value) const
    {
        if constexpr (requires { value.toString(); }) {
            auto buf = TRY(value.toString());
            return TRY(buffer.write(buf.view()));
        } else {
            return TRY(buffer.write(value));
        }
    }
} console;

struct number {
    number(double value)
        : m_value(value)
    {
    }

    number operator+(number other) const
    {
        return number(m_value + other.m_value);
    }

    number operator-(number other) const
    {
        return number(m_value + other.m_value);
    }

    bool operator<=(number other) const
    {
        return m_value <= other.m_value;
    }

    bool operator==(number other) const
    {
        return m_value == other.m_value;
    }

    ErrorOr<StringBuffer> toString() const
    {
        return StringBuffer::create_fill(m_value);
    }

private:
    double m_value { 0.0 };
};

using boolean = bool;

}

using JS::console;
using JS::number;
using JS::boolean;
)"sv;
}
