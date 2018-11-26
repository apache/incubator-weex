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

#ifndef CORE_DATA_RENDER_VM_H
#define CORE_DATA_RENDER_VM_H

#include <limits.h>
#include "core/data_render/op_code.h"

#define MAXINTEGER INT_MAX
#define MININTEGER INT_MIN

namespace weex {
namespace core {
namespace data_render {
class ExecState;
class FuncState;
class Value;

struct Frame {
  Value *reg;
  Value *ret;
  FuncState *func;
  const Instruction *pc;
  const Instruction *end;
};

class VM {
 public:
  VM() {}
  ~VM() {}    
  void RunFrame(ExecState *exec_state, Frame frame, Value* ret);
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_VM_H
