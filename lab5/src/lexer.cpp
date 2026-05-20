#include "lexer.h"
#include <cctype>
#include <sstream>
#include <unordered_map>

Lexer::Lexer(std::string source) : src_(std::move(source)) {}

bool Lexer::atEnd() const { return pos_ >= src_.size(); }

char Lexer::peek(int offset) const {
    size_t p = pos_ + static_cast<size_t>(offset);
    if (p >= src_.size()) return '\0';
    return src_[p];
}

char Lexer::advance() {
    if (atEnd()) return '\0';
    char ch = src_[pos_++];
    if (ch == '\n') { line_++; col_ = 1; }
    else { col_++; }
    return ch;
}

bool Lexer::match(char expected) {
    if (atEnd() || src_[pos_] != expected) return false;
    advance();
    return true;
}

Token Lexer::make(TokenType type, const std::string& lexeme, int line, int col) const {
    return Token(type, lexeme, line, col);
}

void Lexer::skipWhitespaceAndComments() {
    while (!atEnd()) {
        char ch = peek();
        if (std::isspace(static_cast<unsigned char>(ch))) { advance(); continue; }
        if (ch == '/' && peek(1) == '/') {
            while (!atEnd() && peek() != '\n') advance();
            continue;
        }
        break;
    }
}

Token Lexer::identifier(int line, int col) {
    std::string text;
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') text += advance();

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"int", TokenType::KW_INT}, {"float", TokenType::KW_FLOAT}, {"void", TokenType::KW_VOID},
        {"if", TokenType::KW_IF}, {"else", TokenType::KW_ELSE}, {"while", TokenType::KW_WHILE},
        {"return", TokenType::KW_RETURN}, {"input", TokenType::KW_INPUT}, {"print", TokenType::KW_PRINT}
    };
    auto it = keywords.find(text);
    if (it != keywords.end()) return make(it->second, text, line, col);
    return make(TokenType::ID, text, line, col);
}

Token Lexer::number(int line, int col) {
    std::string text;
    bool hasDot = false;
    while (std::isdigit(static_cast<unsigned char>(peek())) || peek() == '.') {
        if (peek() == '.') {
            if (hasDot) break;
            hasDot = true;
        }
        text += advance();
    }
    return make(hasDot ? TokenType::FLOAT_LIT : TokenType::INT_LIT, text, line, col);
}

Token Lexer::stringLiteral(int line, int col) {
    std::string text;
    advance();
    while (!atEnd() && peek() != '"') {
        text += advance();
    }
    if (atEnd()) {
        std::ostringstream oss;
        oss << "Line " << line << ": unterminated string literal";
        errors_.push_back(oss.str());
        return make(TokenType::INVALID, text, line, col);
    }
    advance();
    return make(TokenType::STRING_LIT, text, line, col);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!atEnd()) {
        skipWhitespaceAndComments();
        if (atEnd()) break;
        int line = line_;
        int col = col_;
        char ch = peek();

        if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
            tokens.push_back(identifier(line, col));
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            tokens.push_back(number(line, col));
            continue;
        }
        if (ch == '"') {
            tokens.push_back(stringLiteral(line, col));
            continue;
        }

        switch (advance()) {
        case '+':
            if (match('=')) tokens.push_back(make(TokenType::PLUS_ASSIGN, "+=", line, col));
            else if (match('+')) tokens.push_back(make(TokenType::PLUS_PLUS, "++", line, col));
            else tokens.push_back(make(TokenType::PLUS, "+", line, col));
            break;
        case '-': tokens.push_back(make(TokenType::MINUS, "-", line, col)); break;
        case '*': tokens.push_back(make(TokenType::STAR, "*", line, col)); break;
        case '/': tokens.push_back(make(TokenType::SLASH, "/", line, col)); break;
        case '=':
            if (match('=')) tokens.push_back(make(TokenType::EQ, "==", line, col));
            else tokens.push_back(make(TokenType::ASSIGN, "=", line, col));
            break;
        case '<':
            if (match('=')) tokens.push_back(make(TokenType::LE, "<=", line, col));
            else tokens.push_back(make(TokenType::LT, "<", line, col));
            break;
        case '>':
            if (match('=')) tokens.push_back(make(TokenType::GE, ">=", line, col));
            else tokens.push_back(make(TokenType::GT, ">", line, col));
            break;
        case '!':
            if (match('=')) tokens.push_back(make(TokenType::NE, "!=", line, col));
            else tokens.push_back(make(TokenType::NOT, "!", line, col));
            break;
        case '&':
            if (match('&')) tokens.push_back(make(TokenType::AND, "&&", line, col));
            else {
                errors_.push_back("Line " + std::to_string(line) + ": unexpected '&' (did you mean &&?)");
                tokens.push_back(make(TokenType::INVALID, "&", line, col));
            }
            break;
        case '|':
            if (match('|')) tokens.push_back(make(TokenType::OR, "||", line, col));
            else {
                errors_.push_back("Line " + std::to_string(line) + ": unexpected '|' (did you mean ||?)");
                tokens.push_back(make(TokenType::INVALID, "|", line, col));
            }
            break;
        case '(': tokens.push_back(make(TokenType::LPAREN, "(", line, col)); break;
        case ')': tokens.push_back(make(TokenType::RPAREN, ")", line, col)); break;
        case '{': tokens.push_back(make(TokenType::LBRACE, "{", line, col)); break;
        case '}': tokens.push_back(make(TokenType::RBRACE, "}", line, col)); break;
        case '[': tokens.push_back(make(TokenType::LBRACKET, "[", line, col)); break;
        case ']': tokens.push_back(make(TokenType::RBRACKET, "]", line, col)); break;
        case ',': tokens.push_back(make(TokenType::COMMA, ",", line, col)); break;
        case ';': tokens.push_back(make(TokenType::SEMI, ";", line, col)); break;
        default:
            errors_.push_back("Line " + std::to_string(line) + ": invalid character");
            tokens.push_back(make(TokenType::INVALID, std::string(1, ch), line, col));
            break;
        }
    }
    tokens.emplace_back(TokenType::END, "", line_, col_);
    return tokens;
}
