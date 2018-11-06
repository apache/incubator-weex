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
#include "core/data_render/class.h"

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
        block_->reset();
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
        block_->reset();
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
      Instruction i = CREATE_ABx(OP_LOADK, reg, index);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(RegexConstant* node, void* data) {
  RegisterScope scope(block_);

  long reg = data == nullptr ? -1 : *static_cast<long*>(data);
  if (reg >= 0) {
    FuncState *func_state = func_->func_state();
    auto reg_ex = exec_state_->string_table_->StringFromUTF8(node->reg());
    auto flag = exec_state_->string_table_->StringFromUTF8(node->flag());
    auto reg_mem_name = exec_state_->string_table_->StringFromUTF8("_reg");
    auto flag_mem_name = exec_state_->string_table_->StringFromUTF8("_flag");

    int r_index = func_state->AddConstant(std::move(reg_ex));
    int f_index = func_state->AddConstant(std::move(flag));
    int mr_index = func_state->AddConstant(std::move(reg_mem_name));
    int mf_index = func_state->AddConstant(std::move(flag_mem_name));
    auto reg_class_index = exec_state_->global()->IndexOf("RegExp");

    auto mr_id = block_->NextRegisterId();
    auto mf_id = block_->NextRegisterId();
    auto r_id = block_->NextRegisterId();
    auto f_id = block_->NextRegisterId();
    auto class_id = block_->NextRegisterId();
    auto tmp = block_->NextRegisterId();

    func_state->AddInstruction(CREATE_ABx(OP_LOADK, mr_id, mr_index));
    func_state->AddInstruction(CREATE_ABx(OP_LOADK, mf_id, mf_index));
    func_state->AddInstruction(CREATE_ABx(OP_LOADK, r_id, r_index));
    func_state->AddInstruction(CREATE_ABx(OP_LOADK, f_id, f_index));
    func_state->AddInstruction(CREATE_ABx(OP_GETGLOBAL, class_id, reg_class_index));
      
    func_state->AddInstruction(CREATE_ABC(OP_NEW, reg, Value::Type::CLASS_DESC, class_id));

    func_state->AddInstruction(CREATE_ABC(OP_GETMEMBERVAR, tmp, reg, mr_id));
    func_state->AddInstruction(CREATE_ABx(OP_MOVE, tmp, r_id));

    func_state->AddInstruction(CREATE_ABC(OP_GETMEMBERVAR, tmp, reg, mf_id));
    func_state->AddInstruction(CREATE_ABx(OP_MOVE, tmp, f_id));
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
    std::vector<long> caller_regs_order;
    if (stms->callee().get() != NULL) {
        caller = block_->NextRegisterId();
        stms->callee()->Accept(this, &caller);
        argc = stms->args().size();
        if (block_->idx() > caller + 1) {
            long reg_old_caller = caller;
            caller = block_->NextRegisterId();
            func_state->AddInstruction(CREATE_ABx(OP_MOVE, caller, reg_old_caller));
        }
        caller_regs_order.push_back(caller);
    }
    else if (stms->expr().get()) {
        if (class_ && stms->expr()->IsIdentifier() && stms->expr()->AsIdentifier()->GetName() == JS_GLOBAL_SUPER)
        {
            ClassDescriptor *class_desc = ValueTo<ClassDescriptor>(class_->class_value());
            if (!class_desc->p_super_) {
                throw GeneratorError("can't call super without class desc");
            }
            ClassDescriptor *class_desc_super = class_desc->p_super_;
            int index = class_desc_super->funcs_->IndexOf(JS_GLOBAL_CONSTRUCTOR);
            if (index < 0) {
                throw GeneratorError("can't call super without class desc");
            }
            long reg_this = block_->FindRegisterId("this");
            if (reg_this < 0) {
                throw GeneratorError("can't call super this without class desc");
            }
            caller = block_->NextRegisterId();
            caller_regs_order.push_back(caller);
            long arg_super = block_->NextRegisterId();
            caller_regs_order.push_back(arg_super);
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
                caller_regs_order.push_back(caller);
                func_state->AddInstruction(CREATE_ABC(OP_GETCLASS, caller, reg_class, reg_member));
                long arg = block_->NextRegisterId();
                caller_regs_order.push_back(arg);
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
                    func_state->AddInstruction(CREATE_ABx(OP_MOVE, caller, reg_old_caller));
                }
                caller_regs_order.push_back(caller);
                if (stms->expr()->IsMemberExpression()) {
                    Handle<Expression> left = stms->expr()->AsMemberExpression()->expr();
                    if (left->IsIdentifier()) {
                        std::string class_name = left->AsIdentifier()->GetName();
                        int index = exec_state_->global()->IndexOf(class_name);
                        if (index <= 0) {
                            long arg = block_->NextRegisterId();
                            caller_regs_order.push_back(arg);
                            stms->expr()->AsMemberExpression()->expr()->AsIdentifier()->Accept(this, &arg);
                            argc++;
                        }
                    }
                    else {
                        long arg = block_->NextRegisterId();
                        left->Accept(this, &arg);
                        caller_regs_order.push_back(arg);
                        argc++;
                    }
                }
            }
        }
    }
    for (auto it = stms->args().begin(); it != stms->args().end(); ++it) {
        auto temp = (*it).get();
        long arg = block_->NextRegisterId();
        caller_regs_order.push_back(arg);
        temp->Accept(this, &arg);
    }
    if (stms->args_expr() != nullptr && stms->args_expr()->IsArgumentList()) {
        Handle<ArgumentList> arg_list = stms->args_expr()->AsArgumentList();
        for (int i = 0; i < arg_list->length(); i++) {
            long arg = block_->NextRegisterId();
            arg_list->args()->raw_list()[i]->Accept(this, &arg);
            caller_regs_order.push_back(arg);
        }
        argc += arg_list->length();
    }
    AddCallInstruction(ret, OP_CALL, caller_regs_order);
}
    
