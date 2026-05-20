#pragma once
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

enum class SymbolKind { Var, Array, Function, Param };

std::string kindName(SymbolKind k);

struct Symbol {
    std::string name;
    std::string type;
    SymbolKind kind = SymbolKind::Var;
    int scopeLevel = 0;
    std::string scopeName;
    int arraySize = -1;
    std::vector<std::string> paramTypes;
    std::vector<std::string> paramNames;
    int line = 0;
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope(const std::string& name);
    void exitScope();
    int currentLevel() const;
    std::string currentScopeName() const;

    bool declare(const Symbol& symbol, std::string& error);
    const Symbol* lookup(const std::string& name) const;
    const Symbol* lookupCurrent(const std::string& name) const;
    const Symbol* lookupGlobal(const std::string& name) const;

    void print(std::ostream& out) const;

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
    std::vector<std::string> scopeNames_;
    std::vector<Symbol> allSymbols_;
};
