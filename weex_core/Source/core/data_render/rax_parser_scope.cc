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
//
// Created by pengtao.pt on 2018/7/25.
//

#include "core/data_render/rax_parser_scope.h"
#include "core/data_render/rax_parser_context.h"
#include "core/data_render/ast.h"

namespace weex {
namespace core {
namespace data_render {

Scope::Scope(Value *root, Scope *parent) : root_{root}, parent_{parent}
{
    // in case of global scope the root is empty
    if (root_)
        root_->SetScope(this);
    symbol_table_ = new SymbolTable(this);
}
    
Scope::~Scope() {
    delete symbol_table_;
}
    
SymbolTable *Scope::symbol_table() {
    return symbol_table_;
}
    
SymbolTable::SymbolTable(Scope *scope) : scope_{scope}
{ }
    
void SymbolTable::Push(const Name &name, Value *value) {
    entries_[name] = value;
}
    
bool SymbolTable::Exists(const Name &name) const {
    return !!entries_.count(name);
}
    
Scope *SymbolTable::scope() {
    return scope_;
}
    
SymbolTable::Value *SymbolTable::Get(const Name &name) {
    auto it = entries_.find(name);
    if (it == entries_.end()) {
        return nullptr;
    }
    return (*it).second;
}
    
ScopeManager::ScopeManager(ParserContext *context) : context_{context}, global_scope_{context->globalScope()}, current_{global_scope_}
{ }
    
void ScopeManager::PushScope(Scope *scope) {
    scope_stack_.push_back(scope);
}
    
Scope *ScopeManager::NewScope(SymbolTable::Value *value) {
    Scope *scope = new Scope(value, current_);
    PushScope(current_);
    return (current_ = scope);
}

Scope *ScopeManager::PopScope() {
    Scope *scope = current_;
    current_ = scope_stack_.back();
    scope_stack_.pop_back();
    return scope;
}
    
}
}
}
