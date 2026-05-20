#pragma once
#include "ast.h"
#include "semantic.h"
#include "token.h"
#include <string>
#include <vector>

class Parser {
public:
    Parser(std::vector<Token> tokens, SemanticContext& sem);
    AST parseProgram();

private:
    std::vector<Token> tokens_;
    size_t pos_ = 0;
    SemanticContext& sem_;
    std::string currentFunctionType_;
    std::string currentFunctionName_;
    int tempCounter_ = 0;

    std::string newTemp();

    const Token& peek(int offset = 0) const;
    const Token& previous() const;
    bool atEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token advance();
    Token expect(TokenType type, const std::string& message);
    void synchronize();
    void optionalSemi();
    bool isTypeStart(TokenType type) const;

    AST parseExternal();
    std::string parseType();
    std::vector<ParamInfo> parseParamList(AST paramsNode);
    AST parseFunctionDecl();
    AST parseBlock(const std::string& scopeName);
    AST parseDeclOrStmt();
    AST parseVarDecl();
    AST parseStatement();
    AST parseIfStmt();
    AST parseWhileStmt();
    AST parseReturnStmt();
    AST parsePrintStmt();
    AST parseAssignOrExprStmt();

    Attribute parseLValue();
    Attribute parseExpression();
    Attribute parseOr();
    Attribute parseAnd();
    Attribute parseEquality();
    Attribute parseRelational();
    Attribute parseAdditive();
    Attribute parseMultiplicative();
    Attribute parseUnary();
    Attribute parsePrimary();
    Attribute parseFunctionCallFromName(const Token& nameTok);
};
