#include "./Parse.h"

#include "./Token.h"

#include <Ty/Verify.h>
#include <Ty/StringBuffer.h>

struct Parser {
    Parser(Source source, View<Token> tokens)
        : m_source(source)
        , m_tokens(tokens)
    {
    }

    Optional<Token> peek(usize ahead = 0) const
    {
        return m_tokens.peek(m_index + ahead);
    }

    Optional<Token> peek_or_last() const
    {
        return m_tokens.peek(m_index) ?: m_tokens.peek(m_tokens.size() - 1);
    }

    Optional<Token> next()
    {
        return m_tokens.peek(m_index++);
    }

    ErrorOr<Token, ParseError> expect_any(c_string func = __builtin_FUNCTION());
    ErrorOr<Token, ParseError> expect(Token::Kind kind, c_string func = __builtin_FUNCTION());

    template <usize Size>
    ErrorOr<Token, ParseError> expect_one_of(Token::Kind const(&kinds)[Size], c_string func = __builtin_FUNCTION());

    ErrorOr<Token, ParseError> peek_expect_any(c_string func = __builtin_FUNCTION());
    ErrorOr<Token, ParseError> peek_expect(Token::Kind kind, c_string func = __builtin_FUNCTION());

    template <usize Size>
    ErrorOr<Token, ParseError> peek_expect_one_of(Token::Kind const(&kinds)[Size], c_string func = __builtin_FUNCTION());

    usize index() const { return m_index; }

    Source source() const { return m_source; }
    View<Token> tokens() const { return m_tokens; }

private:
    Source m_source {};
    View<Token> m_tokens {};
    usize m_index { 0 };
};

ParseError::ParseError(c_string func, Parser const& parser, View<Token::Kind const> expected)
    : m_parser(new Parser(parser))
    , m_func(func)
    , m_kind(kind_expected_one_of)
{
    auto kinds = new Token::Kind[expected.size()];
    for (usize i = 0; i < expected.size(); i++) {
        kinds[i] = expected[i];
    }
    m_expected = View(kinds, expected.size());
}

ParseError::ParseError(c_string func, Parser const& parser, Token::Kind expected)
    : m_parser(new Parser(parser))
    , m_func(func)
    , m_kind(kind_expected)
{
    auto kind = new Token::Kind(expected);
    *kind = expected;
    m_expected = View(kind, 1);
}

ParseError::ParseError(c_string func, Parser const& parser)
    : m_parser(new Parser(parser))
    , m_func(func)
    , m_kind(kind_expected_any)
{
}

ParseError::operator::Error() const
{
    c_string message = nullptr;
    switch (m_kind) {
    case kind_error:
        return move(m_error);
    case kind_expected_any: {
        message = "expected something here";
    } break;
    case kind_expected: {
        auto buf = MUST(StringBuffer::create_saturated_fill("expected "sv));
        MUST(buf.write(Token(m_expected[0], 0).kind_name()));
        auto tok = m_parser->peek_or_last().or_else(Token(Token::none, 0));
        MUST(buf.write(" but got "sv, tok.kind_name()));
        message = buf.leak();
    } break;
    case kind_expected_one_of: {
        auto buf = MUST(StringBuffer::create_saturated_fill("expected one of ["sv));
        for (usize i = 0; auto expected : m_expected) {
            auto tok = Token(expected, 0);
            MUST(buf.write(tok.kind_name()));
            if (i + 1 != m_expected.size())
                MUST(buf.write(", "sv));
            i++;
        }
        auto tok = m_parser->peek_or_last().or_else(Token(Token::none, 0));
        MUST(buf.write("] but got "sv, tok.kind_name()));
        message = buf.leak();
    } break;
    }

    auto file = m_parser->source().file;
    auto path = m_parser->source().path;

    u32 row = 1;
    u32 col = 1;
    if (auto token = m_parser->peek_or_last()) {
        for (u32 i = 0; i < token->position(); i++) {
            if (file[i] == '\n') {
                row++;
                col = 1;
            } else {
                col++;
            }
        }
    }

    auto path_cstr = MUST(path.to_allocated_c_string());
    return Error::from_string_literal(message, m_func, path_cstr, row, col);
}

StringBuffer ParseError::message() const
{
    auto buf = StringBuffer();
    return buf;
}

template <usize Size>
ParseError ParseError::expected_one_of(Parser const& parser, Token::Kind const (&one_of)[Size], c_string func)
{
    auto expected = View(one_of, Size);
    return ParseError(func, parser, expected);
}

ParseError ParseError::expected(Parser const& parser, Token::Kind kind, c_string func)
{
    return ParseError(func, parser, kind);
}

