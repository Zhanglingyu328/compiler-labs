#include "parser.h"
#include <sstream>

Parser::Parser(std::vector<Token> tokens, SemanticContext& sem)
    : tokens_(std::move(tokens)), sem_(sem) {}

std::string Parser::newTemp() {
    return "t" + std::to_string(++tempCounter_);
}

const Token& Parser::peek(int offset) const {
    size_t p = pos_ + static_cast<size_t>(offset);
    if (p >= tokens_.size()) return tokens_.back();
    return tokens_[p];
}

const Token& Parser::previous() const { return tokens_[pos_ - 1]; }
bool Parser::atEnd() const { return peek().type == TokenType::END; }
bool Parser::check(TokenType type) const { return !atEnd() && peek().type == type; }

bool Parser::match(TokenType type) {
    if (check(type)) { advance(); return true; }
    return false;
}

Token Parser::advance() {
    if (!atEnd()) pos_++;
    return previous();
}

Token Parser::expect(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    sem_.addError(peek().line, message + ", got '" + tokenTypeName(peek().type) + "'");
    return Token(type, "", peek().line, peek().col);
}

void Parser::synchronize() {
    while (!atEnd()) {
        if (previous().type == TokenType::SEMI) return;
        switch (peek().type) {
        case TokenType::KW_INT:
        case TokenType::KW_FLOAT:
        case TokenType::KW_VOID:
        case TokenType::KW_IF:
        case TokenType::KW_WHILE:
        case TokenType::KW_RETURN:
        case TokenType::KW_PRINT:
        case TokenType::RBRACE:
            return;
        default:
            advance();
        }
    }
}

void Parser::optionalSemi() { match(TokenType::SEMI); }

bool Parser::isTypeStart(TokenType type) const {
    return type == TokenType::KW_INT || type == TokenType::KW_FLOAT || type == TokenType::KW_VOID;
}

AST Parser::parseProgram() {
    AST root = makeNode("Program");
    while (!atEnd()) {
        AST ext = parseExternal();
        addChild(root, ext);
    }
    return root;
}

AST Parser::parseExternal() {
    if (isTypeStart(peek().type)) return parseFunctionDecl();
    if (check(TokenType::ID)) {
        Token id = advance();
        if (check(TokenType::LPAREN)) {
            Attribute call = parseFunctionCallFromName(id);
            optionalSemi();
            AST stmt = makeNode("TopLevelCall");
            addChild(stmt, call.node);
            return stmt;
        }
        sem_.addError(id.line, "top-level identifier must be a function call");
        synchronize();
        return makeNode("Error");
    }
    sem_.addError(peek().line, "unexpected token at top level: " + tokenTypeName(peek().type));
    advance();
    return makeNode("Error");
}

std::string Parser::parseType() {
    if (match(TokenType::KW_INT)) return "int";
    if (match(TokenType::KW_FLOAT)) return "float";
    if (match(TokenType::KW_VOID)) return "void";
    sem_.addError(peek().line, "type expected");
    return "error";
}

std::vector<ParamInfo> Parser::parseParamList(AST paramsNode) {
    std::vector<ParamInfo> params;
    if (check(TokenType::RPAREN)) return params;

    while (!atEnd() && !check(TokenType::RPAREN)) {
        if (!isTypeStart(peek().type)) {
            sem_.addError(peek().line, "parameter type expected");
            synchronize();
            break;
        }
        int line = peek().line;
        std::string type = parseType();
        Token name = expect(TokenType::ID, "parameter name expected");
        SymbolKind kind = SymbolKind::Param;

        AST pnode = makeNode("Param", name.lexeme, type);
        if (match(TokenType::LBRACKET)) {
            expect(TokenType::RBRACKET, "']' expected after array parameter");
            kind = SymbolKind::Array;
            pnode->name = "ArrayParam";
        } else if (match(TokenType::LPAREN)) {
            expect(TokenType::RPAREN, "')' expected after function parameter");
            pnode->name = "FunctionParam";
        }
        addChild(paramsNode, pnode);
        params.push_back({name.lexeme, type, kind, line});

        if (match(TokenType::SEMI)) continue;
        if (check(TokenType::RPAREN)) break;
        if (match(TokenType::COMMA)) continue;
        sem_.addError(peek().line, "';' expected between parameters");
        break;
    }
    return params;
}

