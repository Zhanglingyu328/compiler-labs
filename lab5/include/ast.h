#pragma once
#include <memory>
#include <ostream>
#include <string>
#include <vector>

struct ASTNode {
    std::string name;
    std::string value;
    std::string type;
    std::string place;
    std::vector<std::shared_ptr<ASTNode>> children;

    ASTNode(std::string n, std::string v = "", std::string t = "")
        : name(std::move(n)), value(std::move(v)), type(std::move(t)) {}
};

using AST = std::shared_ptr<ASTNode>;

AST makeNode(const std::string& name, const std::string& value = "", const std::string& type = "");
void addChild(const AST& parent, const AST& child);
void printAST(const AST& node, std::ostream& out, int indent = 0);
void writeDot(const AST& root, std::ostream& out);
