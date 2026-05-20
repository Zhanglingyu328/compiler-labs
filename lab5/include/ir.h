#pragma once

#include "ast.h"

#include <ostream>
#include <string>
#include <vector>

struct Quad {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
};

class IRGenerator {
public:
    void generate(const AST& root);
    void writeToFile(const std::string& path) const;
    void print(std::ostream& out) const;

private:
    int tempCount_ = 0;
    int labelCount_ = 0;
    std::vector<Quad> quads_;

    std::string newTemp();
    std::string newLabel();

    std::string generateExpr(const AST& node);
    std::string generateLValue(const AST& node);

    void generateStmt(const AST& node);
    void generateChildren(const AST& node);

    void emit(const std::string& op,
              const std::string& arg1,
              const std::string& arg2,
              const std::string& result);
};