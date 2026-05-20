#include "token.h"

std::string tokenTypeName(TokenType t) {
    switch (t) {
    case TokenType::END: return "EOF";
    case TokenType::INVALID: return "INVALID";
    case TokenType::ID: return "ID";
    case TokenType::INT_LIT: return "INT_LIT";
    case TokenType::FLOAT_LIT: return "FLOAT_LIT";
    case TokenType::STRING_LIT: return "STRING_LIT";
    case TokenType::KW_INT: return "int";
    case TokenType::KW_FLOAT: return "float";
    case TokenType::KW_VOID: return "void";
    case TokenType::KW_IF: return "if";
    case TokenType::KW_ELSE: return "else";
    case TokenType::KW_WHILE: return "while";
    case TokenType::KW_RETURN: return "return";
    case TokenType::KW_INPUT: return "input";
    case TokenType::KW_PRINT: return "print";
    case TokenType::PLUS: return "+";
    case TokenType::MINUS: return "-";
    case TokenType::STAR: return "*";
    case TokenType::SLASH: return "/";
    case TokenType::ASSIGN: return "=";
    case TokenType::PLUS_ASSIGN: return "+=";
    case TokenType::PLUS_PLUS: return "++";
    case TokenType::LT: return "<";
    case TokenType::LE: return "<=";
    case TokenType::EQ: return "==";
    case TokenType::GT: return ">";
    case TokenType::GE: return ">=";
    case TokenType::NE: return "!=";
    case TokenType::AND: return "&&";
    case TokenType::OR: return "||";
    case TokenType::NOT: return "!";
    case TokenType::LPAREN: return "(";
    case TokenType::RPAREN: return ")";
    case TokenType::LBRACE: return "{";
    case TokenType::RBRACE: return "}";
    case TokenType::LBRACKET: return "[";
    case TokenType::RBRACKET: return "]";
    case TokenType::COMMA: return ",";
    case TokenType::SEMI: return ";";
    }
    return "UNKNOWN";
}
