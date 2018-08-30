/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "core/data_render/code_generator.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/string_table.h"
#include "core/data_render/common_error.h"

namespace weex {
namespace core {
namespace data_render {
    
void CodeGenerator::EnterClass(Value *class_value) {
    ClassCnt *parent = class_;
    class_ = new ClassCnt;
    class_->set_parent(parent);
    class_->set_class_value(class_value);
}
    
void CodeGenerator::LeaveClass() {
    ClassCnt *parent = class_->parent();
    if (class_) {
        delete class_;
    }
    class_ = parent;
    if (class_) {
        class_->set_children(nullptr);
    }
}

void CodeGenerator::EnterFunction() {
    FuncCnt *parent_func = func_;
    func_ = new FuncCnt;
    func_->set_parent(parent_func);
    func_->set_func_state(new FuncState);
    if (parent_func != nullptr) {
        parent_func->func_state()->AddChild(func_->func_state());
    }
    BlockCnt *block = new BlockCnt();
    block->set_func_state(func_->func_state());
    block->set_exec_state(exec_state_);
    func_->set_root_block(block);
    func_->set_current_block(block);
    BlockCnt *parent_block = block_;
    block_ = block;
    block_->set_parent(parent_block);
    if (parent_block) {
        parent_block->set_children(block_);
    }
}

void CodeGenerator::LeaveFunction() {
    FuncCnt *parent_func = func_->parent();
    if (func_) {
        delete func_;
    }
    func_ = parent_func;
    if (func_) {
        func_->set_children(nullptr);
    }
    BlockCnt *parent_block = block_->parent();
    if (block_) {
        delete block_;
    }
    block_ = parent_block;
    if (block_) {
        block_->set_children(nullptr);
    }
}

void CodeGenerator::EnterBlock() {
    BlockCnt *parent_block = block_;
    block_ = new BlockCnt;
    block_->set_parent(parent_block);
    block_->set_func_state(func_->func_state());
    block_->set_exec_state(exec_state_);
    if (parent_block != nullptr) {
        parent_block->set_children(block_);
        block_->set_idx(parent_block->idx());
    }
    func_->set_current_block(block_);
}

void CodeGenerator::LeaveBlock() {
    BlockCnt *parent = block_->parent();
    if (block_) {
        delete block_;
    }
    block_ = parent;
    if (block_) {
        block_->set_children(nullptr);
    }
    func_->set_current_block(block_);
}

void CodeGenerator::Visit(StringConstant* node, void* data) {
  long reg = data == nullptr ? -1 : *static_cast<long*>(data);
  if (reg >= 0) {
    FuncState *func_state = func_->func_state();
    auto value = exec_state_->string_table_->StringFromUTF8(node->string());
    int index = func_state->AddConstant(std::move(value));
    Instruction i = CREATE_ABx(OpCode::OP_LOADK, reg, index);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(ExpressionList* node, void* data) {
  for (auto it = node->raw_list().begin(); it != node->raw_list().end(); ++it) {
    auto temp = (*it).get();
    temp->Accept(this, nullptr);
  }
}

void CodeGenerator::Visit(ChunkStatement* stms, void* data) {
  FuncScope scope(this);
  block_->NextRegisterId();
  // For root func
  for (int i = 0; i < stms->statements()->raw_list().size(); ++i) {
    auto temp = stms->statements()->raw_list()[i].get();
    if (temp != NULL) {
      temp->Accept(this, data);
    }
  }
  exec_state_->func_state_.reset(func_->func_state());
  // Save global variables
  exec_state_->global_variables_ = block_->variables();
}

void CodeGenerator::Visit(CallExpression *stms, void *data) {
    FuncState *func_state = func_->func_state();
    long ret = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
    long caller = -1;
    size_t argc = 0;
    if (stms->callee().get() != NULL) {
        caller = block_->NextRegisterId();
        stms->callee()->Accept(this, &caller);
        argc = stms->args().size();
        if (block_->idx() > caller + 1) {
            long reg_old_caller = caller;
            caller = block_->NextRegisterId();
            func_state->AddInstruction(CREATE_ABC(OP_MOVE, caller, reg_old_caller, 0));
        }
    }
    else if (stms->expr().get()) {
        if (class_ && stms->expr()->IsIdentifier() && stms->expr()->AsIdentifier()->GetName() == "super")
        {
            ClassDescriptor *class_desc = ObjectValue<ClassDescriptor>(class_->class_value());
            if (!class_desc->p_super_) {
                throw GeneratorError("can't call super without class desc");
            }
            ClassDescriptor *class_desc_super = class_desc->p_super_;
            int index = class_desc_super->funcs_->IndexOf("constructor");
            if (index < 0) {
                throw GeneratorError("can't call super without class desc");
            }
            long reg_this = block_->FindRegisterId("this");
            if (reg_this < 0) {
                throw GeneratorError("can't call super this without class desc");
            }
            caller = block_->NextRegisterId();
            long arg_super = block_->NextRegisterId();
            func_state->AddInstruction(CREATE_ABC(OP_GETSUPER, arg_super, reg_this, caller));
            argc++;
        }
        else {
            if (stms->expr()->IsIdentifier() && stms->member() && stms->member()->IsIdentifier()) {
                long reg_member = block_->NextRegisterId();
                auto value = exec_state_->string_table_->StringFromUTF8(stms->member()->AsIdentifier()->GetName());
                int tableIndex = func_state->AddConstant(std::move(value));
                func_state->AddInstruction(CREATE_ABx(OP_LOADK, reg_member, tableIndex));
                long reg_class = block_->FindRegisterId(stms->expr()->AsIdentifier()->GetName());
                if (reg_class < 0) {
                    throw GeneratorError("can't find " + stms->expr()->AsIdentifier()->GetName());
                }
                // caller and args must be continuous;
                caller = block_->NextRegisterId();
                func_state->AddInstruction(CREATE_ABC(OP_GETCLASS, caller, reg_class, reg_member));
                long arg = block_->NextRegisterId();
                stms->expr()->Accept(this, &arg);
                argc = stms->args().size() + 1;
            }
            else {
                caller = block_->NextRegisterId();
                stms->expr()->Accept(this, &caller);
                argc = stms->args().size();
                if (block_->idx() > caller + 1) {
                    long reg_old_caller = caller;
                    caller = block_->NextRegisterId();
                    func_state->AddInstruction(CREATE_ABC(OP_MOVE, caller, reg_old_caller, 0));
                }
                if (stms->expr()->IsMemberExpression()) {
                    Handle<Expression> left = stms->expr()->AsMemberExpression()->expr();
                    if (left->IsIdentifier()) {
                        std::string class_name = left->AsIdentifier()->GetName();
                        int index = exec_state_->global()->IndexOf(class_name);
                        if (index <= 0) {
                            long arg = block_->NextRegisterId();
                            stms->expr()->AsMemberExpression()->expr()->AsIdentifier()->Accept(this, &arg);
                            argc++;
                        }
                    }
                    else {
                        long arg = block_->NextRegisterId();
                        left->Accept(this, &arg);
                        argc++;
                    }
                }
            }
        }
    }
    for (auto it = stms->args().begin(); it != stms->args().end(); ++it) {
        auto temp = (*it).get();
        long arg = block_->NextRegisterId();
        temp->Accept(this, &arg);
    }
    if (stms->args_expr() != nullptr && stms->args_expr()->IsArgumentList()) {
        Handle<ArgumentList> arg_list = stms->args_expr()->AsArgumentList();
        for (int i = 0; i < arg_list->length(); i++) {
            long arg = block_->NextRegisterId();
            arg_list->args()->raw_list()[i]->Accept(this, &arg);
        }
        argc += arg_list->length();
    }
    FuncState *state = func_->func_state();
    state->AddInstruction(CREATE_ABC(OP_CALL, ret, argc, caller));
}

void CodeGenerator::Visit(ArgumentList* node, void* data) {
  Handle<ExpressionList> exprList = node->args();
  exprList->Accept(this, data);
}

void CodeGenerator::Visit(IfStatement *node, void *data) {
    RegisterScope scope(block_);
    long condition = block_->NextRegisterId();
    FuncState *func_state = func_->func_state();
    if (node->condition().get() != NULL) {
        node->condition()->Accept(this, &condition);
    }
    auto slot = func_state->AddInstruction(0);
    int tb_start_index = (int)func_state->instructions().size() - 1;
    if (node->body().get() != NULL) {
        node->body()->Accept(this, nullptr);
    }
    int tb_end_index = (int)func_state->instructions().size() - 1;
    func_state->ReplaceInstruction(slot, CREATE_ABx(OP_JMP, condition, tb_end_index - tb_start_index));
}
    
void CodeGenerator::Visit(TernaryExpression *node, void *data) {
    long ret = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
    FuncState *func_state = func_->func_state();
    long first = block_->NextRegisterId();
    if (node->first().get() != NULL) {
        node->first()->Accept(this, &first);
    }
    auto slot = func_state->AddInstruction(0);
    int second_index = (int)func_state->instructions().size() - 1;
    if (node->second().get() != NULL) {
        node->second()->Accept(this, &ret);
    }
    auto goto_slot = func_state->AddInstruction(0);
    int third_index = (int)func_state->instructions().size() - 1;
    if (node->third().get() != NULL) {
        node->third()->Accept(this, &ret);
    }
    int end_index = (int)func_state->instructions().size();
    func_state->ReplaceInstruction(slot, CREATE_ABx(OP_JMP, first, third_index - second_index));
    func_state->ReplaceInstruction(goto_slot, CREATE_Ax(OP_GOTO, end_index));
}

void CodeGenerator::Visit(IfElseStatement *node, void *data) {
    long ifcondition = block_->NextRegisterId();
    FuncState *func_state = func_->func_state();
    if (node->condition().get() != NULL) {
        node->condition()->Accept(this, &ifcondition);
    }
    auto slot = func_state->AddInstruction(0);
    int tb_start_index = (int)func_state->instructions().size() - 1;
    if (node->body().get() != NULL) {
        node->body()->Accept(this, nullptr);
    }
    auto else_slot = func_state->AddInstruction(0);
    int tb_end_index = (int)func_state->instructions().size() - 1;
    if (node->els().get() != NULL) {
        node->els()->Accept(this, nullptr);
    }
    func_state->ReplaceInstruction(slot, CREATE_ABx(OP_JMP, ifcondition, tb_end_index - tb_start_index));
    int end_index = (int)func_state->instructions().size();
    func_state->ReplaceInstruction(else_slot, CREATE_Ax(OP_GOTO, end_index));
}

void CodeGenerator::Visit(ForStatement *node, void *data) {
  BlockScope for_scope(this);  // for var index = 0;

  if (node->init().get() != NULL) {
    node->init()->Accept(this, nullptr);
  }

  int condition_start_index = (int)func_->func_state()->instructions().size();  // aka next one.

  long condition = block_->NextRegisterId();
  if (node->condition().get() != NULL) {
    node->condition()->Accept(this, &condition);
  }

  auto slot = func_->func_state()->AddInstruction(0);

  if (node->body().get() != NULL) {
    node->body()->Accept(this, nullptr);
  }

  long update = block_->NextRegisterId();
  if (node->update().get() != NULL) {
    node->update()->Accept(this, &update);
  }

  FuncState* state = func_->func_state();
  state->AddInstruction(CREATE_Ax(OP_GOTO, condition_start_index));

  int for_end_index = (int)func_->func_state()->instructions().size() - 1;

  state->ReplaceInstruction(
      slot, (CREATE_ABx(OP_JMP, condition, for_end_index - slot)));
}

void CodeGenerator::Visit(BlockStatement* node, void* data) {
  BlockScope block_scoped(this);
  for (int i = 0; i < node->statements()->raw_list().size(); ++i) {
    auto temp = node->statements()->raw_list()[i].get();
    if (temp != NULL) {
      temp->Accept(this, data);
    }
  }
}

void CodeGenerator::Visit(FunctionPrototype *node, void *data) {}

void CodeGenerator::Visit(FunctionStatement *node, void *data) {
    RegisterScope register_scope(block_);
    long ret = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
    Handle<FunctionPrototype> proto = node->proto();
    bool is_class_func = func_->parent() == nullptr && class_ ? true : false;
    // body
    // Slot
    auto slot = func_->func_state()->AddInstruction(0);
    {
        FuncScope scope(this);
        if (is_class_func) {
            Value value;
            value.f = func_->func_state();
            value.type = Value::FUNC;
            ClassDescriptor *class_desc = ObjectValue<ClassDescriptor>(class_->class_value());
            class_desc->funcs_->Add(proto->GetName(), value);
        }
        // skip func value in the fornt of stack;
        block_->NextRegisterId();

        // make arguments var in thie front of stack;
        if (is_class_func) {
            block_->variables().insert(std::make_pair("this", block_->NextRegisterId()));
        }
        for (int i = 0; i < proto->GetArgs().size(); i++) {
            std::string arg = proto->GetArgs().at(i);
            block_->variables().insert(std::make_pair(arg, block_->NextRegisterId()));
        }
        node->body()->Accept(this, nullptr);
    }
    
    // function prototype
    // associate function_name and function_state
    if (func_->parent() == nullptr) {
        // in chunk
        if (!is_class_func) {
            Value value;
            value.f = func_->func_state()->GetChild(func_->func_state()->children().size() - 1);
            value.type = Value::FUNC;
            if (proto->GetName().length() > 0) {
                exec_state_->global()->Add(proto->GetName(), value);
            }
            else {
                int index = exec_state_->global()->Add(value);
                if (index >= 0) {
                    func_->func_state()->AddInstruction(CREATE_ABx(OP_GETGLOBAL, ret, index));
                }
            }
        }
    }
    else {
        // inside function
        int index = (int)func_->func_state()->children().size() - 1;
        Instruction i = CREATE_ABx(OP_GETFUNC, ret, index);
        func_->func_state()->ReplaceInstruction(slot, i);
        if (proto->GetName().length() > 0) {
            block_->variables().insert(std::make_pair(proto->GetName(), ret));
        }
    }
}
    
void CodeGenerator::Visit(ThisExpression *node, void *data) {    
    long ret = data == nullptr ? -1 : *static_cast<long *>(data);
    if (ret >= 0) {
        // searching in global
        long rhs = block_->FindRegisterId("this");
        if (rhs != ret) {
            // a = b
            func_->func_state()->AddInstruction(CREATE_ABC(OP_MOVE, ret, rhs, 0));
        }
    }
}
    
void CodeGenerator::Visit(ArrowFunctionStatement *node, void *data) {
    RegisterScope register_scope(block_);
    long ret = data == nullptr ? block_->NextRegisterId()
    : *static_cast<long *>(data);
    
    // body
    // Slot
    auto slot = func_->func_state()->AddInstruction(0);
    {
        FuncScope scope(this);
        // skip func value in the fornt of stack;
        block_->NextRegisterId();

        // make arguments var in thie front of stack;
        for (int i = 0; i < node->args().size(); i++) {
            assert(node->args()[i]->IsIdentifier());
            std::string arg = node->args()[i]->AsIdentifier()->GetName();
            block_->variables().insert(std::make_pair(arg, block_->NextRegisterId()));
        }
        node->body()->Accept(this, nullptr);
    }
    // associate function_name and function_state
    if (func_->parent() == nullptr) {
        // in chunk
        Value value;
        assert(data);
        value.f = func_->func_state()->GetChild(func_->func_state()->children().size() - 1);
        value.type = Value::FUNC;
        int index = exec_state_->global()->Add(value);
        if (index >= 0) {
            func_->func_state()->AddInstruction(CREATE_ABx(OP_GETGLOBAL, ret, index));
        }
    }
    else {
        // inside function
        int index = (int)func_->func_state()->children().size() - 1;
        Instruction i = CREATE_ABx(OP_GETFUNC, ret, index);
        func_->func_state()->ReplaceInstruction(slot, i);
    }
}
    
void CodeGenerator::Visit(ClassStatement *node, void *data) {
    do {
        Handle<Expression> super_expr = node->Super();
        Value *super_value = nullptr,class_value = nullptr;
        if (super_expr) {
            if (!super_expr->IsIdentifier()) {
                throw GeneratorError("super isn't a Identifier");
                break;
            }
            int index = exec_state_->global()->IndexOf(super_expr->AsIdentifier()->GetName());
            if (index == -1) {
                throw GeneratorError("can't find super class" + super_expr->AsIdentifier()->GetName());
                break;
            }
            super_value = exec_state_->global()->Find(index);
            if (super_value->type != Value::CLASS_DESC) {
                throw GeneratorError("can't find super class" + super_expr->AsIdentifier()->GetName());
                break;
            }
        }
        std::string class_name = node->Identifier()->AsIdentifier()->GetName();
        int index = exec_state_->global()->IndexOf(class_name);
        if (index != -1) {
            throw GeneratorError(class_name + "redefine");
            break;
        }
        class_value = exec_state_->class_factory()->CreateClassDescriptor(super_value ? ObjectValue<ClassDescriptor>(super_value) : nullptr);
        exec_state_->global()->Add(class_name, class_value);
        ClassScope scope(this, &class_value);
        node->Body()->Accept(this, nullptr);
        
    } while (0);
}
    
void CodeGenerator::Visit(ClassBody *node, void *data) {
    for (int i = 0; i < node->raw_list().size(); i++) {
        node->raw_list()[i]->Accept(this, data);
    }
}
    
void CodeGenerator::Visit(NewExpression *node, void *data) {
    do {
        RegisterScope scope(block_);
        long lhs = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
        if (lhs >= 0) {
            FuncState *state = func_->func_state();
            long rhs = block_->FindRegisterId(node->member()->AsIdentifier()->GetName());
            if (rhs >= 0) {
                state->AddInstruction(CREATE_ABC(OP_MOVE, lhs, rhs, 0));
                break;
            }
            int index = exec_state_->global()->IndexOf(node->member()->AsIdentifier()->GetName());
            if (index >= 0) {
                Value *value = exec_state_->global()->Find(index);
                if (IsClass(value)) {
                    state->AddInstruction(CREATE_ABx(OP_NEWCLASS, lhs, index));
                }
                else {
                    state->AddInstruction(CREATE_ABx(OP_GETGLOBAL, lhs, index));
                }
            }
        }
            
    } while (0);

}
    
void CodeGenerator::Visit(JSXNodeExpression *node, void *data) {
    do {
        FuncState *func_state = func_->func_state();
        if (!node->LowerIdentifier()) {
            std::string name = node->Identifier()->AsIdentifier()->GetName();
            int index = exec_state_->global()->IndexOf(name);
            if (index < 0) {
                bool exist = block_->FindVariable(name);
                if (!exist) {
                    throw GeneratorError("can't find identifier:" + name);
                    break;
                }
            }
            else {
                node->SetClass(true);
            }
        }
        std::vector<Handle<Expression>> exprs = node->funcexprs();
        for (int i = 0; i < exprs.size(); i++) {
            if (i == 0) {
                exprs[i]->Accept(this, nullptr);
            }
            else {
                exprs[i]->Accept(this, nullptr);
            }
        }
        std::string vnode_ptr = node->node_ptr()->AsStringConstant()->string();
        long reg_parent = block_->FindRegisterId(vnode_ptr);
        if (reg_parent < 0) {
            throw GeneratorError("can't find identifier: " + node->node_ptr()->AsStringConstant()->string());
        }
        std::vector<Handle<Expression>> childrens = node->childrens();
        // 递归 childrens
        if (childrens.size() > 0) {
            for (int i = 0; i < childrens.size(); i++) {
                long ret = block_->NextRegisterId();
                long caller = block_->NextRegisterId();
                size_t argc = 2;
                long arg_0 = block_->NextRegisterId();
                long arg_1 = block_->NextRegisterId();
                int index = exec_state_->global()->IndexOf("appendChild");
                if (index < 0) {
                    throw GeneratorError("can't find identifier appendChild");
                }
                func_state->AddInstruction(CREATE_ABx(OP_GETGLOBAL, caller, index));
                func_state->AddInstruction(CREATE_ABC(OP_MOVE, arg_0, reg_parent, 0));
                if (childrens[i]->IsJSXNodeExpression()) {
                    childrens[i]->Accept(this, nullptr);
                    std::string vnode_child_ptr = childrens[i]->AsJSXNodeExpression()->node_ptr()->AsStringConstant()->string();
                    long reg_child = block_->FindRegisterId(vnode_child_ptr);
                    if (reg_child < 0) {
                        throw GeneratorError("can't find identifier: " + childrens[i]->AsJSXNodeExpression()->node_ptr()->AsStringConstant()->string());
                    }
                    func_state->AddInstruction(CREATE_ABC(OP_MOVE, arg_1, reg_child, 0));
                }
                else {
                    childrens[i]->Accept(this, &arg_1);
                }
                func_state->AddInstruction(CREATE_ABC(OP_CALL, ret, argc, caller));
            }
        }
        if (data) {
            long ret = *static_cast<long *>(data);
            func_state->AddInstruction(CREATE_ABC(OP_MOVE, ret, reg_parent, 0));
        }
        
    } while (0);
}

void CodeGenerator::Visit(BinaryExpression* node, void* data) {
    RegisterScope scope(block_);
    long ret = data == nullptr ? block_->NextRegisterId() : *static_cast<long*>(data);
    long left = block_->NextRegisterId();
    if (node->lhs().get() != NULL) {
        node->lhs()->Accept(this, &left);
    }
    long right = block_->NextRegisterId();
    if (node->rhs().get() != NULL) {
        node->rhs()->Accept(this, &right);
    }
    BinaryOperation opeartion = node->op();
    FuncState *state = func_->func_state();
    switch (opeartion) {
        case BinaryOperation::kAddition:
        {
            // a + b
            state->AddInstruction(CREATE_ABC(OP_ADD, ret, left, right));
            break;
        }
        case BinaryOperation::kLessThan:
        {
            state->AddInstruction(CREATE_ABC(OP_LT, ret, left, right));
            break;
        }
        case BinaryOperation::kGreaterThan:
        {
            state->AddInstruction(CREATE_ABC(OP_GT, ret, right, left));
            break;
        }
        case BinaryOperation::kLessThanEqual:
        {
            state->AddInstruction(CREATE_ABC(OP_LTE, ret, left, right));
            break;
        }
        case BinaryOperation::kGreaterThanEqual:
        {
            state->AddInstruction(CREATE_ABC(OP_GTE, ret, right, left));
            break;
        }
        case BinaryOperation::kAnd:
        {
            state->AddInstruction(CREATE_ABC(OP_AND, ret, right, left));
            break;
        }
        case BinaryOperation::kStrictEqual:
        {
            state->AddInstruction(CREATE_ABC(OP_SEQ, ret, right, left));
            break;
        }
        default:
        {
            throw GeneratorError("unsupport binary operation");
            break;
        }
    }
}

void CodeGenerator::Visit(AssignExpression *node, void *data) {
    RegisterScope scope(block_);
    // needs to be left value, for now only identifer is supported.
    // this = a;
    // this.b = a; TBD
    long left = -1;
    if (node->lhs().get() != NULL) {
        if (node->lhs()->IsIdentifier()) {
            auto& name = node->lhs()->AsIdentifier()->GetName();
            left = block_->FindRegisterId(name);
        }
        else {
            left = block_->NextRegisterId();
            node->lhs()->Accept(this, &left);
        }
    }
    long right = block_->NextRegisterId();
    if (node->rhs().get() != NULL) {
        node->rhs()->Accept(this, &right);
    }
    // a = b
    FuncState *func_state = func_->func_state();
    if (class_ && node->lhs()->IsMemberExpression() && node->lhs()->AsMemberExpression()->expr()->IsThisExpression()) {
        func_state->AddInstruction(CREATE_ABC(OP_SETMEMBER, left, right, 0));
    }
    else {
        func_state->AddInstruction(CREATE_ABC(OP_MOVE, left, right, 0));
    }
}

void CodeGenerator::Visit(Declaration *node, void *data) {
  long reg = block_->NextRegisterId();
  FuncState *func_state = func_->func_state();
  block_->variables().insert(std::make_pair(node->name(), reg));
  if (node->expr().get() != nullptr) {
      node->expr()->Accept(this, &reg);
  }
  else {
      func_state->AddInstruction(CREATE_ABC(OP_LOADNULL, reg, 0, 0));
  }
  if (data) {
      long ret = *static_cast<long *>(data);
      func_state->AddInstruction(CREATE_ABC(OP_MOVE, ret, reg, 0));
  }
}

void CodeGenerator::Visit(DeclarationList *node, void *data) {
  for (int i = 0; i < node->exprs().size(); ++i) {
    auto temp = node->exprs()[i].get();
    if (temp != NULL) {
      temp->Accept(this, data);
    }
  }
}

void CodeGenerator::Visit(CommaExpression* node, void* data) {
  bool first = true;
  for (auto it = node->exprs()->begin(); it != node->exprs()->end(); ++it) {
    auto temp = (*it).get();
    temp->Accept(this, first ? data : nullptr);
    first = false;
  }
}

void CodeGenerator::Visit(IntegralConstant* node, void* data) {
  long reg = data == nullptr ? -1 : *static_cast<long*>(data);
  if (reg >= 0) {
    FuncState* func_state = func_->func_state();
    int value = node->value();
    int index = func_state->AddConstant(static_cast<int64_t>(value));
    Instruction i = CREATE_ABx(OpCode::OP_LOADK, reg, index);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(BooleanConstant* node, void* data) {
  long reg = data == nullptr ? -1 : *static_cast<long*>(data);
  if (reg >= 0) {
    FuncState* func_state = func_->func_state();
    bool value = node->pred();
    int index = func_state->AddConstant(static_cast<bool>(value));
    Instruction i = CREATE_ABx(OpCode::OP_LOADK, reg, index);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(DoubleConstant* node, void* data) {
  long reg = data == nullptr ? -1 : *static_cast<long*>(data);
  if (reg >= 0) {
    FuncState* func_state = func_->func_state();
    double value = node->value();
    int index = func_state->AddConstant(static_cast<double>(value));
    Instruction i = CREATE_ABx(OpCode::OP_LOADK, reg, index);
    func_state->AddInstruction(i);
  }
}
    
void CodeGenerator::Visit(UndefinedConstant *node, void *data) {
    long reg = data == nullptr ? -1 : *static_cast<long*>(data);
    if (reg >= 0) {
        FuncState *func_state = func_->func_state();
        int index = func_state->AddConstant(Value());
        Instruction i = CREATE_ABx(OpCode::OP_LOADK, reg, index);
        func_state->AddInstruction(i);
    }
}

void CodeGenerator::Visit(ObjectConstant* node, void* data) {
  long ret = data == nullptr ? -1 : *static_cast<long*>(data);

  FuncState* func_state = func_->func_state();

  // new table
  Value table = exec_state_->table_factory()->CreateTable();
  if (ret >= 0) {
    int tableIndex = func_state->AddConstant(table);
    Instruction i = CREATE_ABx(OpCode::OP_LOADK, ret, tableIndex);
    func_state->AddInstruction(i);

    // expr
    for (auto it = node->proxy().begin(); it != node->proxy().end(); it++) {
      RegisterScope scope(block_);
      long item = block_->NextRegisterId();
      long key = block_->NextRegisterId();

      auto ktemp = (*it).second;
      ktemp->Accept(this, &item);

      auto value = exec_state_->string_table_->StringFromUTF8(it->first);
      int keyIndex = func_state->AddConstant(std::move(value));
      func_state->AddInstruction(
          CREATE_ABx(OpCode::OP_LOADK, key, keyIndex));

      Instruction i = CREATE_ABC(OpCode::OP_SETTABLE, ret, key, item);
      func_state->AddInstruction(i);
    }
  }
}
// TODO: this is not correct.
void CodeGenerator::Visit(ArrayConstant* node, void* data) {
  long reg = data == nullptr ? -1 : *static_cast<long*>(data);

  FuncState *func_state = func_->func_state();

  // new table
  Value array = exec_state_->class_factory()->CreateArray();
  if (reg >= 0) {
    int tableIndex = func_state->AddConstant(array);
    Instruction i = CREATE_ABx(OpCode::OP_LOADK, reg, tableIndex);
    func_state->AddInstruction(i);

    // expr
    for (auto it = node->exprs().begin(); it != node->exprs().end(); it++) {
      long item = block_->NextRegisterId();

      auto temp = (*it).get();
      temp->Accept(this, &item);

      Instruction i = CREATE_ABC(OpCode::OP_SETTABLE, reg, 0, item);
      func_state->AddInstruction(i);
    }
  }
}

void CodeGenerator::Visit(MemberExpression *node, void *data) {
    RegisterScope registerScope(block_);
    long ret = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
    FuncState *func_state = func_->func_state();
    if (node->kind() == MemberAccessKind::kIndex) {
        Handle<Expression> left = node->expr();
        left->Accept(this, &ret);
        long mindex = block_->NextRegisterId();
        if (node->member().get() != NULL) {
            node->member()->Accept(this, &mindex);
        }
        func_state->AddInstruction(CREATE_ABC(OP_GETARRAY, ret, ret, mindex));
    }
    else if (node->kind() == MemberAccessKind::kDot || node->kind() == MemberAccessKind::kCall) {
        Handle<Expression> left = node->expr();
        left->Accept(this, &ret);
        long right = block_->NextRegisterId();
        auto value = exec_state_->string_table_->StringFromUTF8(node->member()->AsIdentifier()->GetName());
        int tableIndex = func_state->AddConstant(std::move(value));
        func_state->AddInstruction(CREATE_ABx(OP_LOADK, right, tableIndex));
        func_state->AddInstruction(CREATE_ABC(OP_GETMEMBER, ret, ret, right));
    }
    else if (node->kind() == MemberAccessKind::kClass) {
        Handle<Expression> left = node->expr();
        left->Accept(this, &ret);
        long right = block_->NextRegisterId();
        auto value = exec_state_->string_table_->StringFromUTF8(node->member()->AsIdentifier()->GetName());
        int tableIndex = func_state->AddConstant(std::move(value));
        func_state->AddInstruction(CREATE_ABx(OP_LOADK, right, tableIndex));
        func_state->AddInstruction(CREATE_ABC(OP_GETMEMBER, ret, ret, right));
    }
}

void CodeGenerator::Visit(Identifier *node, void *data) {
  long reg_a = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
  if (reg_a >= 0) {
    FuncState *state = func_->func_state();
    long reg_b = block_->FindRegisterId(node->GetName());
    if (reg_b >= 0) {
      state->AddInstruction(CREATE_ABC(OP_MOVE, reg_a, reg_b, 0));
      return;
    }

    int index = exec_state_->global()->IndexOf(node->GetName());
    if (index >= 0) {
      state->AddInstruction(CREATE_ABx(OP_GETGLOBAL, reg_a, index));
      return;
    }

    // TODO Fix me: Identifer searching in this scope should be ahead of
    // searching in global
    long this_idx = block_->FindRegisterId("this");
    if (this_idx >= 0) {
      // using this
      long right = block_->NextRegisterId();

      FuncState* func_state = func_->func_state();
      auto value = exec_state_->string_table_->StringFromUTF8(node->GetName());
      int tableIndex = func_state->AddConstant(std::move(value));

      func_state->AddInstruction(
          CREATE_ABx(OpCode::OP_LOADK, right, tableIndex));
      func_state->AddInstruction(
          CREATE_ABC(OP_GETMEMBER, reg_a, this_idx, right));
    } else {
      // make data undefined.
      state->AddInstruction(CREATE_ABC(OP_LOADNULL, reg_a, 0, 0));
    }
  }
}

void CodeGenerator::Visit(PrefixExpression *node, void *data) {
    RegisterScope scope(block_);
    long ret = data == nullptr ? block_->NextRegisterId()
                             : *static_cast<long *>(data);
    Handle<Expression> expr = node->expr();
    long reg = -1;
    if (expr->IsIdentifier()) {
        reg = block_->FindRegisterId(expr->AsIdentifier()->GetName());
    }
    else {
        reg = block_->NextRegisterId();
        expr->Accept(this, &reg);
    }
    PrefixOperation operation = node->op();
    // ++i
    if (operation == PrefixOperation::kIncrement) {
        func_->func_state()->AddInstruction(CREATE_ABC(OP_PRE_INCR, reg, ret, 0));
    }
    // --i
    else if (operation == PrefixOperation::kDecrement) {
        func_->func_state()->AddInstruction(CREATE_ABC(OP_PRE_DECR, reg, ret, 0));
    }
    else if (operation == PrefixOperation::kUnfold) {
        func_->func_state()->AddInstruction(CREATE_ABC(OP_MOVE, ret, reg, 0));
    }
}
    
void CodeGenerator::Visit(PostfixExpression *node, void *data) {
    RegisterScope scope(block_);
    long ret = data == nullptr ? block_->NextRegisterId()
    : *static_cast<long *>(data);
    Handle<Expression> expr = node->expr();
    long reg = -1;
    if (expr->IsIdentifier()) {
        reg = block_->FindRegisterId(expr->AsIdentifier()->GetName());
    }
    else {
        reg = block_->NextRegisterId();
        expr->Accept(this, &reg);
    }
    PostfixOperation operation = node->op();
    // i++
    if (operation == PostfixOperation::kIncrement) {
        func_->func_state()->AddInstruction(CREATE_ABC(OP_POST_INCR, reg, ret, 0));
    }
    // i++
    else if (operation == PostfixOperation::kDecrement) {
        func_->func_state()->AddInstruction(CREATE_ABC(OP_POST_DECR, reg, ret, 0));
    }
}

void CodeGenerator::Visit(ReturnStatement* node, void* data) {
    
  if (node->expr() == nullptr) {
    func_->func_state()->AddInstruction(CREATE_ABC(OP_RETURN0, 0, 0, 0));
  }
  else {
    long ret = block_->NextRegisterId();
    node->expr()->Accept(this, &ret);
    func_->func_state()->AddInstruction(CREATE_ABC(OP_RETURN1, ret, 0, 0));
  }
}
    
bool CodeGenerator::BlockCnt::FindVariable(const std::string &name) {
    auto iter = variables_.find(name);
    if (iter != variables_.end()) {
        return true;
    }
    if (parent() != nullptr) {
        return parent()->FindRegisterId(name) >= 0 ? true : false;
    }
    return false;
}
    
long CodeGenerator::BlockCnt::FindRegisterId(const std::string &name) {
    auto iter = variables_.find(name);
    if (iter != variables_.end()) {
        return iter->second;
    }
    if (parent() != nullptr) {
        if (parent()->func_state() == func_state_) {
            long reg_ref = parent()->FindRegisterId(name);
            if (reg_ref >= 0) {
                variables_.insert(std::make_pair(name, reg_ref));
            }
            return reg_ref;
        }
        else {
            long reg_ref = -1;
            ValueRef *ref = parent()->FindValueRef(name, reg_ref);
            if (ref) {
                BlockCnt *root_block = this;
                while (root_block->children() && root_block->children()->func_state() == func_state_)
                {
                    root_block = root_block->children();
                }
                reg_ref = root_block->NextRegisterId();
                func_state_->AddInstruction(CREATE_ABx(OP_GETOUTVAR, reg_ref, ref->ref_id()));
            }
            return reg_ref;
        }
    }
    return -1;
}
    
ValueRef *CodeGenerator::BlockCnt::FindValueRef(const std::string &name, long &reg_ref) {
    auto iter = variables_.find(name);
    if (iter != variables_.end()) {
        if (exec_state_ && func_state_) {
            ValueRef *ref = exec_state_->AddRef(func_state_, iter->second);
            Instruction instruction = func_state_->instructions().size() > 0 ? func_state_->instructions()[func_state_->instructions().size() - 1] : 0;
            OpCode op(GET_OP_CODE(instruction));
            if (instruction && (op == OP_RETURN0 || op == OP_RETURN1)) {
                func_state_->instructions().pop_back();
            }
            func_state_->AddInstruction(CREATE_ABx(OP_SETOUTVAR, iter->second, ref->ref_id()));
            if (instruction && (op == OP_RETURN0 || op == OP_RETURN1)) {
                func_state_->AddInstruction(instruction);
            }
            return ref;
        }
        else {
            reg_ref = iter->second;
            return nullptr;
        }
    }
    if (parent() != nullptr) {
        return parent()->FindValueRef(name, reg_ref);
    }
    return nullptr;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
