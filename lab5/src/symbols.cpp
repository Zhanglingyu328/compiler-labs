#include "symbols.h"
#include <iomanip>

std::string kindName(SymbolKind k) {
    switch (k) {
    case SymbolKind::Var: return "var";
    case SymbolKind::Array: return "array";
    case SymbolKind::Function: return "function";
    case SymbolKind::Param: return "param";
    }
    return "unknown";
}

SymbolTable::SymbolTable() {
    scopes_.push_back({});
    scopeNames_.push_back("global");
}

void SymbolTable::enterScope(const std::string& name) {
    scopes_.push_back({});
    scopeNames_.push_back(name);
}

void SymbolTable::exitScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
        scopeNames_.pop_back();
    }
}

int SymbolTable::currentLevel() const { return static_cast<int>(scopes_.size()) - 1; }

std::string SymbolTable::currentScopeName() const { return scopeNames_.empty() ? "global" : scopeNames_.back(); }

bool SymbolTable::declare(const Symbol& in, std::string& error) {
    if (scopes_.empty()) return false;
    auto& scope = scopes_.back();
    if (scope.find(in.name) != scope.end()) {
        error = "duplicate declaration of '" + in.name + "' in scope '" + currentScopeName() + "'";
        return false;
    }
    Symbol s = in;
    s.scopeLevel = currentLevel();
    s.scopeName = currentScopeName();
    scope[s.name] = s;
    allSymbols_.push_back(s);
    return true;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto f = it->find(name);
        if (f != it->end()) return &f->second;
    }
    return nullptr;
}

const Symbol* SymbolTable::lookupCurrent(const std::string& name) const {
    if (scopes_.empty()) return nullptr;
    auto f = scopes_.back().find(name);
    if (f == scopes_.back().end()) return nullptr;
    return &f->second;
}

const Symbol* SymbolTable::lookupGlobal(const std::string& name) const {
    if (scopes_.empty()) return nullptr;
    auto f = scopes_.front().find(name);
    if (f == scopes_.front().end()) return nullptr;
    return &f->second;
}

void SymbolTable::print(std::ostream& out) const {
    out << std::left << std::setw(16) << "name" << std::setw(10) << "kind"
        << std::setw(10) << "type" << std::setw(8) << "scope" << std::setw(14)
        << "scopeName" << std::setw(10) << "arraySize" << "params" << "\n";
    out << std::string(86, '-') << "\n";
    for (const auto& s : allSymbols_) {
        out << std::left << std::setw(16) << s.name << std::setw(10) << kindName(s.kind)
            << std::setw(10) << s.type << std::setw(8) << s.scopeLevel << std::setw(14)
            << s.scopeName;
        if (s.kind == SymbolKind::Array) out << std::setw(10) << s.arraySize;
        else out << std::setw(10) << "-";
        if (!s.paramTypes.empty()) {
            out << "(";
            for (size_t i = 0; i < s.paramTypes.size(); ++i) {
                if (i) out << ",";
                out << s.paramTypes[i];
                if (i < s.paramNames.size()) out << " " << s.paramNames[i];
            }
            out << ")";
        }
        out << "\n";
    }
}
