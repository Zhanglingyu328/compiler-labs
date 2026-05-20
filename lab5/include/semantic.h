#pragma once
#include "ast.h"
#include "symbols.h"
#include <string>
#include <vector>

struct Attribute {
    AST node;
    std::string type;
    std::string lexeme;
    std::string place;
};

struct ParamInfo {
    std::string name;
    std::string type;
    SymbolKind kind = SymbolKind::Param;
    int line = 0;
};

class SemanticContext {
public:
    SymbolTable symbols;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    void enterScope(const std::string& name);
    void exitScope();

    void declareVariable(const std::string& name, const std::string& type, int line, bool isArray = false, int arraySize = -1);
    void declareFunction(const std::string& name, const std::string& returnType, const std::vector<ParamInfo>& params, int line);
    void declareParam(const ParamInfo& param);

    const Symbol* lookup(const std::string& name) const;
    const Symbol* lookupFunction(const std::string& name) const;

    std::string arithmeticType(const std::string& left, const std::string& right, const std::string& op, int line);
    std::string relationType(const std::string& left, const std::string& right, const std::string& op, int line);
    std::string logicalType(const std::string& left, const std::string& right, const std::string& op, int line);
    bool canAssign(const std::string& lhs, const std::string& rhs) const;
    void checkAssignment(const std::string& name, const std::string& lhs, const std::string& rhs, int line);
    std::string checkFunctionCall(const std::string& name, const std::vector<Attribute>& args, int line);
    void checkReturn(const std::string& expected, const std::string& actual, bool hasValue, int line);

    void addError(int line, const std::string& message);
    void addWarning(int line, const std::string& message);
};