void CodeGenerator::AddCallInstruction(long ret, OPCode code, std::vector<long> orders) {
    FuncState *func_state = func_->func_state();
    bool reorder = false;
    int regs_count =  static_cast<int>(orders.size());
    for (int i = 1; i < regs_count; i++) {
        if (orders[i] - orders[i - 1] != 1) {
            reorder = true;
            break;
        }
    }
    if (!reorder && regs_count > 0) {
        reorder = orders[regs_count - 1] + 1 >= block_->idx() ? false : true;
    }
    int argc = regs_count - 1;
    if (reorder) {
        long caller = block_->NextRegisterId();
        func_state->AddInstruction(CREATE_ABx(OP_MOVE, caller, orders[0]));
        for (int i = 1; i < orders.size(); i++) {
            long arg = block_->NextRegisterId();
            func_state->AddInstruction(CREATE_ABx(OP_MOVE, arg, orders[i]));
        }
        func_state->AddInstruction(CREATE_ABC(code, ret, argc, caller));
    }
    else {
        func_state->AddInstruction(CREATE_ABC(code, ret, argc, orders[0]));
    }
}

void CodeGenerator::Visit(ArgumentList *node, void *data) {
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
    
void CodeGenerator::Visit(ContinueStatement *node, void *data) {
    do {
        FuncState *func_state = func_->func_state();
        int for_update_index = block_->for_update_index();
        if (for_update_index >= 0) {
            func_state->AddInstruction(CREATE_Ax(OP_GOTO, for_update_index));
        }
        else {
            auto slot = func_state->AddInstruction(0);
            block_->for_continue_slots().push_back(slot);
        }
        
    } while (0);
}

void CodeGenerator::Visit(ForStatement *node, void *data) {
    BlockScope for_scope(this);  // for var index = 0;
    FuncState *func_state = func_->func_state();
    block_->set_is_loop(true);
    long condition = block_->NextRegisterId();
    if (node->init().get() != NULL) {
        node->init()->Accept(this, node->kind() == ForKind::kForIn ? &condition : nullptr);
    }
    int condition_start_index = -1;
    if (node->kind() == ForKind::kForIn) {
        condition_start_index = block_->for_start_index();
    }
    else {
        condition_start_index = (int)func_state->instructions().size();  // aka next one.
        block_->set_for_start_index(condition_start_index);
    }
    if (node->condition().get() != NULL) {
        node->condition()->Accept(this, &condition);
    }
    auto slot = func_state->AddInstruction(0);
    if (node->body().get() != NULL) {
        node->body()->Accept(this, nullptr);
    }
    int for_update_index = condition_start_index;
    if (node->update().get() != NULL) {
        for_update_index = (int)func_state->instructions().size();  // aka next one.
        long update = block_->NextRegisterId();
        node->update()->Accept(this, &update);
    }
    block_->set_for_update_index(for_update_index);
    func_state->AddInstruction(CREATE_Ax(OP_GOTO, condition_start_index));
    int for_end_index = (int)func_->func_state()->instructions().size() - 1;
    func_state->ReplaceInstruction(slot, (CREATE_ABx(OP_JMP, condition, for_end_index - slot)));
    std::vector<size_t> for_contiue_slots = block_->for_continue_slots();
    for (int i = 0; i < for_contiue_slots.size(); i++) {
        func_state->ReplaceInstruction(for_contiue_slots[i], CREATE_Ax(OP_GOTO, for_update_index));
    }
    for_contiue_slots.clear();
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
    long ret = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
    RegisterScope register_scope(block_);
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
            ClassDescriptor *class_desc = ValueTo<ClassDescriptor>(class_->class_value());
            class_desc->funcs_->Add(proto->GetName(), value);            
            func_->func_state()->set_is_class_func(is_class_func);
        }
        // skip func value in the fornt of stack;
        block_->NextRegisterId();

        // make arguments var in thie front of stack;
        if (is_class_func) {
            long arg = block_->NextRegisterId();
            block_->AddVariable("this", arg);
            func_->func_state()->argc()++;
            func_->func_state()->args().push_back(arg);
        }
        for (int i = 0; i < proto->GetArgs().size(); i++) {
            long arg = block_->NextRegisterId();
            block_->AddVariable(proto->GetArgs().at(i), arg);
            func_->func_state()->args().push_back(arg);
            func_->func_state()->argc()++;
        }
        node->body()->Accept(this, nullptr);
        if (func_->func_state()->out_closure().size() > 0) {
            for (int i = 0; i < func_->func_state()->out_closure().size(); i++) {
                ValueRef *ref = func_->func_state()->out_closure()[i];
                func_->func_state()->AddInstruction(CREATE_Ax(OP_REMOVE_CLOSURE, ref->ref_id()));
            }
        }
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
            block_->AddVariable(proto->GetName(), ret);
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
            func_->func_state()->AddInstruction(CREATE_ABx(OP_MOVE, ret, rhs));
        }
    }
}
    