ParseError ParseError::expected_any(Parser const& parser, c_string func)
{
    return ParseError(func, parser);
}

ErrorOr<Token, ParseError> Parser::expect_any(c_string func)
{
    auto token = next();
    if (!token)
        return ParseError::expected_any(*this, func);
    return *token;
}

ErrorOr<Token, ParseError> Parser::expect(Token::Kind kind, c_string func)
{
    auto token = TRY(expect_any());
    if (token != kind)
        return ParseError::expected(*this, kind, func);
    return token;
}

ErrorOr<Token, ParseError> Parser::peek_expect_any(c_string func)
{
    auto token = peek();
    if (!token)
        return ParseError::expected_any(*this, func);
    return *token;
}

ErrorOr<Token, ParseError> Parser::peek_expect(Token::Kind kind, c_string func)
{
    auto token = TRY(peek_expect_any());
    if (token != kind)
        return ParseError::expected(*this, kind, func);
    return token;
}

template <usize Size>
ErrorOr<Token, ParseError> Parser::expect_one_of(Token::Kind const(&kinds)[Size], c_string func)
{
    auto token = TRY(expect_any());
    for (auto kind : kinds) {
        if (token == kind)
            return token;
    }
    return ParseError::expected_one_of(*this, kinds, func);
}

template <usize Size>
ErrorOr<Token, ParseError> Parser::peek_expect_one_of(Token::Kind const(&kinds)[Size], c_string func)
{
    auto token = TRY(peek_expect_any());
    for (auto kind : kinds) {
        if (token == kind)
            return token;
    }
    return ParseError::expected_one_of(*this, kinds, func);
}




ErrorOr<Vector<VarDecl>, ParseError> parse_parameters(Parser& parser);
ErrorOr<FuncDecl, ParseError> parse_function(Parser& parser);
ErrorOr<FuncCall, ParseError> parse_func_call(Parser& parser);
ErrorOr<Vector<VarDecl>, ParseError> parse_func_call_args(Parser& parser);
ErrorOr<Type, ParseError> parse_type(Parser& parser);
ErrorOr<Block, ParseError> parse_block(Parser& parser);
ErrorOr<Expr, ParseError> parse_expression(Parser& parser);
ErrorOr<IfStmt, ParseError> parse_if(Parser& parser);
ErrorOr<RValue, ParseError> parse_rvalue(Parser& parser);
ErrorOr<ThrowStmt, ParseError> parse_throw(Parser& parser);
ErrorOr<ReturnStmt, ParseError> parse_return(Parser& parser);

ErrorOr<UnaryExpr, ParseError> parse_unary(Parser& parser);
ErrorOr<BinaryExpr, ParseError> parse_binary(Parser& parser);
ErrorOr<DotExpr, ParseError> parse_dot_expr(Parser& parser);

ErrorOr<ParseTree, ParseError> parse(Source source, View<Token> tokens)
{
    auto tree = ParseTree();
    auto parser = Parser(source, tokens);

    while(parser.peek().has_value()) {
        TRY(tree.expressions.append(TRY(parse_expression(parser))));
        if (parser.peek() == Token::sym_semicolon) {
            TRY(parser.expect(Token::sym_semicolon));
        }
    }

    return tree;
}

ErrorOr<FuncDecl, ParseError> parse_function(Parser& parser)
{
    auto func = FuncDecl();
    TRY(parser.expect(Token::kw_function));
    auto name = TRY(parser.expect(Token::lit_ident));
    TRY(parser.expect(Token::sym_lparen));
    auto parameters = TRY(parse_parameters(parser));
    TRY(parser.expect(Token::sym_colon));
    auto type = TRY(parse_type(parser));
    auto block = TRY(parse_block(parser));
    return FuncDecl {
        .name = name,
        .args = move(parameters),
        .return_type = type,
        .block = move(block),
    };
}

ErrorOr<FuncCall, ParseError> parse_func_call(Parser& parser)
{
    auto ident = TRY(parser.expect(Token::lit_ident));
    auto args = TRY(parse_func_call_args(parser));
    return FuncCall {
        .name = ident,
        .args = move(args),
    };
}

ErrorOr<Vector<VarDecl>, ParseError> parse_func_call_args(Parser& parser)
{
    auto args = Vector<VarDecl>();

    TRY(parser.expect(Token::sym_lparen));
    while (parser.peek() != Token::sym_rparen) {
        auto value = TRY(parse_rvalue(parser));
        TRY(args.append(VarDecl {
            .default_value = value,
        }));
        if (parser.peek() == Token::sym_comma) {
            TRY(parser.expect(Token::sym_comma));
        }
    }
    TRY(parser.expect(Token::sym_rparen));
    return args;
}