AST Parser::parseFunctionDecl() {
    int line = peek().line;
    std::string retType = parseType();
    Token name = expect(TokenType::ID, "function name expected");
    expect(TokenType::LPAREN, "'(' expected after function name");
    AST paramsNode = makeNode("Params");
    std::vector<ParamInfo> params = parseParamList(paramsNode);
    expect(TokenType::RPAREN, "')' expected after parameter list");

    sem_.declareFunction(name.lexeme, retType, params, line);

    AST fn = makeNode("FunctionDecl", name.lexeme, retType);
    addChild(fn, paramsNode);

    std::string oldType = currentFunctionType_;
    std::string oldName = currentFunctionName_;
    currentFunctionType_ = retType;
    currentFunctionName_ = name.lexeme;

    sem_.enterScope("function:" + name.lexeme);
    for (const auto& p : params) sem_.declareParam(p);
    addChild(fn, parseBlock("block:" + name.lexeme));
    sem_.exitScope();

    currentFunctionType_ = oldType;
    currentFunctionName_ = oldName;

    optionalSemi();
    return fn;
}

AST Parser::parseBlock(const std::string& scopeName) {
    expect(TokenType::LBRACE, "'{' expected to start block");
    sem_.enterScope(scopeName);
    AST block = makeNode("Block");
    while (!atEnd() && !check(TokenType::RBRACE)) {
        addChild(block, parseDeclOrStmt());
    }
    expect(TokenType::RBRACE, "'}' expected to end block");
    sem_.exitScope();
    return block;
}

AST Parser::parseDeclOrStmt() {
    if (isTypeStart(peek().type)) return parseVarDecl();
    return parseStatement();
}

AST Parser::parseVarDecl() {
    int line = peek().line;
    std::string type = parseType();
    Token name = expect(TokenType::ID, "variable name expected");
    bool isArray = false;
    int arraySize = -1;
    AST decl = makeNode("VarDecl", name.lexeme, type);

    if (match(TokenType::LBRACKET)) {
        isArray = true;
        Token sizeTok = expect(TokenType::INT_LIT, "integer array size expected");
        if (!sizeTok.lexeme.empty()) arraySize = std::stoi(sizeTok.lexeme);
        expect(TokenType::RBRACKET, "']' expected after array size");
        decl->name = "ArrayDecl";
        decl->value = name.lexeme;
        decl->type = type;
        addChild(decl, makeNode("Size", sizeTok.lexeme, "int"));
    }

    sem_.declareVariable(name.lexeme, type, line, isArray, arraySize);

    if (match(TokenType::ASSIGN)) {
        Attribute init = parseExpression();
        sem_.checkAssignment(name.lexeme, type, init.type, line);
        addChild(decl, init.node);
    }

    expect(TokenType::SEMI, "';' expected after declaration");
    return decl;
}

AST Parser::parseStatement() {
    if (check(TokenType::KW_IF)) return parseIfStmt();
    if (check(TokenType::KW_WHILE)) return parseWhileStmt();
    if (check(TokenType::KW_RETURN)) return parseReturnStmt();
    if (check(TokenType::KW_PRINT)) return parsePrintStmt();
    return parseAssignOrExprStmt();
}

AST Parser::parseIfStmt() {
    Token kw = advance();
    expect(TokenType::LPAREN, "'(' expected after if");
    Attribute cond = parseExpression();
    expect(TokenType::RPAREN, "')' expected after if condition");
    AST node = makeNode("If");
    addChild(node, cond.node);
    addChild(node, parseBlock("if"));
    if (match(TokenType::KW_ELSE)) addChild(node, parseBlock("else"));
    (void)kw;
    return node;
}

AST Parser::parseWhileStmt() {
    advance();
    expect(TokenType::LPAREN, "'(' expected after while");
    Attribute cond = parseExpression();
    expect(TokenType::RPAREN, "')' expected after while condition");
    AST node = makeNode("While");
    addChild(node, cond.node);
    addChild(node, parseBlock("while"));
    return node;
}

AST Parser::parseReturnStmt() {
    Token ret = advance();
    AST node = makeNode("Return");
    bool hasValue = false;
    std::string actual = "void";
    if (!check(TokenType::SEMI) && !check(TokenType::RBRACE) && !atEnd()) {
        Attribute expr = parseExpression();
        hasValue = true;
        actual = expr.type;
        addChild(node, expr.node);
    }
    optionalSemi();
    sem_.checkReturn(currentFunctionType_.empty() ? "void" : currentFunctionType_, actual, hasValue, ret.line);
    return node;
}