void CodeGenerator::Visit(ArrowFunctionStatement *node, void *data) {
    RegisterScope register_scope(block_);
    long ret = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
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
            ClassDescriptor *class_desc = ValueTo<ClassDescriptor>(class_->class_value());
            if (node->name().size() > 0) {
                class_desc->funcs_->Add(node->name(), value);
            }
            func_->func_state()->set_is_class_func(is_class_func);
        }
        // skip func value in the fornt of stack;
        block_->NextRegisterId();
        // make arguments var in thie front of stack;
        if (is_class_func) {
            long arg = block_->NextRegisterId();
            block_->AddVariable("this", arg);
            func_->func_state()->argc()++;
            func_->func_state()->args().push_back(arg);
        }
        // make arguments var in thie front of stack;
        for (int i = 0; i < node->args().size(); i++) {
            if (node->args()[i]->IsIdentifier()) {
                long arg = block_->NextRegisterId();
                block_->AddVariable(node->args()[i]->AsIdentifier()->GetName(), arg);
                func_->func_state()->argc()++;
                func_->func_state()->args().push_back(arg);
            }
            else if (node->args()[i]->IsCommaExpression()) {
                Handle<ExpressionList> arg_list = node->args()[i]->AsCommaExpression()->exprs();
                for (int j = 0; j < arg_list->Size(); j++) {
                    long arg = block_->NextRegisterId();
                    block_->AddVariable(arg_list->raw_list()[j]->AsIdentifier()->GetName(), arg);
                    func_->func_state()->argc()++;
                    func_->func_state()->args().push_back(arg);
                }
            }
            else {
                // force to error
                throw GeneratorError("arrow function only supporting args list");
            }
        }
        if (node->body()->IsJSXNodeExpression()) {
            long return1 = block_->NextRegisterId();
            node->body()->Accept(this, &return1);
            func_->func_state()->AddInstruction(CREATE_Ax(OP_RETURN1, return1));
        }
        else {
            node->body()->Accept(this, nullptr);
        }
        if (func_->func_state()->out_closure().size() > 0) {
            for (int i = 0; i < func_->func_state()->out_closure().size(); i++) {
                ValueRef *ref = func_->func_state()->out_closure()[i];
                func_->func_state()->AddInstruction(CREATE_Ax(OP_REMOVE_CLOSURE, ref->ref_id()));
            }
        }
    }
    // associate function_name and function_state
    if (func_->parent() == nullptr) {
        if (!is_class_func) {
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
        class_value = exec_state_->class_factory()->CreateClassDescriptor(super_value ? ValueTo<ClassDescriptor>(super_value) : nullptr);
        exec_state_->global()->Add(class_name, class_value);
        ClassScope scope(this, &class_value);
        class_->class_name() = class_name;
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
            FuncState *func_state = func_->func_state();
            long rhs = block_->FindRegisterId(node->name()->AsIdentifier()->GetName());
            if (rhs < 0) {
                int index = exec_state_->global()->IndexOf(node->name()->AsIdentifier()->GetName());
                if (index >= 0) {
                    Value *name = exec_state_->global()->Find(index);
                    if (IsClass(name)) {
                        rhs = block_->NextRegisterId();
                        func_state->AddInstruction(CREATE_ABx(OP_GETGLOBAL, rhs, index));
                    }
                    else {
                        // function call
                        func_state->AddInstruction(CREATE_ABx(OP_GETGLOBAL, lhs, index));
                        break;
                    }
                }
                else {
                    throw GeneratorError("can't find new identifier: " + node->name()->AsIdentifier()->GetName());
                }
            }
            func_state->AddInstruction(CREATE_ABC(OP_NEW, lhs, Value::CLASS_DESC, rhs));
            long caller = block_->NextRegisterId();
            long inst = block_->NextRegisterId();
            func_state->AddInstruction(CREATE_ABx(OP_MOVE, inst, lhs));
            std::vector<long> orders = { caller, inst };
            if (node->args()->Size() > 0) {
                for (int i = 0; i < node->args()->Size(); i++) {
                    long arg = block_->NextRegisterId();
                    orders.push_back(arg);
                    node->args()->raw_list()[i]->Accept(this, &arg);
                }
            }
            AddCallInstruction(lhs, OP_CONSTRUCTOR, orders);
        }
            
    } while (0);

}
    