ErrorOr<Vector<VarDecl>, ParseError> parse_parameters(Parser& parser)
{
    auto parameters = Vector<VarDecl>();

    while (parser.peek() != Token::sym_rparen) {
        auto name = TRY(parser.expect(Token::lit_ident));
        TRY(parser.expect(Token::sym_colon));
        auto type = TRY(parse_type(parser));

        TRY(parameters.append(VarDecl {
            .name = name,
            .type = type,
            .default_value = {},
        }));
    }
    TRY(parser.expect(Token::sym_rparen));
    return parameters;
}

ErrorOr<Type, ParseError> parse_type(Parser& parser)
{
    return Type::from_token(TRY(parser.expect_one_of({
        Token::type_boolean,
        Token::type_number,
        Token::type_void,
    })));
}

ErrorOr<Block, ParseError> parse_block(Parser& parser)
{
    auto block = Block();
    TRY(parser.expect(Token::sym_lcurly));
    while (parser.peek() != Token::sym_rcurly) {
        TRY(block.exprs.append(TRY(parse_expression(parser))));
        if (parser.peek() == Token::sym_semicolon) {
            TRY(parser.expect(Token::sym_semicolon));
        }
    }
    TRY(parser.expect(Token::sym_rcurly));
    return block;
}

ErrorOr<Expr, ParseError> parse_expression(Parser& parser)
{
    auto token = TRY(parser.peek_expect_one_of({
        Token::kw_function,
        Token::kw_if,
        Token::kw_throw,
        Token::kw_return,
        Token::sym_lcurly,
        Token::lit_ident,
    }));

    if (token == Token::kw_function) {
        return Expr(TRY(parse_function(parser)));
    }
    if (token == Token::kw_if) {
        return Expr(TRY(parse_if(parser)));
    }
    if (token == Token::kw_throw) {
        return Expr(TRY(parse_throw(parser)));
    }
    if (token == Token::kw_return) {
        return Expr(TRY(parse_return(parser)));
    }
    if (token == Token::sym_lcurly) {
        return Expr(TRY(parse_block(parser)));
    }
    if (token == Token::lit_ident) {
        return Expr(TRY(parse_rvalue(parser)));
    }

    return Error::unreachable();
}

ErrorOr<IfStmt, ParseError> parse_if(Parser& parser)
{
    TRY(parser.expect(Token::kw_if));
    TRY(parser.expect(Token::sym_lparen));
    auto cond = TRY(parse_rvalue(parser));
    TRY(parser.expect(Token::sym_rparen));
    auto then = TRY(parse_expression(parser));

    return IfStmt {
        .cond = cond,
        .then = move(then),
        .else_ = Expr(), // FIXME: Else block
    };
}

ErrorOr<RValue, ParseError> parse_rvalue(Parser& parser)
{
    if (parser.peek() == Token::op_bang) {
        return RValue {
            .value = Expr(TRY(parse_unary(parser))),
        };
    }

    if (parser.peek() == Token::lit_string) {
        auto value = parser.next();
        return RValue {
            .value = Expr::string(*value)
        };
    }

    if (parser.peek() == Token::lit_number) {
        auto value = parser.next();
        return RValue {
            .value = Expr::number(*value)
        };
    }
    
    if (parser.peek() == Token::lit_ident) {
        if (parser.peek(1) == Token::sym_dot) {
            return RValue {
                .value = TRY(parse_dot_expr(parser)),
            };
        }
        if (parser.peek(1) == Token::sym_lparen) {
            return RValue {
                .value = TRY(parse_func_call(parser)),
            };
        }
        return RValue {
            .value = Expr(TRY(parse_binary(parser))),
        };
    }

    return ParseError::expected_one_of(parser, {
        Token::op_bang,
        Token::lit_string,
        Token::lit_ident,
    });
}

ErrorOr<UnaryExpr, ParseError> parse_unary(Parser& parser)
{
    auto op = TRY(parser.expect(Token::op_bang));
    auto identifier = TRY(parser.expect(Token::lit_ident));
    return UnaryExpr {
        .op = op,
        .value = RValue {
            .value = Expr::lvalue(identifier),
        }
    };
}