AST Parser::parsePrintStmt() {
    Token pr = advance();
    AST node = makeNode("Print");
    Attribute expr;
    if (match(TokenType::LPAREN)) {
        expr = parseExpression();
        expect(TokenType::RPAREN, "')' expected after print argument");
    } else {
        expr = parseExpression();
    }
    if (expr.type == "string") sem_.addError(pr.line, "string literal is not supported by the base grammar");
    addChild(node, expr.node);
    optionalSemi();
    return node;
}

AST Parser::parseAssignOrExprStmt() {
    if (check(TokenType::ID) &&
        (peek(1).type == TokenType::ASSIGN || peek(1).type == TokenType::PLUS_ASSIGN ||
         peek(1).type == TokenType::PLUS_PLUS || peek(1).type == TokenType::LBRACKET)) {
        size_t save = pos_;
        Attribute lv = parseLValue();
        if (match(TokenType::ASSIGN) || match(TokenType::PLUS_ASSIGN)) {
            Token op = previous();
            Attribute rhs = parseExpression();
            sem_.checkAssignment(lv.lexeme, lv.type, rhs.type, op.line);
            AST node = makeNode(op.type == TokenType::PLUS_ASSIGN ? "AddAssign" : "Assign");
            addChild(node, lv.node);
            addChild(node, rhs.node);
            optionalSemi();
            return node;
        }
        if (match(TokenType::PLUS_PLUS)) {
            if (lv.type != "int" && lv.type != "float" && lv.type != "error") sem_.addError(previous().line, "++ requires numeric variable");
            AST node = makeNode("PostInc");
            addChild(node, lv.node);
            optionalSemi();
            return node;
        }
        pos_ = save;
    }
    Attribute expr = parseExpression();
    optionalSemi();
    AST node = makeNode("ExprStmt");
    addChild(node, expr.node);
    return node;
}

Attribute Parser::parseLValue() {
    Token name = expect(TokenType::ID, "identifier expected");
    const Symbol* sym = sem_.lookup(name.lexeme);
    std::string type = "error";
    bool isArray = false;
    if (!sym) sem_.addError(name.line, "identifier '" + name.lexeme + "' is not declared");
    else {
        if (sym->kind == SymbolKind::Function) sem_.addError(name.line, "function '" + name.lexeme + "' is not assignable");
        type = sym->type;
        isArray = sym->kind == SymbolKind::Array;
    }

    AST node = makeNode("ID", name.lexeme, type);
    if (match(TokenType::LBRACKET)) {
        Attribute idx = parseExpression();
        expect(TokenType::RBRACKET, "']' expected after array index");
        if (!isArray && sym) sem_.addError(name.line, "identifier '" + name.lexeme + "' is not an array");
        if (idx.type != "int" && idx.type != "error") sem_.addError(name.line, "array index must be int");
        AST arr = makeNode("ArrayAccess", name.lexeme, type);
        addChild(arr, idx.node);
        node = arr;
    } else if (isArray) {
        sem_.addWarning(name.line, "array '" + name.lexeme + "' used without index");
    }
    return {node, type, name.lexeme, ""};
}

Attribute Parser::parseExpression() { return parseOr(); }

Attribute Parser::parseOr() {
    Attribute left = parseAnd();
    while (match(TokenType::OR)) {
        Token op = previous();
        Attribute right = parseAnd();
        std::string t = sem_.logicalType(left.type, right.type, op.lexeme, op.line);
        AST n = makeNode("BinaryOp", op.lexeme, t);
        n->place = newTemp();
        addChild(n, left.node); addChild(n, right.node);
        left = {n, t, "", n->place};
    }
    return left;
}

Attribute Parser::parseAnd() {
    Attribute left = parseEquality();
    while (match(TokenType::AND)) {
        Token op = previous();
        Attribute right = parseEquality();
        std::string t = sem_.logicalType(left.type, right.type, op.lexeme, op.line);
        AST n = makeNode("BinaryOp", op.lexeme, t);
        n->place = newTemp();
        addChild(n, left.node); addChild(n, right.node);
        left = {n, t, "", n->place};
    }
    return left;
}

Attribute Parser::parseEquality() {
    Attribute left = parseRelational();
    while (match(TokenType::EQ) || match(TokenType::NE)) {
        Token op = previous();
        Attribute right = parseRelational();
        std::string t = sem_.relationType(left.type, right.type, op.lexeme, op.line);
        AST n = makeNode("BinaryOp", op.lexeme, t);
        n->place = newTemp();
        addChild(n, left.node); addChild(n, right.node);
        left = {n, t, "", n->place};
    }
    return left;
}

