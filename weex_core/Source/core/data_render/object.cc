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
#include <cstdlib>
#include "core/data_render/object.h"

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
  } else if (IsString(o)) {
      v = atoi(CStringValue(o));
      return 1;
      
  } else {
    return 0;
  }
}
        
bool ObjectEquals(const Value *a, const Value *b) {
    bool equal = false;
    do {
        if (a->type != b->type) {
            break;
        }
        switch (a->type) {
            case Value::STRING:
            {
                equal = std::string(CStringValue(a)) == std::string(CStringValue(b));
                break;
            }
            case Value::BOOL:
            {
                equal = BoolValue(a) == BoolValue(b);
                break;
            }
            case Value::INT:
            {
                equal = IntValue(a) == IntValue(b);
                break;
            }
            case Value::NUMBER:
            {
                equal = NumValue(a) == NumValue(b);
                break;
            }
            default:
                break;
        }
        
    } while (0);
    
    return equal;
}

bool ValueEquals(const Value *a, const Value *b) {
    double d1, d2;
    if (IsNumber(a)) {
        return NumEq(NumValue(a), NumValue(b));
    }
    else if (IsInt(a)) {
        return IntValue(a) == IntValue(b);
    }
    else if (IsBool(a)) {
        return BoolValue(a) == BoolValue(b);
    }
    else if (ToNum(a, d1) && ToNum(b, d2)) {
        return NumEq(d1, d2);
    }
    else if (IsString(a) && IsString(b)) {
        return strcmp(CStringValue(a), CStringValue(b)) == 0;
    }
    else {
        return false;
    }
}
    
bool ValueStrictEquals(const Value *a, const Value *b) {
    double d1, d2;
    if (IsNumber(a)) {
        return NumEq(NumValue(a), NumValue(b));
    }
    else if (IsInt(a)) {
        return IntValue(a) == IntValue(b);
    }
    else if (IsBool(a)) {
        return BoolValue(a) == BoolValue(b);
    }
    else if (ToNum(a, d1) && ToNum(b, d2)) {
        return NumEq(d1, d2);
    }
    else if (IsString(a) && IsString(b)) {
        return strcmp(CStringValue(a), CStringValue(b)) == 0;
    }
    else if (IsNil(a)) {
        return IsNil(b) ? true : false;
    }
    else if (IsNil(b)) {
        return IsNil(a) ? true : false;
    }
    else {
        return false;
    }
}
    
bool ValueAND(const Value *a, const Value *b) {
    bool aval = false, bval = false;
    ToBool(a, aval);
    ToBool(b, bval);
    return aval && bval;
}
    
bool ValueOR(const Value *a, const Value *b) {
    if (IsBool(a) && IsBool(b)) {
        return BoolValue(a) || BoolValue(b);
    }
    return false;
}

bool ValueLT(const Value *a, const Value *b) {
    double d1, d2;
    if (IsNumber(a) && IsNumber(b)) {
        return NumLT(NumValue(a), NumValue(b));
    }
    else if (IsInt(a) && IsInt(b)) {
        return IntValue(a) < IntValue(b);
    }
    else if (ToNum(a, d1) && ToNum(b, d2)) {
        return NumLT(d1, d2);
    }
    else {
        return false;
    }
}
    
bool ValueLTE(const Value *a, const Value *b) {
    double d1, d2;
    if (IsNumber(a) && IsNumber(b)) {
        return NumLTE(NumValue(a), NumValue(b));
    }
    else if (IsInt(a) && IsInt(b)) {
        return IntValue(a) <= IntValue(b);
    }
    else if (ToNum(a, d1) && ToNum(b, d2)) {
        return NumLTE(d1, d2);
    }
    else {
        return false;
    }
}
    
bool ValueGT(const Value *a, const Value *b) {
    double d1, d2;
    if (IsNumber(a) && IsNumber(b)) {
        return NumGT(NumValue(a), NumValue(b));
    }
    else if (IsInt(a) && IsInt(b)) {
        return IntValue(a) > IntValue(b);
    }
    else if (ToNum(a, d1) && ToNum(b, d2)) {
        return NumGT(d1, d2);
    }
    else if (IsInt(b) && IsString(a)) {
        int64_t aval = 0;
        ToInteger(a, 0, aval);
        return aval > IntValue(b);
    }
    else {
        return false;
    }
}

bool ValueGTE(const Value *a, const Value *b) {
    double d1, d2;
    if (IsNumber(a) && IsNumber(b)) {
        return NumGTE(NumValue(a), NumValue(b));
    }
    else if (IsInt(a) && IsInt(b)) {
        return IntValue(a) >= IntValue(b);
    }
    else if (ToNum(a, d1) && ToNum(b, d2)) {
        return NumGTE(d1, d2);
    }
    else {
        return false;
    }
}

Value* Variables::Find(int index) {
    if (index >= values_.size() || index < 0) {
        return nullptr;
    }
    return &values_[index];
}

int Variables::IndexOf(const std::string& name) {
    auto iter = map_.find(name);
    if (iter != map_.end()) {
        return iter->second;
    }
    return -1;
}

int Variables::Add(const std::string& name, Value value) {
    auto iter = map_.find(name);
    if (iter != map_.end()) {
        return iter->second;
    }
    values_.push_back(value);
    GCRetain(&value);
    int index = (int)values_.size() - 1;
    map_.insert(std::make_pair(name, index));
    return index;
}

int Variables::Add(Value value) {
    values_.push_back(value);
    GCRetain(&value);
    return (int)values_.size() - 1;
}

int Variables::Set(const std::string& name, Value value) {
    auto iter = map_.find(name);
    if (iter != map_.end()) {
        int index = iter->second;
        GCRelease(&values_[static_cast<size_t>(index)]);
        values_[static_cast<size_t>(index)] = value;
        GCRetain(&values_[static_cast<size_t>(index)]);
        return index;
    }
    else {
        values_.push_back(value);
        int index = (int)values_.size() - 1;
        map_.insert(std::make_pair(name, index));
        GCRetain(&value);
        return index;
    }
}
    
void SetRefValue(Value *o) {
    Value *value = o;
    while (value->ref) {
        *value->ref = *value;
        value = value->ref;
    }
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