ErrorOr<BinaryExpr, ParseError> parse_binary(Parser& parser)
{
    // FIXME: Don't hardcode this
    auto lhs = TRY(parser.expect_one_of({
        Token::lit_ident,
        Token::lit_number
    }));
    auto op = TRY(parser.expect_one_of({
        Token::op_lt_eq,
        Token::op_minus,
        Token::op_plus,
        Token::op_triple_eq,
        Token::op_assign,
    }));
    auto rhs = TRY(parser.expect_one_of({
        Token::lit_ident,
        Token::lit_number
    }));

    auto lhs_rvalue = RValue {
        .value = Expr::lvalue(lhs),
    };
    if (lhs == Token::lit_number) {
        lhs_rvalue = RValue {
            .type = Type::number,
            .value = Expr::number(lhs),
        };
    }

    auto rhs_rvalue = RValue {
        .value = Expr::lvalue(lhs),
    };
    if (rhs == Token::lit_number) {
        rhs_rvalue = RValue {
            .type = Type::number,
            .value = Expr::number(rhs),
        };
    }

    return BinaryExpr {
        .op = op,
        .lhs = lhs_rvalue,
        .rhs = rhs_rvalue,
    };
}

ErrorOr<DotExpr, ParseError> parse_dot_expr(Parser& parser)
{
    auto lhs = TRY(parser.expect(Token::lit_ident));
    TRY(parser.expect(Token::sym_dot));
    auto rhs = TRY(parse_rvalue(parser));
    return DotExpr {
        .rhs = rhs,
        .lhs = lhs,
    };
}

ErrorOr<ThrowStmt, ParseError> parse_throw(Parser& parser)
{
    TRY(parser.expect(Token::kw_throw));
    auto value = TRY(parse_rvalue(parser));
    return ThrowStmt {
        .value = value,
    };
}

ErrorOr<ReturnStmt, ParseError> parse_return(Parser& parser)
{
    TRY(parser.expect(Token::kw_return));
    auto value = TRY(parse_rvalue(parser));
    return ReturnStmt {
        .value = value,
    };
}

Expr Expr::lvalue(Token token)
{
    return Expr(lvalue_expr, token);
}

Expr Expr::string(Token token)
{
    return Expr(string_literal, token);
}

Expr Expr::number(Token token)
{
    return Expr(number_literal, token);
}

Expr::Expr(Block&& value)
    : kind(block)
{
    as.block = new Block(move(value));
}

Expr::Expr(FuncDecl&& value)
    : kind(func_decl)
{
    as.func_decl = new FuncDecl(move(value));
}

Expr::Expr(FuncCall&& value)
    : kind(func_call)
{
    as.func_call = new FuncCall(move(value));
}

Expr::Expr(VarDecl&& value)
    : kind(var_decl)
{
    as.var_decl = new VarDecl(move(value));
}

Expr::Expr(IfStmt&& value)
    : kind(if_stmt)
{
    as.if_stmt = new IfStmt(move(value));
}

Expr::Expr(ThrowStmt&& value)
    : kind(throw_stmt)
{
    as.throw_stmt = new ThrowStmt(move(value));
}

Expr::Expr(ReturnStmt&& value)
    : kind(return_stmt)
{
    as.return_stmt = new ReturnStmt(move(value));
}

Expr::Expr(UnaryExpr&& value)
    : kind(unary_expr)
{
    as.unary_expr = new UnaryExpr(move(value));
}

Expr::Expr(BinaryExpr&& value)
    : kind(binary_expr)
{
    as.binary_expr = new BinaryExpr(move(value));
}

Expr::Expr(RValue&& value)
    : kind(rvalue_expr)
{
    as.rvalue_expr = new RValue(move(value));
}

Expr::Expr(DotExpr&& value)
    : kind(dot_expr)
{
    as.dot_expr = new DotExpr(move(value));
}

Expr::Expr(Kind kind, Token token)
    : kind(kind)
{
    VERIFY(kind == lvalue_expr || kind == string_literal || kind == number_literal);
    if (kind == lvalue_expr) {
        as.lvalue_expr = token;
    }
    if (kind == string_literal) {
        as.string_literal = token;
    }
    if (kind == number_literal) {
        as.number_literal = token;
    }
}


Type Type::from_token(Token token)
{
    switch (token.as_type()) {
    case Token::Type::boolean:
        return Type::boolean; 
    case Token::Type::number:
        return Type::number;
    case Token::Type::void_:
        return Type::void_;
    }
}

ErrorOr<StringBuffer> Type::to_string() const
{
    switch (kind()) {
    case Type::boolean:
        return StringBuffer::create_fill("boolean"sv);
    case Type::number:
        return StringBuffer::create_fill("number"sv);
    case Type::void_:
        return StringBuffer::create_fill("void"sv);
    case Type::none:
        return StringBuffer::create_fill("none"sv);
    }
}
