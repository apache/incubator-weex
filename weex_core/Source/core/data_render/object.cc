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

namespace weex {
namespace core {
namespace data_render {

double NumPow(const double &d1, const double &d2) {
  return MATH_OP(pow)(d1, d2);
}

double NumIDiv(const double &d1, const double &d2) {
  return MATH_OP(floor)(NUM_OP(/, d1, d2));
}

double NumMod(const double &d1, const double &d2) {
  double ret = MATH_OP(fmod)(d1, d2);
  if (ret * d2 < 0) ret += d2;
  return ret;
}

bool NumEq(const double &d1, const double &d2) { return d1 == d2; }

bool NumLT(const double &d1, const double &d2) { return d1 < d2; }

int Number2Int(const double &n, int64_t &p) {
  if (n >= MININTEGER && n < -MININTEGER) {
    p = n;
    return 1;
  }
  return 0;
}

double NumUnm(const double &d) { return -d; }

int64_t ShiftLeft(const int64_t &a, const int64_t &b) {
  if (b < 0) {
    if (b <= -NUM_BITS) {
      return 0;
    } else {
      return INT_OP(>>, a, b);
    }
  } else {
    if (b >= NUM_BITS) {
      return 0;
    } else {
      return INT_OP(<<, a, b);
    }
  }
}

bool IsInt(const Value *o) { return Value::Type::INT == o->type; }

bool IsNil(const Value *o) {
  return nullptr == o || Value::Type::NIL == o->type;
}

void SetNil(Value *o) {
  if (nullptr != o) {
    o->type = Value::Type::NIL;
  }
}

int IntMod(const int &a, const int &b) {
  if (CAST_S2U(b) + 1u <= 1u) {
    if (b == 0) {
      LOGE("Error ValueMod Values[%d, %d]", a, b);
    }
    return 0;
  } else {
    int ret = a % b;
    if (ret != 0 && (a ^ b) < 0) {
      ret += b;
    }
    return ret;
  }
}

bool IsNumber(const Value *o) { return Value::Type::NUMBER == o->type; }

bool IsBool(const Value *o) { return Value::Type::BOOL == o->type; }

bool IsTable(const Value *o) { return Value::Type::TABLE == o->type; }

bool IsString(const Value *o) { return nullptr != o && Value::Type::STRING == o->type; }

int64_t IntValue(const Value *o) { return o->i; }

double NumValue(const Value *o) { return o->n; }

bool BoolValue(const Value *o) {
  return (Value::Type::BOOL == o->type) ? o->b : false;
}

String *StringValue(const Value *o) { return IsString(o) ? o->str : nullptr; }

char *CStringValue(const Value *o) {
  return (IsString(o) && nullptr != o->str) ? o->str->c_str() : nullptr;
}

int ToNum(const Value *o, double &n) {
  return IsNumber(o) ? (n = NumValue(o), 1) : ToNumber_(o, n);
}

void SetIValue(Value *o, int iv) {
  o->type = Value::Type::INT;
  o->i = iv;
}

void SetDValue(Value *o, double d) {
  o->type = Value::Type::NUMBER;
  o->n = d;
}

void SetBValue(Value *o, bool b) {
  o->type = Value::Type::BOOL;
  o->b = b;
}

void SetTValue(Value *v, GCObject *o) {
  v->type = Value::Type::TABLE;
  v->gc = o;
}

void SetSValue(Value *v, String *s) {
  v->type = Value::Type::STRING;
  v->str = s;
}

String *StringAdd(StringTable *t, Value *a, Value *b) {
    std::string str;
    if (IsString(a)) {
        str = a->str->c_str();
        return t->StringFromUTF8(str += ToCString(b));
    } else {
        str = b->str->c_str();
        return t->StringFromUTF8(str += ToCString(a));
    }
}

std::string ToCString(const Value *o) {
    switch (o->type) {

        case Value::Type::INT: {
            std::stringstream ss;
            ss<<IntValue(0);
            return ss.str();
        }

        case Value::Type::NUMBER: {
            std::stringstream ss;
            ss<<NumValue(o);
            return ss.str();
        }

        case Value::Type::BOOL: {
            std::stringstream ss;
            ss<<BoolValue(o);
            return ss.str();
        }

        case Value::Type::STRING: {
            return CStringValue(o);
        }

        default:
            return "";
    }
}

int ToBool(const Value *o, bool &b) {
  double d1;
  if (Value::Type::BOOL == o->type) {
    b = BoolValue(o);
  } else if (Value::Type::INT == o->type) {
    b = IntValue(o);
  } else if (Value::Type::NUMBER == o->type) {
    b = NumValue(o);
  } else if (ToNum(o, d1)) {
    b = d1;
  } else {
    b = false;
    return 0;
  }
  return 1;
}

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
  if (a->type != b->type) {
    return false;
  }
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
  if (a->type != b->type) {
    return false;
  }
  double d1, d2;
  d1 = NumValue(a);
  d2 = NumValue(b);
label:
  if (IsNumber(a)) {
    return NumLT(d1, d2) || NumEq(d1, d2);
  } else if (IsInt(a)) {
    return IntValue(a) <= IntValue(b);
  } else if (ToNum(a, d1) && ToNum(b, d2)) {
    goto label;
  } else {
    return false;
  }
}

bool ValueLT(const Value *a, const Value *b) {
  if (a->type != b->type) {
    return false;
  }
  double d1, d2;
  if (IsNumber(a)) {
    return NumLT(NumValue(a), NumValue(b));
  } else if (IsInt(a)) {
    return IntValue(a) < IntValue(b);
  } else if (ToNum(a, d1) && ToNum(b, d2)) {
    return NumLT(d1, d2);
  } else {
    return false;
  }
}

void FreeValue(Value *o) {
  if (!o) {
    return;
  }
  if (Value::Type::TABLE == o->type) {
    delete o->gc;
    delete o;
  }
}

}  // namespace data_render
}  // namespace core
}  // namespace weex