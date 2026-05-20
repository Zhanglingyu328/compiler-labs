#pragma once
#include "token.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();
    const std::vector<std::string>& errors() const { return errors_; }

private:
    std::string src_;
    size_t pos_ = 0;
    int line_ = 1;
    int col_ = 1;
    std::vector<std::string> errors_;

    bool atEnd() const;
    char peek(int offset = 0) const;
    char advance();
    bool match(char expected);
    void skipWhitespaceAndComments();
    Token make(TokenType type, const std::string& lexeme, int line, int col) const;
    Token identifier(int line, int col);
    Token number(int line, int col);
    Token stringLiteral(int line, int col);
};
