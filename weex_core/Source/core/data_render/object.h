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

#ifndef CORE_DATA_RENDER_OBJECT_H
#define CORE_DATA_RENDER_OBJECT_H

#include <vector>
#include <unordered_map>
#include <cmath>
#include "core/data_render/string_table.h"
#include "core/data_render/vm.h"

#define CommonHeader GCObject *gc

#define LOGE(...) ((void)0)

#define INT_OP(op, v1, v2) CAST_U2S((CAST_S2U(v1))op CAST_S2U(v2))
#define NUM_OP(op, d1, d2) ((d1)op(d2))
#define MATH_OP(op) op
#define CAST_S2U(o) ((unsigned)(o))
#define CAST_U2S(o) ((signed)(o))
#define CAST(t, exp) ((t)(exp))
#define CAST_INT(i) CAST(int64_t, (i))
#define NUM_BITS CAST_INT(sizeof(int64_t) * CHAR_BIT)

namespace weex {
namespace core {
namespace data_render {
class VM;

class Frame;

class ExecState;

class FuncState;

class String;

class StringTable;

class Value;

typedef unsigned char byte;

typedef struct GCObject {
  CommonHeader;
} GCObject;

typedef struct Key {
  int next;
  Value *val;
} Key;

typedef struct Node {
  Key *key;
  Value *val;
  Node *next;
} Node;

typedef struct Table {
  CommonHeader;

  size_t sizearray; /* size of 'array' array */
  size_t sizenode;
  std::vector<Value> *array; /* array part */
  int *hash;
  Node *node;
  std::unordered_map<std::string, Value> *map;

} Table;

typedef Value (*CFunction)(ExecState *);

struct Value {
  union {
    GCObject *gc;
    int64_t i;
    double n;
    bool b;
    FuncState *f;
    void *cf;
    String *str;
    void *cptr;  // Lifecycle is managed outside vm
  };

  enum Type { NIL, INT, NUMBER, BOOL, FUNC, CFUNC, STRING, CPTR, TABLE };

  Type type;

  Value() : type(NIL) {}

  Value(int64_t value) : i(value), type(INT) {}

  Value(double value) : n(value), type(NUMBER) {}

  Value(bool value) : b(value), type(BOOL) {}

  Value(String *value) : str(value), type(STRING) {}

  Value(const Value &value) {
    type = value.type;
    switch (type) {
      case INT:
        i = value.i;
        break;
      case NUMBER:
        n = value.n;
        break;
      case BOOL:
        b = value.b;
        break;
      case STRING:
        str = value.str;
        break;
      case FUNC:
        f = value.f;
        break;
      case CFUNC:
        cf = value.cf;
        break;
      case CPTR:
        cptr = value.cptr;
      case TABLE:
        gc = value.gc;
        break;
      default:
        break;
    }
  }

  friend bool operator==(const Value &left, const Value &right) {
    if (left.type != right.type) return false;
    switch (left.type) {
      case NIL:
        return true;
      case INT:
        return left.i == right.i;
      case NUMBER:
        return fabs(left.n - right.n) < 0.000001;
      case BOOL:
        return left.b == right.b;
      case STRING:
        return left.str == right.str;
      case FUNC:
        return left.f == right.f;
      case CFUNC:
        return left.cf == right.cf;
      case CPTR:
        return left.cptr == right.cptr;
      case TABLE:
        return left.gc == right.gc;
      default:
        break;
    }
    return false;
  }
};

double NumPow(const double &d1, const double &d2);

double NumIDiv(const double &d1, const double &d2);

double NumMod(const double &d1, const double &d2);

bool NumEq(const double &d1, const double &d2);

bool NumLT(const double &d1, const double &d2);

int Number2Int(const double &n, int64_t &p);

double NumUnm(const double &d);

int64_t ShiftLeft(const int64_t &a, const int64_t &b);

bool IsInt(const Value *o);

bool IsNil(const Value *o);

void SetNil(Value *o);

int IntMod(const int &a, const int &b);

bool IsNumber(const Value *o);

bool IsBool(const Value *o);

bool IsTable(const Value *o);

bool IsString(const Value *o);

int64_t IntValue(const Value *o);

double NumValue(const Value *o);

bool BoolValue(const Value *o);

String *StringValue(const Value *o);

char *CStringValue(const Value *o);

inline Table* TableValue(const Value *o) {
  if (IsTable(o)) {
    return reinterpret_cast<Table *>(o->gc);
  }
  return nullptr;
}

inline int ToNumber_(const Value *value, double &ret) {
  if (IsInt(value)) {
    ret = IntValue(value);
    return 1;
  } else if (IsNumber(value)) {
    ret = NumValue(value);
    return 1;
  } else {
    return 0;
  }
}

int ToNum(const Value *o, double &n);

int ToBool(const Value *o, bool &b);

std::string ToCString(const Value *o);

void SetIValue(Value *o, int iv);

void SetDValue(Value *o, double d);

void SetBValue(Value *o, bool b);

void SetTValue(Value *v, GCObject *o);

void SetSValue(Value *v, String *s);

String *StringAdd(StringTable *t, Value *a, Value *b);

/*
** try to convert a value to an integer, rounding according to 'mode':
** mode == 0: accepts only integral values
** mode == 1: takes the floor of the number
** mode == 2: takes the ceil of the number
*/
int ToInteger(const Value *o, const int &mode, int64_t &v);

bool ValueEqulas(const Value *a, const Value *b);

bool ValueLE(const Value *a, const Value *b);

bool ValueLT(const Value *a, const Value *b);

void FreeValue(Value *o);

}  // namespace data_render
}  // namespace core
}  // namespace weex
#endif  // CORE_DATA_RENDER_OBJECT_H