Attribute Parser::parseRelational() {
    Attribute left = parseAdditive();
    while (match(TokenType::LT) || match(TokenType::LE) || match(TokenType::GT) || match(TokenType::GE)) {
        Token op = previous();
        Attribute right = parseAdditive();
        std::string t = sem_.relationType(left.type, right.type, op.lexeme, op.line);
        AST n = makeNode("BinaryOp", op.lexeme, t);
        n->place = newTemp();
        addChild(n, left.node); addChild(n, right.node);
        left = {n, t, "", n->place};
    }
    return left;
}

Attribute Parser::parseAdditive() {
    Attribute left = parseMultiplicative();
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = previous();
        Attribute right = parseMultiplicative();
        std::string t = sem_.arithmeticType(left.type, right.type, op.lexeme, op.line);
        AST n = makeNode("BinaryOp", op.lexeme, t);
        n->place = newTemp();
        addChild(n, left.node); addChild(n, right.node);
        left = {n, t, "", n->place};
    }
    return left;
}

Attribute Parser::parseMultiplicative() {
    Attribute left = parseUnary();
    while (match(TokenType::STAR) || match(TokenType::SLASH)) {
        Token op = previous();
        Attribute right = parseUnary();
        std::string t = sem_.arithmeticType(left.type, right.type, op.lexeme, op.line);
        AST n = makeNode("BinaryOp", op.lexeme, t);
        n->place = newTemp();
        addChild(n, left.node); addChild(n, right.node);
        left = {n, t, "", n->place};
    }
    return left;
}

Attribute Parser::parseUnary() {
    if (match(TokenType::MINUS)) {
        Token op = previous();
        Attribute expr = parseUnary();
        if (expr.type != "int" && expr.type != "float" && expr.type != "error") sem_.addError(op.line, "unary minus requires numeric operand");
        sem_.addWarning(op.line, "unary minus is treated as an optional extension");
        AST n = makeNode("UnaryOp", "-", expr.type);
        n->place = newTemp();
        addChild(n, expr.node);
        return {n, expr.type, "", n->place};
    }
    if (match(TokenType::NOT)) {
        Token op = previous();
        Attribute expr = parseUnary();
        if (expr.type != "int" && expr.type != "float" && expr.type != "error") sem_.addError(op.line, "! requires numeric operand");
        sem_.addWarning(op.line, "logical not is treated as an optional extension");
        AST n = makeNode("UnaryOp", "!", "int");
        n->place = newTemp();
        addChild(n, expr.node);
        return {n, "int", "", n->place};
    }
    return parsePrimary();
}

Attribute Parser::parsePrimary() {
    if (match(TokenType::INT_LIT)) {
        Token t = previous();
        AST n = makeNode("IntLiteral", t.lexeme, "int");
        return {n, "int", t.lexeme, t.lexeme};
    }
    if (match(TokenType::FLOAT_LIT)) {
        Token t = previous();
        AST n = makeNode("FloatLiteral", t.lexeme, "float");
        return {n, "float", t.lexeme, t.lexeme};
    }
    if (match(TokenType::STRING_LIT)) {
        Token t = previous();
        AST n = makeNode("StringLiteral", t.lexeme, "string");
        return {n, "string", t.lexeme, t.lexeme};
    }
    if (match(TokenType::ID)) {
        Token id = previous();
        if (check(TokenType::LPAREN)) return parseFunctionCallFromName(id);
        pos_--;
        return parseLValue();
    }
    if (match(TokenType::LPAREN)) {
        Attribute e = parseExpression();
        expect(TokenType::RPAREN, "')' expected after expression");
        return e;
    }
    sem_.addError(peek().line, "expression expected, got '" + tokenTypeName(peek().type) + "'");
    if (!atEnd()) advance();
    return {makeNode("Error"), "error", "", ""};
}

Attribute Parser::parseFunctionCallFromName(const Token& nameTok) {
    expect(TokenType::LPAREN, "'(' expected for function call");
    std::vector<Attribute> args;
    AST call = makeNode("FunctionCall", nameTok.lexeme);
    if (!check(TokenType::RPAREN)) {
        while (!atEnd()) {
            if (check(TokenType::RPAREN)) break;
            Attribute arg = parseExpression();
            args.push_back(arg);
            addChild(call, arg.node);
            if (match(TokenType::COMMA)) {
                if (check(TokenType::RPAREN)) break;
                continue;
            }
            break;
        }
    }
    expect(TokenType::RPAREN, "')' expected after arguments");
    std::string ret = sem_.checkFunctionCall(nameTok.lexeme, args, nameTok.line);
    call->type = ret;
    return {call, ret, nameTok.lexeme, ""};
}
