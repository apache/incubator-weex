#include "statement.h"
#include "json/json11.hpp"
#include "ast_factory.h"
#include "ast.h"
#include "parser.h"

namespace weex {
namespace core {
namespace data_render {
    
void BlockStatement::PushExpression(Handle<Expression> expr)
{
    stmts_->Insert(expr);
}
        
const std::string &FunctionPrototype::GetName() const
{
    return name_;
}

const std::vector<std::string> &FunctionPrototype::GetArgs() const
{
    return args_;
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
