#ifndef WEEX_AST_VISITOR_H_
#define WEEX_AST_VISITOR_H_

#include "ast.h"
#include "core/data_render/statement.h"

namespace weex {
namespace core {
namespace data_render {

// More strict visitor which wants you to override every visit member
class BasicASTVisitor {
public:
    BasicASTVisitor() = default;
    virtual ~BasicASTVisitor() = default;

protected:

#define DECLARE_VISITOR_METHOD(type) virtual void Visit(type *) = 0;
AST_NODE_LIST(DECLARE_VISITOR_METHOD)
#undef DECLARE_VISITOR_METHOD
};

// Lesser strict version of above visitor
class ASTVisitor : public BasicASTVisitor {
public:
    void visit(Handle<Expression> expr) { expr->Accept(this); }
#define DECLARE_VISITOR_METHOD(type) void Visit(type *) override { throw std::runtime_error("Not implemented walker for " #type); }
AST_NODE_LIST(DECLARE_VISITOR_METHOD)
#undef DECLARE_VISITOR_METHOD
};

template <typename NodeType>
class SingleNodeVisitor {
public:
  void Visit(NodeType *node) {};
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif
