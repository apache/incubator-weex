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
// Created by pentao.pt on 2018/7/25.
//

#ifndef DATA_RENDER_RAX_PARSER_SCOPE_
#define DATA_RENDER_RAX_PARSER_SCOPE_

#include <map>
#include <list>

namespace weex {
namespace core {
namespace data_render {

class Expression;
class SymbolTable;
class ParserContext;

class Scope {
public:
    using Value = Expression;
public:
    Scope(Value *root, Scope *parent);
    ~Scope();
    SymbolTable *symbol_table();
    Scope *parent() { return parent_; }
private:
    // root of tree where scope starts
    Value *root_;
    SymbolTable *symbol_table_;
    Scope *parent_;
};

class SymbolTable {
public:
    using Name = std::string;
    using Value = Expression;
    
    SymbolTable(Scope *scope);
    
    void Push(const Name &name, Value *value);
    
    bool Exists(const Name &name) const;
    
    Value *Get(const Name &name);
    
    Scope *scope();
private:
    std::map<Name, Value *> entries_;
    Scope *scope_;
};

// ScopeManager manages stack of scope. It keeps a pointer to the current scope
// and hence current SymbolTable.
class ScopeManager {
public:
    ScopeManager(ParserContext *context);
    
    // NewScope creates a new scope and pushes the current scope to the stack
    // and makes current_ point to new scope
    Scope *NewScope(SymbolTable::Value *value);
        
    // pops the scope from stack, and current now points to popped scope.
    Scope *PopScope();
        
    Scope *current() { return current_; }
        
    Scope *global_scope() { return global_scope_; }
    
private:
    void PushScope(Scope *scope);
    ParserContext *context_;
    Scope *global_scope_;
    Scope *current_;
    std::list<Scope *> scope_stack_;
};
    
}
}
}

#endif
