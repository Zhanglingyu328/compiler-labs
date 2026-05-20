#include "semantic.h"
#include <sstream>

void SemanticContext::enterScope(const std::string& name) { symbols.enterScope(name); }
void SemanticContext::exitScope() { symbols.exitScope(); }

void SemanticContext::addError(int line, const std::string& message) {
    std::ostringstream oss;
    oss << "Line " << line << ": " << message;
    errors.push_back(oss.str());
}

void SemanticContext::addWarning(int line, const std::string& message) {
    std::ostringstream oss;
    oss << "Line " << line << ": " << message;
    warnings.push_back(oss.str());
}

void SemanticContext::declareVariable(const std::string& name, const std::string& type, int line, bool isArray, int arraySize) {
    Symbol s;
    s.name = name;
    s.type = type;
    s.kind = isArray ? SymbolKind::Array : SymbolKind::Var;
    s.arraySize = arraySize;
    s.line = line;
    std::string err;
    if (!symbols.declare(s, err)) addError(line, err);
}

void SemanticContext::declareFunction(const std::string& name, const std::string& returnType, const std::vector<ParamInfo>& params, int line) {
    Symbol s;
    s.name = name;
    s.type = returnType;
    s.kind = SymbolKind::Function;
    s.line = line;
    for (const auto& p : params) {
        s.paramTypes.push_back(p.type);
        s.paramNames.push_back(p.name);
    }
    std::string err;
    if (!symbols.declare(s, err)) addError(line, err);
}

void SemanticContext::declareParam(const ParamInfo& param) {
    Symbol s;
    s.name = param.name;
    s.type = param.type;
    s.kind = param.kind;
    s.line = param.line;
    std::string err;
    if (!symbols.declare(s, err)) addError(param.line, err);
}

const Symbol* SemanticContext::lookup(const std::string& name) const { return symbols.lookup(name); }

const Symbol* SemanticContext::lookupFunction(const std::string& name) const {
    const Symbol* s = symbols.lookupGlobal(name);
    if (s && s->kind == SymbolKind::Function) return s;
    return nullptr;
}

std::string SemanticContext::arithmeticType(const std::string& left, const std::string& right, const std::string& op, int line) {
    if (left == "error" || right == "error") return "error";
    if ((left == "int" || left == "float") && (right == "int" || right == "float")) {
        if (left == "float" || right == "float") return "float";
        return "int";
    }
    addError(line, "operator '" + op + "' cannot be applied to " + left + " and " + right);
    return "error";
}

std::string SemanticContext::relationType(const std::string& left, const std::string& right, const std::string& op, int line) {
    if (left == "error" || right == "error") return "error";
    if ((left == "int" || left == "float") && (right == "int" || right == "float")) return "int";
    addError(line, "relation operator '" + op + "' cannot be applied to " + left + " and " + right);
    return "error";
}

std::string SemanticContext::logicalType(const std::string& left, const std::string& right, const std::string& op, int line) {
    addWarning(line, "logical operator '" + op + "' is treated as an optional extension");
    if (left == "error" || right == "error") return "error";
    if ((left == "int" || left == "float") && (right == "int" || right == "float")) return "int";
    addError(line, "logical operator '" + op + "' cannot be applied to " + left + " and " + right);
    return "error";
}

bool SemanticContext::canAssign(const std::string& lhs, const std::string& rhs) const {
    if (lhs == "error" || rhs == "error") return true;
    if (lhs == rhs) return true;
    if (lhs == "float" && rhs == "int") return true;
    return false;
}

void SemanticContext::checkAssignment(const std::string& name, const std::string& lhs, const std::string& rhs, int line) {
    if (!canAssign(lhs, rhs)) addError(line, "cannot assign " + rhs + " to " + lhs + " variable '" + name + "'");
}

std::string SemanticContext::checkFunctionCall(const std::string& name, const std::vector<Attribute>& args, int line) {
    const Symbol* f = lookupFunction(name);
    if (!f) {
        addError(line, "function '" + name + "' is not declared");
        return "error";
    }
    if (args.size() != f->paramTypes.size()) {
        addError(line, "function '" + name + "' expects " + std::to_string(f->paramTypes.size()) +
                       " argument(s), got " + std::to_string(args.size()));
    }
    size_t n = args.size() < f->paramTypes.size() ? args.size() : f->paramTypes.size();
    for (size_t i = 0; i < n; ++i) {
        if (!canAssign(f->paramTypes[i], args[i].type)) {
            addError(line, "argument " + std::to_string(i + 1) + " of '" + name + "' expects " +
                           f->paramTypes[i] + ", got " + args[i].type);
        }
    }
    return f->type;
}

void SemanticContext::checkReturn(const std::string& expected, const std::string& actual, bool hasValue, int line) {
    if (expected == "void") {
        if (hasValue) addError(line, "void function should not return a value");
        return;
    }
    if (!hasValue) {
        addError(line, "non-void function must return a value of type " + expected);
        return;
    }
    if (!canAssign(expected, actual)) {
        addError(line, "return type mismatch: expected " + expected + ", got " + actual);
    }
}
