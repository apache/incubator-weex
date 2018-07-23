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

#ifndef local_cfunction_h
#define local_cfunction_h

#include <stdio.h>
#include "exec_state.h"
#include "object.h"
#include "vnode_render_manager.h"

namespace weex {
namespace core {
namespace data_render {

class local_cfunction {
 public:
  ~local_cfunction();

  static void registerCFunction(ExecState* exec_state) {
    // size of table
    Value sizeof_func;
    sizeof_func.type = Value::Type::CFUNC;
    sizeof_func.cf = reinterpret_cast<void*>(GetTableSize);
    exec_state->global()->Add("sizeof", sizeof_func);

    // log
    Value log_func;
    log_func.type = Value::Type::CFUNC;
    log_func.cf = reinterpret_cast<void*>(Log);
    exec_state->global()->Add("log", log_func);
  }

  static Value GetTableSize(ExecState* exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length > 0) {
      Value* value = exec_state->GetArgument(0);
      if (value->type == Value::Type::TABLE) {
        Table* table = TableValue(value);
        if (table->sizearray > 0) {
          return Value(static_cast<int64_t>(table->sizearray));
        } else if (table->map->size() > 0) {
          return Value(static_cast<int64_t>(table->map->size()));
        };
      }
    }
    return Value(static_cast<int64_t>(-1));
  }

  static Value Log(ExecState* exec_state) {
    size_t length = exec_state->GetArgumentCount();
    for (int i = 0; i < length; ++i) {
      Value* a = exec_state->GetArgument(i);
      switch (a->type) {
        case weex::core::data_render::Value::STRING:
          std::cout << a->str->c_str();
          break;
        case weex::core::data_render::Value::NUMBER:
          std::cout << a->n;
          break;
        case weex::core::data_render::Value::INT:
          std::cout << a->i << "\n";
          break;
        default:
          break;
      }
    }
    return Value();
  }
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif
