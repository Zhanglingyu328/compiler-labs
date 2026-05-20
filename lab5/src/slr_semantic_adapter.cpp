#include "slr_semantic_adapter.h"
#include <algorithm>
#include <stdexcept>

SLRSemanticAdapter::SLRSemanticAdapter(SemanticContext& sem) : sem_(sem) {}

std::string SLRSemanticAdapter::newTemp() { return "t" + std::to_string(++tempCounter_); }

Attribute SLRSemanticAdapter::makeTokenAttribute(const Token& tok) const {
    Attribute a;
    a.lexeme = tok.lexeme;
    switch (tok.type) {
    case TokenType::ID:
        a.node = makeNode("ID", tok.lexeme);
        break;
    case TokenType::INT_LIT:
        a.type = "int";
        a.node = makeNode("NUM", tok.lexeme, "int");
        a.place = tok.lexeme;
        break;
    case TokenType::FLOAT_LIT:
        a.type = "float";
        a.node = makeNode("FLO", tok.lexeme, "float");
        a.place = tok.lexeme;
        break;
    case TokenType::KW_INT:
        a.type = "int";
        a.node = makeNode("Type", "int", "int");
        break;
    case TokenType::KW_FLOAT:
        a.type = "float";
        a.node = makeNode("Type", "float", "float");
        break;
    case TokenType::KW_VOID:
        a.type = "void";
        a.node = makeNode("Type", "void", "void");
        break;
    default:
        a.node = makeNode(tokenTypeName(tok.type), tok.lexeme);
        break;
    }
    return a;
}

void SLRSemanticAdapter::onShift(const Token& tok) {
    attrStack_.push_back(makeTokenAttribute(tok));
}

std::vector<Attribute> SLRSemanticAdapter::popRhs(size_t count) {
    if (count > attrStack_.size()) throw std::runtime_error("semantic attribute stack underflow");
    std::vector<Attribute> rhs;
    rhs.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        rhs.push_back(attrStack_.back());
        attrStack_.pop_back();
    }
    std::reverse(rhs.begin(), rhs.end());
    return rhs;
}

Attribute SLRSemanticAdapter::makeType(const std::string& typeName) {
    Attribute a;
    a.type = typeName;
    a.lexeme = typeName;
    a.node = makeNode("Type", typeName, typeName);
    return a;
}

Attribute SLRSemanticAdapter::makeBinary(const std::string& op, const Attribute& left, const Attribute& right, int line) {
    Attribute a;
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        a.type = sem_.arithmeticType(left.type, right.type, op, line);
    } else if (op == "<" || op == "<=" || op == "==" || op == ">" || op == ">=" || op == "!=") {
        a.type = sem_.relationType(left.type, right.type, op, line);
    } else {
        a.type = sem_.logicalType(left.type, right.type, op, line);
    }
    a.node = makeNode("BinaryOp", op, a.type);
    a.place = newTemp();
    a.node->place = a.place;
    addChild(a.node, left.node);
    addChild(a.node, right.node);
    return a;
}

Attribute SLRSemanticAdapter::makeGenericNode(const SLRProduction& prod, const std::vector<Attribute>& rhs) {
    Attribute a;
    a.node = makeNode(prod.lhs);
    for (const auto& r : rhs) addChild(a.node, r.node);
    if (!rhs.empty()) {
        a.type = rhs.front().type;
        a.place = rhs.front().place;
    }
    return a;
}

Attribute SLRSemanticAdapter::onReduce(const SLRProduction& prod) {
    std::vector<Attribute> rhs = popRhs(prod.rhs.size());
    Attribute lhs;

    // Common Lab 5 semantic actions. Your production ids/names can be mapped here.
    if (prod.lhs == "T" && prod.rhs.size() == 1) {
        lhs = makeType(rhs[0].lexeme.empty() ? rhs[0].type : rhs[0].lexeme);
    } else if ((prod.lhs == "D" || prod.lhs == "Decl") && rhs.size() >= 2 && rhs[0].node && rhs[1].node) {
        // D -> T d
        std::string type = rhs[0].type;
        std::string name = rhs[1].lexeme;
        sem_.declareVariable(name, type, 0, false, -1);
        lhs.node = makeNode("VarDecl", name, type);
        addChild(lhs.node, rhs[0].node);
        addChild(lhs.node, rhs[1].node);
    } else if ((prod.lhs == "E" || prod.lhs == "B") && rhs.size() == 1) {
        // E -> d | i | f | (already-built node)
        lhs = rhs[0];
        if (rhs[0].node && rhs[0].node->name == "ID") {
            const Symbol* sym = sem_.lookup(rhs[0].lexeme);
            if (!sym) {
                sem_.addError(0, "variable '" + rhs[0].lexeme + "' is not declared");
                lhs.type = "error";
            } else {
                lhs.type = sym->type;
                lhs.node->type = sym->type;
            }
        }
    } else if ((prod.lhs == "E" || prod.lhs == "B") && rhs.size() == 3 && rhs[1].node) {
        // E -> E op E, B -> E r E, E -> ( E )
        std::string op = rhs[1].lexeme;
        if (op == "(" || prod.rhs[0] == "(") lhs = rhs[1];
        else lhs = makeBinary(op, rhs[0], rhs[2], 0);
    } else if ((prod.lhs == "S" || prod.lhs == "Stmt") && rhs.size() >= 3 && rhs[0].node && rhs[0].node->name == "ID") {
        // S -> d = E
        std::string name = rhs[0].lexeme;
        const Symbol* sym = sem_.lookup(name);
        std::string lhsType = sym ? sym->type : "error";
        if (!sym) sem_.addError(0, "variable '" + name + "' is not declared");
        sem_.checkAssignment(name, lhsType, rhs[2].type, 0);
        lhs.node = makeNode("Assign");
        addChild(lhs.node, rhs[0].node);
        addChild(lhs.node, rhs[2].node);
    } else {
        lhs = makeGenericNode(prod, rhs);
    }

    attrStack_.push_back(lhs);
    return lhs;
}

Attribute SLRSemanticAdapter::finalAttribute() const {
    if (attrStack_.empty()) return Attribute{};
    return attrStack_.back();
}