void CodeGenerator::Visit(JSXNodeExpression *node, void *data) {
    do {
        FuncState *func_state = func_->func_state();
        if (!node->LowerIdentifier()) {
            std::string name = node->Identifier()->AsIdentifier()->GetName();
            int index = exec_state_->global()->IndexOf(name);
            if (index >= 0) {
                node->SetClass(true);
            }
        }
        std::vector<Handle<Expression>> exprs = node->funcexprs();
        for (int i = 0; i < exprs.size(); i++) {
            exprs[i]->Accept(this, nullptr);
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
                func_state->AddInstruction(CREATE_ABx(OP_MOVE, arg_0, reg_parent));
                childrens[i]->Accept(this, &arg_1);
                func_state->AddInstruction(CREATE_ABC(OP_CALL, ret, argc, caller));
            }
        }
        if (data) {
            long ret = *static_cast<long *>(data);
            func_state->AddInstruction(CREATE_ABx(OP_MOVE, ret, reg_parent));
        }
        
    } while (0);
}

void CodeGenerator::Visit(BinaryExpression *node, void *data) {
    long left = -1;
    long ret = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
    if (node->lhs() && (node->lhs()->IsDeclaration() || node->lhs()->IsDeclarationList())) {
        left = block_->NextRegisterId();
        node->lhs()->Accept(this, &left);
    }
    BinaryOperation opeartion = node->op();
    FuncState *func_state = func_->func_state();
    RegisterScope scope(block_);
    if (left < 0) {
        left = block_->NextRegisterId();
        if (node->lhs().get() != NULL) {
            node->lhs()->Accept(this, &left);
        }
    }
    if (opeartion == BinaryOperation::kIn) {
        int for_start_index = (int)func_state->instructions().size();  // aka next one.
        block_->set_for_start_index(for_start_index);
    }
    switch (opeartion) {
        case BinaryOperation::kAddition:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            // a + b
            func_state->AddInstruction(CREATE_ABC(OP_ADD, ret, left, right));
            break;
        }
        case BinaryOperation::kSubtraction:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            // a - b
            func_state->AddInstruction(CREATE_ABC(OP_SUB, ret, left, right));
            break;
        }
        case BinaryOperation::kMultiplication:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            // a * b
            func_state->AddInstruction(CREATE_ABC(OP_MUL, ret, left, right));
            break;
        }
        case BinaryOperation::kDivision:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            // a / b
            func_state->AddInstruction(CREATE_ABC(OP_DIV, ret, left, right));
            break;
        }
        case BinaryOperation::kMod:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            // a % b
            func_state->AddInstruction(CREATE_ABC(OP_MOD, ret, left, right));
            break;
        }
        case BinaryOperation::kLessThan:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_LT, ret, left, right));
            break;
        }
        case BinaryOperation::kGreaterThan:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_GT, ret, left, right));
            break;
        }
        case BinaryOperation::kLessThanEqual:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_LTE, ret, left, right));
            break;
        }
        case BinaryOperation::kGreaterThanEqual:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_GTE, ret, left, right));
            break;
        }
        case BinaryOperation::kAnd:
        {
            auto slot = func_state->AddInstruction(0);
            int second_index = (int)func_state->instructions().size() - 1;
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_AND, ret, left, right));
            auto goto_slot = func_state->AddInstruction(0);
            int third_index = (int)func_state->instructions().size() - 1;
            func_state->AddInstruction(CREATE_ABx(OP_MOVE, ret, left));
            int end_index = (int)func_state->instructions().size();
            func_state->ReplaceInstruction(slot, CREATE_ABx(OP_JMP, left, third_index - second_index));
            func_state->ReplaceInstruction(goto_slot, CREATE_Ax(OP_GOTO, end_index));
            break;
        }
        case BinaryOperation::kStrictEqual:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_SEQ, ret, left, right));
            break;
        }
        case BinaryOperation::kIn:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_IN, ret, left, right));
            break;
        }
        case BinaryOperation::kOr:
        {
            auto slot = func_state->AddInstruction(0);
            int second_index = (int)func_state->instructions().size() - 1;
            func_state->AddInstruction(CREATE_ABx(OP_MOVE, ret, left));
            auto goto_slot = func_state->AddInstruction(0);
            int third_index = (int)func_state->instructions().size() - 1;
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_OR, ret, left, right));
            int end_index = (int)func_state->instructions().size();
            func_state->ReplaceInstruction(slot, CREATE_ABx(OP_JMP, left, third_index - second_index));
            func_state->ReplaceInstruction(goto_slot, CREATE_Ax(OP_GOTO, end_index));
            break;
        }
        case BinaryOperation::kEqual:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_EQ, ret, left, right));
            break;
        }
        case BinaryOperation::kNotEqual:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_EQ, ret, left, right));
            func_state->AddInstruction(CREATE_ABx(OP_NOT, ret, ret));
            break;
        }
        case BinaryOperation::kStrictNotEqual:
        {
            long right = block_->NextRegisterId();
            if (node->rhs().get() != NULL) {
                node->rhs()->Accept(this, &right);
            }
            func_state->AddInstruction(CREATE_ABC(OP_SEQ, ret, left, right));
            func_state->AddInstruction(CREATE_ABx(OP_NOT, ret, ret));
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
    switch (node->op()) {
        case AssignOperation::kAssign:
        {
            if (class_ && node->lhs()->IsMemberExpression() && node->lhs()->AsMemberExpression()->expr()->IsThisExpression()) {
                func_state->AddInstruction(CREATE_ABx(OP_SETMEMBERVAR, left, right));
            }
            else {
                func_state->AddInstruction(CREATE_ABx(OP_MOVE, left, right));
            }
            break;
        }
        case AssignOperation::kAssignAdd:
        {
            long ret = block_->NextRegisterId();
            func_state->AddInstruction(CREATE_ABC(OP_ADD, ret, left, right));
            func_state->AddInstruction(CREATE_ABx(OP_MOVE, left, ret));
            break;
        }
        case AssignOperation::kAssignSub:
        {
            long ret = block_->NextRegisterId();
            func_state->AddInstruction(CREATE_ABC(OP_SUB, ret, left, right));
            func_state->AddInstruction(CREATE_ABx(OP_MOVE, left, ret));
            break;
        }
        default:
        break;
    }
}

