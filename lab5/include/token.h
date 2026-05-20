#pragma once
#include <string>

enum class TokenType {
    END,
    INVALID,
    ID,
    INT_LIT,
    FLOAT_LIT,
    STRING_LIT,
    KW_INT,
    KW_FLOAT,
    KW_VOID,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_RETURN,
    KW_INPUT,
    KW_PRINT,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    ASSIGN,
    PLUS_ASSIGN,
    PLUS_PLUS,
    LT,
    LE,
    EQ,
    GT,
    GE,
    NE,
    AND,
    OR,
    NOT,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    COMMA,
    SEMI
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int col;

    Token(TokenType t = TokenType::END, std::string x = "", int l = 1, int c = 1)
        : type(t), lexeme(std::move(x)), line(l), col(c) {}
};

std::string tokenTypeName(TokenType t);
