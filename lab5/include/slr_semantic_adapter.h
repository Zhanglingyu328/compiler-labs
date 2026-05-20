#pragma once
#include "semantic.h"
#include "token.h"
#include <string>
#include <vector>

struct SLRProduction {
    int id = -1;
    std::string lhs;
    std::vector<std::string> rhs;
    std::string note;
};

// This adapter is the bridge from the Lab 4 SLR shift/reduce driver to Lab 5 semantic actions.
// In your Lab 4 parser, call onShift() whenever ACTION is shift, and call onReduce() whenever ACTION is reduce.
class SLRSemanticAdapter {
public:
    explicit SLRSemanticAdapter(SemanticContext& sem);

    void onShift(const Token& tok);
    Attribute onReduce(const SLRProduction& prod);

    const std::vector<Attribute>& stack() const { return attrStack_; }
    Attribute finalAttribute() const;

private:
    SemanticContext& sem_;
    std::vector<Attribute> attrStack_;
    int tempCounter_ = 0;

    std::string newTemp();
    Attribute makeTokenAttribute(const Token& tok) const;
    std::vector<Attribute> popRhs(size_t count);

    Attribute makeType(const std::string& typeName);
    Attribute makeBinary(const std::string& op, const Attribute& left, const Attribute& right, int line);
    Attribute makeGenericNode(const SLRProduction& prod, const std::vector<Attribute>& rhs);
};
