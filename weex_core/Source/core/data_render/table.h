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

#ifndef DATA_RENDER_TABLE_H
#define DATA_RENDER_TABLE_H

#include "core/data_render/object.h"

namespace weex {
namespace core {
namespace data_render {

int SetTableValue(Table *, Value *, const Value &);
    
Value *GetTableValue(Table *, const Value &);
    
Value *GetTableVar(Table *table, const Value &key);
    
Value *GetTableValue(Table *table, std::string key);
    
std::vector<std::string> GetTableKeys(Table *table);
    
size_t GetTableSize(Table *);
    
std::string TableToString(Table *table);
    
std::string ArrayToString(Array *array);
    
int TableInKey(StringTable *string_table,Table *table, Value *condition, Value *var);
    
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // DATA_RENDER_TABLE_H
