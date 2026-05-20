#include "ast.h"
#include <functional>
#include <sstream>

AST makeNode(const std::string& name, const std::string& value, const std::string& type) {
    return std::make_shared<ASTNode>(name, value, type);
}

void addChild(const AST& parent, const AST& child) {
    if (parent && child) parent->children.push_back(child);
}

static std::string nodeText(const AST& node) {
    if (!node) return "<null>";
    std::string s = node->name;
    if (!node->value.empty()) s += "(" + node->value + ")";
    if (!node->type.empty()) s += " : " + node->type;
    if (!node->place.empty()) s += " @" + node->place;
    return s;
}

void printAST(const AST& node, std::ostream& out, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; ++i) out << "  ";
    out << nodeText(node) << "\n";
    for (const auto& child : node->children) printAST(child, out, indent + 1);
}

static std::string escapeLabel(const std::string& s) {
    std::string r;
    for (char ch : s) {
        if (ch == '"') r += "\\\"";
        else if (ch == '\\') r += "\\\\";
        else if (ch == '\n') r += "\\n";
        else r += ch;
    }
    return r;
}

void writeDot(const AST& root, std::ostream& out) {
    out << "digraph AST {\n";
    out << "  node [shape=box];\n";
    int id = 0;
    std::function<int(AST)> dfs = [&](AST node) -> int {
        int my = id++;
        out << "  n" << my << " [label=\"" << escapeLabel(nodeText(node)) << "\"];\n";
        for (const auto& child : node->children) {
            int cid = dfs(child);
            out << "  n" << my << " -> n" << cid << ";\n";
        }
        return my;
    };
    if (root) dfs(root);
    out << "}\n";
}
