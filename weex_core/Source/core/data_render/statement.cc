#include "statement.h"
#include "json11.hpp"
#include "ast_factory.h"
#include "ast.h"
#include "parser.h"

namespace weex {
namespace core {
namespace data_render {

void BlockStatement::Pasing(Json &json) {
    do {
        ASTFactory *factory = ASTFactory::GetFactoryInstance();
        Json control = json["control"];
        Handle<Expression> controlExpr = nullptr;
        if (!control.is_null()) {
            controlExpr = factory->NewControlStatement(json);
            if (controlExpr) {
                PushExpression(controlExpr);
            }
            break;
        }
        Json nodeId = json["nodeId"];
        Json tagName = json["tagName"];
        Handle<Expression> nodeIdExpr = nullptr;
        if (nodeId.is_string() && tagName.is_string()) {
            Handle<Expression> callExpr = nullptr;
            std::vector<Handle<Expression>> args;
            Handle<Expression> func = factory->NewIdentifier(json, "createElement");
            args.push_back(factory->NewStringLiteral(tagName, tagName.string_value()));
            if (controlExpr && controlExpr->IsForStatement()) {
                Json repeat = control["repeat"];
                Json index = repeat["index"];
                nodeIdExpr = factory->NewBinaryExpression(nodeId, BinaryOperation::kAnd, factory->NewStringLiteral(nodeId, nodeId.string_value() + "_REPEAT_"), factory->NewIdentifier(nodeId, index.string_value()));
                args.push_back(nodeIdExpr);
            }
            else {
                nodeIdExpr = factory->NewStringLiteral(nodeId, nodeId.string_value());
                args.push_back(nodeIdExpr);
            }
            callExpr = factory->NewCallExpression(json, func, args);
            if (controlExpr) {
                if (controlExpr->IsForStatement()) {
                    Handle<ForStatement> forExpr = controlExpr;
                    Handle<BlockStatement> forBlock = forExpr->body();
                    forBlock->PushExpression(callExpr);
                }
            }
            else {
                PushExpression(callExpr);
            }
        }
        Json childs = json["childNodes"];
        if (childs.is_array() && childs.array_items().size() > 0) {
            Handle<ExpressionList> list = factory->NewExpressionList();
            for (int i = 0;i < childs.array_items().size(); i++) {
                Json child = childs[i];
                if (child.is_null() || !child.is_object()) {
                    continue; 
                }
                Handle<Expression> block = factory->NewChildBlockStatement(child, factory->NewExpressionList(), nodeId.string_value());
                list->Insert(block);
            }
            if (controlExpr) {
                if (controlExpr->IsForStatement()) {
                    Handle<ForStatement> forExpr = controlExpr;
                    Handle<BlockStatement> forBlock = forExpr->body();
                    forBlock->PushExpression(list);
                }
            }
            else {
                PushExpression(list);
            }
        }
        
    } while (0);
}
    
void BlockStatement::PushExpression(Handle<Expression> expr)
{
    stmts_->Insert(expr);
}
    
void ChunkStatement::Pasing(Json &json) {
    do {
        ASTFactory *factory = ASTFactory::GetFactoryInstance();
        Json control = json["control"];
        Handle<Expression> controlExpr = nullptr;
        if (!control.is_null()) {
            controlExpr = factory->NewControlStatement(json);
            if (controlExpr) {
                PushExpression(controlExpr);
            }
            break;
        }
        Json nodeId = json["nodeId"];
        Json tagName = json["tagName"];
        Handle<Expression> nodeIdExpr = nullptr;
        if (nodeId.is_string() && tagName.is_string()) {
            Handle<Expression> callExpr = nullptr;
            std::vector<Handle<Expression>> args;
            Handle<Expression> func = factory->NewIdentifier(json, "createElement");
            args.push_back(factory->NewStringLiteral(tagName, tagName.string_value()));
            if (controlExpr && controlExpr->IsForStatement()) {
                Json repeat = control["repeat"];
                Json index = repeat["index"];
                nodeIdExpr = factory->NewBinaryExpression(nodeId, BinaryOperation::kAnd, factory->NewStringLiteral(nodeId, nodeId.string_value() + "_REPEAT_"), factory->NewIdentifier(nodeId, index.string_value()));
                args.push_back(nodeIdExpr);
            }
            else {
                nodeIdExpr = factory->NewStringLiteral(nodeId, nodeId.string_value());
                args.push_back(nodeIdExpr);
            }
            callExpr = factory->NewCallExpression(json, func, args);
            if (controlExpr) {
                if (controlExpr->IsForStatement()) {
                    Handle<ForStatement> forExpr = controlExpr;
                    Handle<BlockStatement> forBlock = forExpr->body();
                    forBlock->PushExpression(callExpr);
                }
            }
            else {
                PushExpression(callExpr);
            }
        }
        Json childs = json["childNodes"];
        if (childs.is_array() && childs.array_items().size() > 0) {
            Handle<ExpressionList> list = factory->NewExpressionList();
            for (int i = 0;i < childs.array_items().size(); i++) {
                Json child = childs[i];
                if (child.is_null() || !child.is_object()) {
                    continue;
                }
                Handle<Expression> block = factory->NewChildBlockStatement(child, factory->NewExpressionList(), nodeId.string_value());
                list->Insert(block);
            }
            if (controlExpr) {
                if (controlExpr->IsForStatement()) {
                    Handle<ForStatement> forExpr = controlExpr;
                    Handle<BlockStatement> forBlock = forExpr->body();
                    forBlock->PushExpression(list);
                }
            }
            else {
                PushExpression(list);
            }
        }
        
    } while (0);
}

void ChunkStatement::PushExpression(Handle<Expression> expr)
{
    stmts_->Insert(expr);
}
    
void ChildBlockStatement::Pasing(Json &json) {
    do {
        ASTFactory *factory = ASTFactory::GetFactoryInstance();
        Json control = json["control"];
        Handle<Expression> controlExpr = nullptr;
        if (!control.is_null()) {
            controlExpr = factory->NewControlStatement(json);
            if (controlExpr) {
                PushExpression(controlExpr);
            }
        }
        Json nodeId = json["nodeId"];
        Json tagName = json["tagName"];
        Handle<Expression> nodeIdExpr = nullptr;
        if (nodeId.is_string() && tagName.is_string()) {
            Handle<Expression> callExpr = nullptr;
            Handle<Expression> callAppendExpr = nullptr;
            std::vector<Handle<Expression>> args,appendArgs;
            Handle<Expression> func = factory->NewIdentifier(json, "createElement");
            Handle<Expression> appendFunc = factory->NewIdentifier(json, "appendChild");
            args.push_back(factory->NewStringLiteral(tagName, tagName.string_value()));
            appendArgs.push_back(factory->NewStringLiteral(json, parent_));
            if (controlExpr && controlExpr->IsForStatement()) {
                Json repeat = control["repeat"];
                Json index = repeat["index"];
                nodeIdExpr = factory->NewBinaryExpression(nodeId, BinaryOperation::kAnd, factory->NewStringLiteral(nodeId, nodeId.string_value() + "_REPEAT_"), factory->NewIdentifier(nodeId, index.string_value()));
                args.push_back(nodeIdExpr);
                appendArgs.push_back(nodeIdExpr);
            }
            else {
                nodeIdExpr = factory->NewStringLiteral(nodeId, nodeId.string_value());
                args.push_back(nodeIdExpr);
                appendArgs.push_back(nodeIdExpr);
            }
            callExpr = factory->NewCallExpression(json, func, args);
            callAppendExpr = factory->NewCallExpression(json, appendFunc, appendArgs);
            if (controlExpr) {
                if (controlExpr->IsForStatement()) {
                    Handle<ForStatement> forExpr = controlExpr;
                    Handle<BlockStatement> forBlock = forExpr->body();
                    forBlock->PushExpression(callExpr);
                    forBlock->PushExpression(callAppendExpr);
                }
            }
            else {
                PushExpression(callExpr);
                PushExpression(callAppendExpr);
            }
        }
        Json attributes = json["attributes"];
        if (attributes.is_object()) {
            std::string error;
            Handle<Expression> attr = Parser::parseExpression(attributes, error);
            if (controlExpr) {
                
            }
            else {
            }
        }
        Json childs = json["childNodes"];
        if (childs.is_array() && childs.array_items().size() > 0) {
            Handle<ExpressionList> list = factory->NewExpressionList();
            for (int i = 0;i < json.array_items().size(); i++) {
                Json child = childs[i];
                if (child.is_null() || !child.is_object()) {
                    continue;
                }
                Handle<Expression> block = factory->NewChildBlockStatement(child, factory->NewExpressionList(), nodeId.string_value());
                list->Insert(block);
            }
            if (controlExpr) {
                if (controlExpr->IsForStatement()) {
                    Handle<ForStatement> forExpr = controlExpr;
                    Handle<BlockStatement> forBlock = forExpr->body();
                    forBlock->PushExpression(list);
                }
            }
            else {
                PushExpression(list);
            }
        }
        
    } while (0);
}
    
void ChildBlockStatement::PushExpression(Handle<Expression> expr)
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
