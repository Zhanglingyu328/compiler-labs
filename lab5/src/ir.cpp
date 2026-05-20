#include "ir.h"

#include <fstream>
#include <stdexcept>

void IRGenerator::emit(const std::string& op,
                       const std::string& arg1,
                       const std::string& arg2,
                       const std::string& result) {
    quads_.push_back({op, arg1, arg2, result});
}

std::string IRGenerator::newTemp() {
    return "t" + std::to_string(++tempCount_);
}

std::string IRGenerator::newLabel() {
    return "L" + std::to_string(++labelCount_);
}

void IRGenerator::generate(const AST& root) {
    quads_.clear();
    tempCount_ = 0;
    labelCount_ = 0;
    generateStmt(root);
}

void IRGenerator::writeToFile(const std::string& path) const {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("cannot write IR file: " + path);
    }

    print(out);
}

void IRGenerator::print(std::ostream& out) const {
    for (size_t i = 0; i < quads_.size(); ++i) {
        const Quad& q = quads_[i];

        out << i << ": "
            << "("
            << q.op << ", "
            << q.arg1 << ", "
            << q.arg2 << ", "
            << q.result
            << ")"
            << "\n";
    }
}

void IRGenerator::generateChildren(const AST& node) {
    if (!node) {
        return;
    }

    for (const auto& child : node->children) {
        generateStmt(child);
    }
}

std::string IRGenerator::generateLValue(const AST& node) {
    if (!node) {
        return "_";
    }

    if (node->name == "ID") {
        return node->value;
    }

    if (node->name == "ArrayAccess") {
        std::string index = "_";

        if (!node->children.empty()) {
            index = generateExpr(node->children[0]);
        }

        return node->value + "[" + index + "]";
    }

    return generateExpr(node);
}

std::string IRGenerator::generateExpr(const AST& node) {
    if (!node) {
        return "_";
    }

    if (node->name == "IntLiteral" ||
        node->name == "FloatLiteral" ||
        node->name == "StringLiteral") {
        return node->value;
    }

    if (node->name == "ID") {
        return node->value;
    }

    if (node->name == "ArrayAccess") {
        std::string index = "_";

        if (!node->children.empty()) {
            index = generateExpr(node->children[0]);
        }

        std::string temp = newTemp();
        emit("=[]", node->value, index, temp);
        return temp;
    }

    if (node->name == "BinaryOp") {
        std::string left = "_";
        std::string right = "_";

        if (node->children.size() >= 1) {
            left = generateExpr(node->children[0]);
        }

        if (node->children.size() >= 2) {
            right = generateExpr(node->children[1]);
        }

        std::string temp = node->place.empty() ? newTemp() : node->place;
        emit(node->value, left, right, temp);
        return temp;
    }

    if (node->name == "UnaryOp") {
        std::string value = "_";

        if (!node->children.empty()) {
            value = generateExpr(node->children[0]);
        }

        std::string temp = node->place.empty() ? newTemp() : node->place;

        if (node->value == "-") {
            emit("uminus", value, "_", temp);
        } else if (node->value == "!") {
            emit("not", value, "_", temp);
        } else {
            emit(node->value, value, "_", temp);
        }

        return temp;
    }

    if (node->name == "FunctionCall") {
        for (const auto& arg : node->children) {
            std::string argPlace = generateExpr(arg);
            emit("param", argPlace, "_", "_");
        }

        std::string temp = newTemp();
        emit("call", node->value, std::to_string(node->children.size()), temp);
        return temp;
    }

    if (node->name == "Assign" && node->children.size() >= 2) {
        std::string left = generateLValue(node->children[0]);
        std::string right = generateExpr(node->children[1]);
        emit("=", right, "_", left);
        return left;
    }

    return "_";
}

void IRGenerator::generateStmt(const AST& node) {
    if (!node) {
        return;
    }

    if (node->name == "Program") {
        generateChildren(node);
        return;
    }

    if (node->name == "FunctionDecl") {
        emit("func", node->value, node->type, "_");

        for (const auto& child : node->children) {
            if (child && child->name == "Params") {
                for (const auto& p : child->children) {
                    emit("param_decl", p->type, "_", p->value);
                }
            }
        }

        generateChildren(node);
        emit("endfunc", node->value, "_", "_");
        return;
    }

    if (node->name == "Params") {
        return;
    }

    if (node->name == "Block") {
        generateChildren(node);
        return;
    }

    if (node->name == "VarDecl") {
        emit("decl", node->type, "_", node->value);

        if (!node->children.empty()) {
            std::string initValue = generateExpr(node->children[0]);
            emit("=", initValue, "_", node->value);
        }

        return;
    }

    if (node->name == "ArrayDecl") {
        std::string size = "_";

        if (!node->children.empty() && node->children[0]->name == "Size") {
            size = node->children[0]->value;
        }

        emit("array_decl", node->type, size, node->value);
        return;
    }

    if (node->name == "Assign") {
        if (node->children.size() >= 2) {
            std::string left = generateLValue(node->children[0]);
            std::string right = generateExpr(node->children[1]);
            emit("=", right, "_", left);
        }

        return;
    }

    if (node->name == "AddAssign") {
        if (node->children.size() >= 2) {
            std::string left = generateLValue(node->children[0]);
            std::string right = generateExpr(node->children[1]);
            std::string temp = newTemp();

            emit("+", left, right, temp);
            emit("=", temp, "_", left);
        }

        return;
    }

    if (node->name == "PostInc") {
        if (!node->children.empty()) {
            std::string left = generateLValue(node->children[0]);
            emit("+", left, "1", left);
        }

        return;
    }

    if (node->name == "Return") {
        std::string value = "_";

        if (!node->children.empty()) {
            value = generateExpr(node->children[0]);
        }

        emit("return", value, "_", "_");
        return;
    }

    if (node->name == "Print") {
        std::string value = "_";

        if (!node->children.empty()) {
            value = generateExpr(node->children[0]);
        }

        emit("print", value, "_", "_");
        return;
    }

    if (node->name == "ExprStmt") {
        if (!node->children.empty()) {
            generateExpr(node->children[0]);
        }

        return;
    }

    if (node->name == "TopLevelCall") {
        generateChildren(node);
        return;
    }

    if (node->name == "FunctionCall") {
        generateExpr(node);
        return;
    }

    if (node->name == "If") {
        std::string labelElse = newLabel();
        std::string labelEnd = newLabel();

        if (!node->children.empty()) {
            std::string cond = generateExpr(node->children[0]);
            emit("jz", cond, "_", labelElse);
        }

        if (node->children.size() >= 2) {
            generateStmt(node->children[1]);
        }

        emit("jmp", "_", "_", labelEnd);
        emit("label", "_", "_", labelElse);

        if (node->children.size() >= 3) {
            generateStmt(node->children[2]);
        }

        emit("label", "_", "_", labelEnd);
        return;
    }

    if (node->name == "While") {
        std::string labelBegin = newLabel();
        std::string labelEnd = newLabel();

        emit("label", "_", "_", labelBegin);

        if (!node->children.empty()) {
            std::string cond = generateExpr(node->children[0]);
            emit("jz", cond, "_", labelEnd);
        }

        if (node->children.size() >= 2) {
            generateStmt(node->children[1]);
        }

        emit("jmp", "_", "_", labelBegin);
        emit("label", "_", "_", labelEnd);
        return;
    }

    generateChildren(node);
}