void CodeGenerator::Visit(Declaration *node, void *data) {
    long reg = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
    FuncState *func_state = func_->func_state();
    block_->AddVariable(node->name(), reg);
    if (node->expr().get() != nullptr) {
        node->expr()->Accept(this, &reg);
    }
    else {
        func_state->AddInstruction(CREATE_Ax(OP_LOADNULL, reg));
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
    Instruction i = CREATE_ABx(OP_LOADK, reg, index);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(BooleanConstant* node, void* data) {
  long reg = data == nullptr ? -1 : *static_cast<long*>(data);
  if (reg >= 0) {
    FuncState* func_state = func_->func_state();
    bool value = node->pred();
    int index = func_state->AddConstant(static_cast<bool>(value));
    Instruction i = CREATE_ABx(OP_LOADK, reg, index);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(DoubleConstant* node, void* data) {
  long reg = data == nullptr ? -1 : *static_cast<long*>(data);
  if (reg >= 0) {
    FuncState* func_state = func_->func_state();
    double value = node->value();
    int index = func_state->AddConstant(static_cast<double>(value));
    Instruction i = CREATE_ABx(OP_LOADK, reg, index);
    func_state->AddInstruction(i);
  }
}
    
void CodeGenerator::Visit(NullConstant *node, void *data) {
    long reg = data == nullptr ? -1 : *static_cast<long *>(data);
    if (reg >= 0) {
        FuncState *func_state = func_->func_state();
        func_state->AddInstruction(CREATE_Ax(OP_LOADNULL, reg));
    }
}
    
void CodeGenerator::Visit(UndefinedConstant *node, void *data) {
    long reg = data == nullptr ? -1 : *static_cast<long*>(data);
    if (reg >= 0) {
        FuncState *func_state = func_->func_state();
        func_state->AddInstruction(CREATE_Ax(OP_LOADNULL, reg));
    }
}

void CodeGenerator::Visit(ObjectConstant *node, void *data) {
    long ret = data == nullptr ? -1 : *static_cast<long *>(data);
    FuncState *func_state = func_->func_state();
    if (ret >= 0) {
        func_state->AddInstruction(CREATE_ABC(OP_NEW, ret, Value::TABLE, 0));
        if (node->SpreadProperty().size() > 0) {
            std::vector<std::pair<ProxyOrder, std::string>> &orders = node->Orders();
            for (int i = 0; i < orders.size(); i++) {
                switch (orders[i].first) {
                    case ProxyOrder::ProxyArray:
                    {
                        int index = atoi(orders[i].second.c_str());
                        if (index < node->SpreadProperty().size()) {
                            long key = block_->NextRegisterId();
                            node->SpreadProperty()[index]->Accept(this, &key);
                            func_state->AddInstruction(CREATE_ABC(OP_SETTABLE, ret, key, key));
                        }
                        break;
                    }
                    case ProxyOrder::ProxyObject:
                    {
                        RegisterScope scope(block_);
                        auto iter = node->proxy().find(orders[i].second);
                        if (iter != node->proxy().end()) {
                            long item = block_->NextRegisterId();
                            long key = block_->NextRegisterId();
                            iter->second->Accept(this, &item);
                            auto value = exec_state_->string_table_->StringFromUTF8(iter->first);
                            int keyIndex = func_state->AddConstant(std::move(value));
                            func_state->AddInstruction(CREATE_ABx(OP_LOADK, key, keyIndex));
                            func_state->AddInstruction(CREATE_ABC(OP_SETTABLE, ret, key, item));
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        else {
            // expr
            for (auto it = node->proxy().begin(); it != node->proxy().end(); it++) {
                RegisterScope scope(block_);
                long item = block_->NextRegisterId();
                long key = block_->NextRegisterId();
                auto ktemp = (*it).second;
                ktemp->Accept(this, &item);
                auto value = exec_state_->string_table_->StringFromUTF8(it->first);
                int keyIndex = func_state->AddConstant(std::move(value));
                func_state->AddInstruction(CREATE_ABx(OP_LOADK, key, keyIndex));
                func_state->AddInstruction(CREATE_ABC(OP_SETTABLE, ret, key, item));
            }
        }
    }
}
// TODO: this is not correct.
void CodeGenerator::Visit(ArrayConstant *node, void *data) {
    long reg = data == nullptr ? -1 : *static_cast<long*>(data);
    FuncState *func_state = func_->func_state();
    // new table
    if (reg >= 0) {
        func_state->AddInstruction(CREATE_ABC(OP_NEW, reg, Value::ARRAY, 0));
        int index = 0;
        for (auto it = node->exprs().begin(); it != node->exprs().end(); it++) {
            long item = block_->NextRegisterId();
            auto temp = (*it).get();
            temp->Accept(this, &item);
            Instruction i = CREATE_ABC(OP_SETARRAY, reg, index++, item);
            func_state->AddInstruction(i);
        }
    }
}
    
void CodeGenerator::Visit(ClassProperty *node, void *data) {
    RegisterScope scope(block_);
    assert(class_);
    do {
        FuncState *func_state = func_->func_state();
        if (node->is_static()) {
            long lhs = block_->NextRegisterId();
            long rhs = block_->NextRegisterId();
            int index = exec_state_->global()->IndexOf(class_->class_name());
            if (index <= 0) {
                throw GeneratorError("can't find class name " + class_->class_name());
                break;
            }
            func_state->AddInstruction(CREATE_ABx(OP_GETGLOBAL, lhs, index));
            auto constant = exec_state_->string_table_->StringFromUTF8(node->name());
            index = func_state->AddConstant(std::move(constant));
            func_state->AddInstruction(CREATE_ABx(OP_LOADK, rhs, index));
            func_state->AddInstruction(CREATE_ABC(OP_GETMEMBERVAR, lhs, lhs, rhs));
            if (node->init()) {
                node->init()->Accept(this, &rhs);
                func_state->AddInstruction(CREATE_ABx(OP_MOVE, lhs, rhs));
            }
            else {
                func_state->AddInstruction(CREATE_Ax(OP_LOADNULL, lhs));
            }
        }
        
    } while (0);
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
        if (!node->ProduceRValue()) {
            func_state->AddInstruction(CREATE_ABC(OP_GETINDEXVAR, ret, ret, mindex));
        }
        else {
            func_state->AddInstruction(CREATE_ABC(OP_GETINDEX, ret, ret, mindex));
        }
    }
    else if (node->kind() == MemberAccessKind::kDot || node->kind() == MemberAccessKind::kCall) {
        Handle<Expression> left = node->expr();
        left->Accept(this, &ret);
        long right = block_->NextRegisterId();
        auto value = exec_state_->string_table_->StringFromUTF8(node->member()->AsIdentifier()->GetName());
        int tableIndex = func_state->AddConstant(std::move(value));
        func_state->AddInstruction(CREATE_ABx(OP_LOADK, right, tableIndex));
        if (!node->ProduceRValue()) {
            func_state->AddInstruction(CREATE_ABC(OP_GETMEMBERVAR, ret, ret, right));
        }
        else {
            func_state->AddInstruction(CREATE_ABC(OP_GETMEMBER, ret, ret, right));
        }
    }
    else if (node->kind() == MemberAccessKind::kClass) {
        Handle<Expression> left = node->expr();
        left->Accept(this, &ret);
        long right = block_->NextRegisterId();
        auto value = exec_state_->string_table_->StringFromUTF8(node->member()->AsIdentifier()->GetName());
        int tableIndex = func_state->AddConstant(std::move(value));
        func_state->AddInstruction(CREATE_ABx(OP_LOADK, right, tableIndex));
        if (!node->ProduceRValue()) {
            func_state->AddInstruction(CREATE_ABC(OP_GETMEMBERVAR, ret, ret, right));
        }
        else {
            func_state->AddInstruction(CREATE_ABC(OP_GETMEMBER, ret, ret, right));
        }
    }
}

void CodeGenerator::Visit(Identifier *node, void *data) {
  RegisterScope register_scope(block_);
  long reg_a = data == nullptr ? block_->NextRegisterId() : *static_cast<long *>(data);
  if (reg_a >= 0) {
    FuncState *state = func_->func_state();
    long reg_b = block_->FindRegisterId(node->GetName());
    if (reg_b >= 0) {
      state->AddInstruction(CREATE_ABx(OP_MOVE, reg_a, reg_b));
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
          CREATE_ABx(OP_LOADK, right, tableIndex));
      func_state->AddInstruction(
          CREATE_ABC(OP_GETMEMBER, reg_a, this_idx, right));
    } else {
      // make data undefined.
      state->AddInstruction(CREATE_Ax(OP_LOADNULL, reg_a));
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
        func_->func_state()->AddInstruction(CREATE_ABx(OP_PREV_INCR, reg, ret));
    }
    // --i
    else if (operation == PrefixOperation::kDecrement) {
        func_->func_state()->AddInstruction(CREATE_ABx(OP_PREV_DECR, reg, ret));
    }
    else if (operation == PrefixOperation::kNot) {
        func_->func_state()->AddInstruction(CREATE_ABx(OP_NOT, ret, reg));
    }
    else if (operation == PrefixOperation::kUnfold) {
        func_->func_state()->AddInstruction(CREATE_ABx(OP_MOVE, ret, reg));
    }
    else if (operation == PrefixOperation::kTypeof) {
        func_->func_state()->AddInstruction(CREATE_ABx(OP_TYPEOF, ret, reg));
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
        func_->func_state()->AddInstruction(CREATE_ABx(OP_POST_INCR, reg, ret));
    }
    // i++
    else if (operation == PostfixOperation::kDecrement) {
        func_->func_state()->AddInstruction(CREATE_ABx(OP_POST_DECR, reg, ret));
    }
}

void CodeGenerator::Visit(ReturnStatement *node, void *data) {
    FuncState *func_state = func_->func_state();
    if (node->expr() == nullptr) {
        func_state->AddInstruction(CREATE_Ax(OP_RETURN0, 0));
    }
    else {
        long ret = block_->NextRegisterId();
        node->expr()->Accept(this, &ret);
        func_state->AddInstruction(CREATE_Ax(OP_RETURN1, ret));
    }
}

void CodeGenerator::Visit(SwitchStatement* node, void* data) {
    BlockScope for_scope(this);
    block_->set_is_switch(true);

    long test_value_id = block_->NextRegisterId();
    long case_value_id = block_->NextRegisterId();
    long test_result_id = block_->NextRegisterId();

    FuncState *func_state = func_->func_state();
    auto &cases = node->get_cases();
    //load test value into [test_value_id]
    node->test_case()->Accept(this,&test_value_id);
    std::vector<size_t> switch_case_index;
    std::vector<size_t> jump_slots;

    //value test for cases
    int defaultIndex = -1;
    Handle<CaseStatement> defaultCase;
    int index = 0;
    for(auto& a_case: cases){
        //load case value into [case_value_id]
        const Handle<CaseStatement>& caseStatement = a_case->AsCaseStatement();
        if (caseStatement->is_default()){
            jump_slots.push_back(0);
            defaultCase = caseStatement;
            defaultIndex = index;
            continue;
        }
        caseStatement->test_case()->Accept(this, &case_value_id);
        func_state->AddInstruction(CREATE_ABC(OP_EQ, test_result_id, test_value_id, case_value_id));
        size_t jump_slot = func_state->AddInstruction(0);
        jump_slots.push_back(jump_slot);
        index++;
    }

    size_t skip_to_end_slot = 0;
    if (defaultIndex != -1){
        //has default case
        size_t defaultJump = func_state->AddInstruction(0);
        jump_slots[defaultIndex] = defaultJump;
    } else {
        //no default
        skip_to_end_slot = func_state->AddInstruction(0);
    }

    //expression list for cases
    for(auto& a_case: cases){
        size_t lable_index = func_state->instructions().size();
        switch_case_index.push_back(lable_index);

        a_case->AsCaseStatement()->statements()->Accept(this, nullptr);
    }

    //replace case jump;
    index = 0;
    for(size_t to_replace:jump_slots){
        if(index == defaultIndex){
            func_state->ReplaceInstruction(to_replace,CREATE_Ax(OP_GOTO,switch_case_index[index]));
        } else {
            func_state->ReplaceInstruction(to_replace,CREATE_ABx(OP_TRUE_JMPTO,test_result_id,switch_case_index[index]));
        }
        index++;
    }

    //replace break;
    size_t end_index = func_state->instructions().size();;
    for(auto& to_replace:block_->break_slots()){
        func_state->ReplaceInstruction(to_replace, CREATE_Ax(OP_GOTO,end_index));
    }

    if (defaultIndex == -1){
        //no default, replace skip instruction;
      func_state->ReplaceInstruction(skip_to_end_slot, CREATE_Ax(OP_GOTO,end_index));
    }
}

void CodeGenerator::Visit(BreakStatement* node, void* data) {
    auto slot = func_->func_state()->AddInstruction(0);
    block_->break_slots().push_back(slot);
}

void CodeGenerator::Visit(TryCatchStatement* node, void* data) {
    node->try_block()->Accept(this, nullptr);
    if(node->finally()!= nullptr){
        node->finally()->Accept(this, nullptr);
    }
}

CodeGenerator::RegisterScope::~RegisterScope()
{
    int start_idx = stored_idx_;
    BlockCnt *block = block_;
    FuncState *func_state = block_->func_state();
    if (func_state) {
        while (block) {
            if (block->func_state() == func_state) {
                auto iter = block->variables().begin();
                while (iter != block->variables().end()) {
                    long reg_ref = iter->second;
                    if (reg_ref >= start_idx) {
                        iter = block->variables().erase(iter);
                    }
                    else {
                        iter++;
                    }
                }
            }
            block = block->parent();
        }
    }
    block = block_;
    if (func_state) {
        while (block && block->reg_refs().size() > 0) {
            if (block->func_state() == func_state) {
                auto iter = block->reg_refs().begin();
                while (iter != block->reg_refs().end()) {
                    long reg_ref = *iter;
                    if (reg_ref >= start_idx) {
                        func_state->AddInstruction(CREATE_Ax(OP_RESETOUTVAR, reg_ref));
                        iter = block->reg_refs().erase(iter);
                    }
                    else {
                        iter++;
                    }
                }
            }
            block = block->children();
        }
    }
    block_->is_register_scope() = false;
    block_->set_idx(stored_idx_);
}
    
void CodeGenerator::BlockCnt::AddVariable(const std::string &name, long reg) {
    auto iter = variables_.find(name);
    if (iter != variables_.end()) {
        variables_.erase(iter);
    }
    variables_.insert(std::make_pair(name, reg));
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
    
int CodeGenerator::BlockCnt::for_start_index() {
    if (is_loop()) {
        return for_start_index_;
    }
    if (parent() != nullptr) {
        if (parent()->is_loop()) {
            return parent()->for_start_index();
        }
    }
    return -1;
}
    
std::vector<size_t>& CodeGenerator::BlockCnt::for_continue_slots() {
    if (is_loop()) {
        return for_continue_slots_;
    }
    if (parent() != nullptr) {
        if (parent()->is_loop()) {
            return parent()->for_continue_slots_;
        }
    }
    return for_continue_slots_;
}

std::vector<size_t>& CodeGenerator::BlockCnt::break_slots() {
    if (is_loop()||is_switch()) {
        return for_break_slots_;
    }
    if (parent() != nullptr) {
        return parent()->break_slots();
    }
    return for_break_slots_;
}

int CodeGenerator::BlockCnt::for_update_index() {
    if (is_loop()) {
        return for_update_index_;
    }
    if (parent() != nullptr) {
        if (parent()->is_loop()) {
            return parent()->for_update_index();
        }
    }
    return -1;
}

void CodeGenerator::BlockCnt::reset() {
    for (int i = 0; i < reg_refs_.size(); i++) {
        if (func_state()) {
            func_state()->AddInstruction(CREATE_Ax(OP_RESETOUTVAR, reg_refs_[i]));
        }
    }
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
                root_block->reg_refs().push_back(reg_ref);
                if (ref->is_closure()) {
                    func_state_->AddInClosure(ref);
                    func_state_->AddInstruction(CREATE_ABx(OP_IN_CLOSURE, reg_ref, ref->ref_id()));
                }
                else {
                    func_state_->AddInstruction(CREATE_ABx(OP_GETOUTVAR, reg_ref, ref->ref_id()));
                }
            }
            return reg_ref;
        }
    }
    return -1;
}
    
ValueRef *CodeGenerator::BlockCnt::FindValueRef(const std::string &name, long &reg_ref) {
    auto iter = variables_.find(name);
    if (iter != variables_.end()) {
        if (exec_state_ && func_state_ ) {
            ValueRef *ref = exec_state_->AddRef(func_state_, iter->second);
            bool is_closure = func_state_->Inclusive(iter->second);
            if (is_closure) {
                ref->is_closure() = is_closure;
                func_state_->AddOutClosure(ref);
            }
            Instruction instruction = func_state_->instructions().size() > 0 ? func_state_->instructions()[func_state_->instructions().size() - 1] : 0;
            OPCode op(GET_OP_CODE(instruction));
            if (instruction && (op == OP_RETURN0 || op == OP_RETURN1)) {
                func_state_->instructions().pop_back();
            }
            func_state_->AddInstruction(CREATE_ABx(is_closure ? OP_OUT_CLOSURE : OP_SETOUTVAR, iter->second, ref->ref_id()));
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
