#include "./Token.h"
#include "./Lex.h"

bool Token::is_literal() const
{
    if (m_kind >= lit__start)
        return true;
    if (m_kind <= lit__end)
        return true;
    return false;
}

Token::Literal Token::as_literal() const
{
    VERIFY(is_literal());
    return Literal(m_kind - lit__start);
}

bool Token::is_type() const
{
    if (m_kind >= type__start)
        return true;
    if (m_kind <= type__end)
        return true;
    return false;
}

Token::Type Token::as_type() const
{
    VERIFY(is_type());
    return Type(m_kind - type__start);
}

StringView Token::kind_name() const
{
    switch (*this) {
    case none:          return "none";

    case kw_function:   return "kw_function";
    case kw_if:         return "kw_if";
    case kw_throw:      return "kw_throw";
    case kw_return:     return "kw_return";
    case kw_const:      return "kw_const";

    case type_boolean:  return "type_boolean";
    case type_number:   return "type_number";
    case type_void:     return "type_void";

    case lit_ident:     return "lit_ident";
    case lit_string:    return "lit_string";
    case lit_number:    return "lit_number";

    case sym_lparen:    return "sym_lparen";
    case sym_rparen:    return "sym_rparen";

    case sym_lcurly:    return "sym_lcurly";
    case sym_rcurly:    return "sym_rcurly";

    case sym_fat_arrow: return "sym_fat_arrow";
    case sym_colon:     return "sym_colon";
    case sym_semicolon: return "sym_semicolon";
    case sym_dot:       return "sym_dot";
    case sym_comma:     return "sym_comma";

    case op_lt_eq:      return "op_lt_eq";
    case op_minus:      return "op_minus";
    case op_plus:       return "op_plus";
    case op_assign:     return "op_assign";
    case op_bang:       return "op_bang";
    case op_triple_eq:  return "op_triple_eq";
    }
}

StringView Token::view_in(StringView file) const
{
    u32 size = 0;
    switch (*this) {
    case none: VERIFY(false);

    case kw_function:   size = "function"sv.size(); break;
    case kw_if:         size = "if"sv.size();       break;
    case kw_throw:      size = "throw"sv.size();    break;
    case kw_return:     size = "return"sv.size();   break;
    case kw_const:      size = "const"sv.size();    break;

    case type_boolean:  size = "boolean"sv.size();  break;
    case type_number:   size = "number"sv.size();   break;
    case type_void:     size = "void"sv.size();     break;

    case lit_ident:     size = relex_ident_size(file,   position()); break;
    case lit_string:    size = relex_string_size(file,  position()); break;
    case lit_number:    size = relex_number_size(file,  position()); break;

    case sym_lparen:    size = "("sv.size();        break;
    case sym_rparen:    size = ")"sv.size();        break;

    case sym_lcurly:    size = "{"sv.size();        break;
    case sym_rcurly:    size = "}"sv.size();        break;

    case sym_fat_arrow: size = "=>"sv.size();       break;
    case sym_colon:     size = ":"sv.size();        break;
    case sym_semicolon: size = ";"sv.size();        break;
    case sym_dot:       size = "."sv.size();        break;
    case sym_comma:     size = ","sv.size();        break;

    case op_lt_eq:      size = "<="sv.size();       break;
    case op_minus:      size = "-"sv.size();        break;
    case op_plus:       size = "+"sv.size();        break;
    case op_assign:     size = "="sv.size();        break;
    case op_bang:       size = "!"sv.size();        break;
    case op_triple_eq:  size = "==="sv.size();      break;
    }
    return file.sub_view(position(), size);
}


