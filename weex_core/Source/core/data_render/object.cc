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

#include <sstream>
#include "core/data_render/object.h"
#include "core/data_render/vm_mem.h"

namespace weex {
namespace core {
namespace data_render {

/*
** try to convert a value to an integer, rounding according to 'mode':
** mode == 0: accepts only integral values
** mode == 1: takes the floor of the number
** mode == 2: takes the ceil of the number
*/
int ToInteger(const Value *o, const int &mode, int64_t &v) {
  Value tmp;
  double d;
  label:
  if (IsNumber(o)) {
    double n = NumValue(o);
    double f = MATH_OP(floor)(n);
    if (n != f) {
      if (mode == 0) {
        return 0;
      } else if (mode > 1) {
        f += 1;
      }
    }
    return Number2Int(f, v);
  } else if (IsInt(o)) {
    v = IntValue(o);
    return 1;
  } else if (ToNum(o, d)) {
    SetDValue(&tmp, d);
    o = &tmp;
    goto label;
  } else {
    return 0;
  }
}

bool ValueEqulas(const Value *a, const Value *b) {
  double d1, d2;
  if (IsNumber(a)) {
    return NumEq(NumValue(a), NumValue(b));
  } else if (IsInt(a)) {
    return IntValue(a) == IntValue(b);
  } else if (IsBool(a)) {
    return BoolValue(a) == BoolValue(b);
  } else if (ToNum(a, d1) && ToNum(b, d2)) {
    return NumEq(d1, d2);
  } else {
    return false;
  }
}

bool ValueLE(const Value *a, const Value *b) {
  double d1, d2;
  d1 = NumValue(a);
  d2 = NumValue(b);
  if (IsNumber(a) && IsNumber(b)) {
    return NumLT(d1, d2) || NumEq(d1, d2);
  } else if (IsInt(a) && IsInt(b)) {
    return IntValue(a) <= IntValue(b);
  } else if (ToNum(a, d1) && ToNum(b, d2)) {
    return NumLT(d1, d2) || NumEq(d1, d2);
  } else {
    return false;
  }
}

bool ValueLT(const Value *a, const Value *b) {
  double d1, d2;
  if (IsNumber(a) && IsNumber(b)) {
    return NumLT(NumValue(a), NumValue(b));
  } else if (IsInt(a) && IsInt(b)) {
    return IntValue(a) < IntValue(b);
  } else if (ToNum(a, d1) && ToNum(b, d2)) {
    return NumLT(d1, d2);
  } else {
    return false;
  }
}

void FreeValue(Value *o) {
  if (nullptr == o) {
    return;
  }
  if (Value::Type::TABLE == o->type) {
    freeMem(o->gc);
    delete o;
  }
}

}  // namespace data_render
}  // namespace core
}  // namespace weex