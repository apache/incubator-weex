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
#include <sstream>
#include <map>
#include <memory>
#include "base/string_util.h"
#include "core/data_render/object.h"
#include "core/data_render/string_table.h"
#include "core/data_render/vm.h"

#define CommonHeader GCObject *next_; int ref_;

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
    
class FuncClosure;

class String;

class StringTable;

class Value;
    
class ValueRef;
    
class Variables;
    
typedef struct GCObject {
    CommonHeader;
    inline int decrement() { return --ref_; }
    inline int increment() { return ++ref_; }

} GCObject;

typedef Value (*CFunction)(ExecState *);

#define ttype(o)            ((o)->type)
#define iscollectable(o)    (ttype(o) >= Value::Type::ARRAY)

inline void GCRelease(Value *o);
inline void GCRetain(Value *o);

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
    struct Value *var;
  };

  struct Value *ref {nullptr};

  enum Type { NIL, INT, NUMBER, BOOL, FUNC, CFUNC, STRING, CPTR, VALUE_REF, ARRAY, TABLE, CLASS_DESC, CLASS_INST, FUNC_INST };

  Type type;

  Value() : type(NIL) {}
    
  Value(int value) : i(value), type(INT) {}

  Value(int64_t value) : i(value), type(INT) {}

  Value(double value) : n(value), type(NUMBER) {}

  explicit Value(bool value) : b(value), type(BOOL) {}

  Value(String *value) : str(value), type(STRING) {}
    
  explicit Value(FuncState *func) : f(func), type(FUNC) {}
    
  Value(const Value &value) {
    type = value.type;
    switch (type) {
      case INT:i = value.i;
        break;
      case NUMBER:n = value.n;
        break;
      case BOOL:b = value.b;
        break;
      case STRING:str = value.str;
        break;
      case FUNC:f = value.f;
        break;
      case CFUNC:cf = value.cf;
        break;
      case CPTR:cptr = value.cptr;
        break;
      case VALUE_REF:var = value.var;
        break;
      case TABLE:
      case ARRAY:
      case CLASS_DESC:
      case CLASS_INST:
      case FUNC_INST:
        gc = value.gc;
        if(gc)
          gc->increment();
        break;
      default:break;
    }
  }

  inline Value operator=(Value value) {
      GCRelease(this);
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
              break;
          case VALUE_REF:
              var = value.var;
              break;
          case TABLE:
          case ARRAY:
          case CLASS_DESC:
          case CLASS_INST:
          case FUNC_INST:
              gc = value.gc;
              if(gc)
                gc->increment();
              break;
          default:break;
      }
      return *this;
  }
  friend bool operator==(const Value &left, const Value &right) {
    if (left.type != right.type) return false;
    switch (left.type) {
      case NIL:return true;
      case INT:return left.i == right.i;
      case NUMBER:return fabs(left.n - right.n) < 0.000001;
      case BOOL:return left.b == right.b;
      case STRING:return left.str == right.str;
      case FUNC:return left.f == right.f;
      case CFUNC:return left.cf == right.cf;
      case CPTR:return left.cptr == right.cptr;
      case ARRAY:
      case TABLE:
      case CLASS_DESC:
      case CLASS_INST:
      case FUNC_INST:
            return left.gc == right.gc;
      default:break;
    }
    return false;
  }
    friend bool operator!=(const Value &left, const Value &right) {
        return !(left == right);
    }
};
    
typedef struct Array {
    CommonHeader;
    std::vector<Value> items;
    Array() : items() {}
    
} Array;

typedef struct Table {
    CommonHeader;
    std::unordered_map<std::string, Value> map;
    Table() : map() {}

} Table;
        
class Variables {
public:
    Value *Find(int index);
    int IndexOf(const std::string& name);
    int Add(Value value);
    int Add(const std::string& name, Value value);
    int Set(const std::string& name, Value value);
    inline size_t size() {return values_.size();}
    inline const std::map<std::string, int>& map() { return map_; }    
private:
    std::map<std::string, int> map_;
    std::vector<Value> values_;
};

struct ClassDescriptor;
    
typedef struct ClassDescriptor {
    CommonHeader;
    ClassDescriptor *p_super_{nullptr};
    std::unique_ptr<Variables> statics_;
    std::unique_ptr<Variables> funcs_;
    ClassDescriptor(ClassDescriptor *p_super) : p_super_(p_super), funcs_(new Variables), statics_(new Variables) {}
} ClassDescriptor;

typedef struct ClassInstance {
    CommonHeader;
    ClassInstance *p_super_{nullptr};
    ClassDescriptor *p_desc_{nullptr};
    std::unique_ptr<Variables> vars_;
    ClassInstance(ClassDescriptor *p_desc) : p_desc_(p_desc), vars_(new Variables) {}
    
} ClassInstance;
    
typedef struct FuncInstance {
    CommonHeader;
    FuncState *func_{nullptr};
    std::vector<FuncClosure *> closures_;
    FuncInstance(FuncState *func) : func_(func), closures_() { }
    
} FuncInstance;

/*
** try to convert a value to an integer, rounding according to 'mode':
** mode == 0: accepts only integral values
** mode == 1: takes the floor of the number
** mode == 2: takes the ceil of the number
*/
int ToInteger(const Value *o, const int &mode, int64_t &v);

bool ValueEquals(const Value *a, const Value *b);

bool ObjectEquals(const Value *a, const Value *b);
    
bool ValueStrictEquals(const Value *a, const Value *b);
    
bool ValueLT(const Value *a, const Value *b);
    
bool ValueLTE(const Value *a, const Value *b);
    
bool ValueGT(const Value *a, const Value *b);

bool ValueGTE(const Value *a, const Value *b);
    
bool ValueAND(const Value *a, const Value *b);
    
bool ValueOR(const Value *a, const Value *b);

inline double NumPow(const double &d1, const double &d2) {
  return MATH_OP(pow)(d1, d2);
}

inline double NumIDiv(const double &d1, const double &d2) {
  return MATH_OP(floor)(NUM_OP(/, d1, d2));
}

inline double NumMod(const double &d1, const double &d2) {
  double ret = MATH_OP(fmod)(d1, d2);
  if (ret * d2 < 0) ret += d2;
  return ret;
}

inline bool NumEq(const double &d1, const double &d2) { return d1 == d2; }

inline bool NumLT(const double &d1, const double &d2) {
  return d1 < d2;
}

inline bool NumLTE(const double &d1, const double &d2) {
    return d1 <= d2;
}

inline bool NumGT(const double &d1, const double &d2) {
    return d1 > d2;
}
    
inline bool NumGTE(const double &d1, const double &d2) {
    return d1 >= d2;
}
    
inline int Number2Int(const double &n, int64_t &p) {
  if (n >= MININTEGER && n < -MININTEGER) {
    p = n;
    return 1;
  }
  return 0;
}

inline double NumUnm(const double &d) { return -d; }

inline int64_t ShiftLeft(const int64_t &a, const int64_t &b) {
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

inline bool IsInt(const Value *o) { return Value::Type::INT == o->type; }
    
inline bool IsFunction(const Value *o) { return Value::Type::FUNC == o->type || Value::Type::CFUNC == o->type || Value::Type::FUNC_INST == o->type; }
    
inline bool IsPrototypeFunction(const Value *o) { return Value::Type::FUNC == o->type; }
    
inline bool IsValueRef(const Value *o) { return Value::Type::VALUE_REF == o->type; }
    
inline bool IsNil(const Value *o) {
  return nullptr == o || Value::Type::NIL == o->type;
}
    
inline void GCRelease(Value *o) {
    if (iscollectable(o) && o->gc) {
        o->gc->decrement();
    }
}
    
inline void GCRetain(Value *o) {
    if (iscollectable(o) && o->gc) {
        o->gc->increment();
    }
}

inline void SetNil(Value *o) {
    if (nullptr != o) {
        GCRelease(o);
        o->type = Value::Type::NIL;
    }
}
    
inline void SetValueRef(Value *o, Value *src) {
    if (nullptr != o) {
        GCRelease(o);
        o->type = Value::Type::VALUE_REF;
        o->var = src;
    }
}

inline int IntMod(const int &a, const int &b) {
  if (CAST_S2U(b) + 1u <= 1u) {
    if (b == 0) {
      //LOGE("Error ValueMod Values[%d, %d]", a, b);
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
    
inline bool IsNumber(const Value *o) { return Value::Type::NUMBER == o->type; }

inline bool IsCptr(const Value *o) { return Value::Type::CPTR == o->type; }

inline bool IsBool(const Value *o) { return Value::Type::BOOL == o->type; }

inline bool IsTable(const Value *o) { return Value::Type::TABLE == o->type; }

inline bool IsArray(const Value *o) { return Value::Type::ARRAY == o->type; }
    
inline bool IsString(const Value *o) { return nullptr != o && Value::Type::STRING == o->type; }
    
inline bool IsClass(const Value *o) { return nullptr != o && Value::Type::CLASS_DESC == o->type; }
    
inline bool IsClassInstance(const Value *o) { return nullptr != o && Value::Type::CLASS_INST == o->type; }
    
inline bool IsFuncInstance(const Value *o) { return nullptr != o && Value::Type::FUNC_INST == o->type; }
    
inline int64_t IntValue(const Value *o) { return o->i; }

inline double NumValue(const Value *o) { return o->n; }

inline bool BoolValue(const Value *o) {
  return (Value::Type::BOOL == o->type) ? o->b : false;
}

inline String *StringValue(const Value *o) { return IsString(o) ? o->str : nullptr; }

inline char *CStringValue(const Value *o) {
  return (IsString(o) && nullptr != o->str) ? o->str->c_str() : nullptr;
}
    
template <typename T>
inline T* ValueTo(const Value *o) {
    return reinterpret_cast<T*>(o->gc);
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
    
inline int ToNum(const Value *o, double &n) {
  return IsNumber(o) ? (static_cast<void>(n = NumValue(o)), 1) : ToNumber_(o, n);
}

inline void SetIValue(Value *o, int iv) {
    GCRelease(o);
    o->type = Value::Type::INT;
    o->i = iv;
}
    
    
void SetRefValue(Value *o);

inline void SetDValue(Value *o, double d) {
    GCRelease(o);
    o->type = Value::Type::NUMBER;
    o->n = d;
}

inline void SetBValue(Value *o, bool b) {
    GCRelease(o);
    o->type = Value::Type::BOOL;
    o->b = b;
}

inline void SetTValue(Value *v, GCObject *o) {
    GCRelease(v);
    v->type = Value::Type::TABLE;
    v->gc = o;
    GCRetain(v);
}
    
inline void SetAValue(Value *v, GCObject *o) {
    GCRelease(v);
    v->type = Value::Type::ARRAY;
    v->gc = o;
    GCRetain(v);
}
    
inline void SetCDValue(Value *v, GCObject *o) {
    GCRelease(v);
    v->type = Value::Type::CLASS_DESC;
    v->gc = o;
    GCRetain(v);
}

inline void SetCIValue(Value *v, GCObject *o) {
    GCRelease(v);
    v->type = Value::Type::CLASS_INST;
    v->gc = o;
    GCRetain(v);
}

inline void SetGCFuncValue(Value *v, GCObject *o) {
    GCRelease(v);
    v->type = Value::Type::FUNC_INST;
    v->gc = o;
    GCRetain(v);
}

inline void SetSValue(Value *v, String *s) {
    GCRelease(v);
    v->type = Value::Type::STRING;
    v->str = s;
}

inline std::string ToCString(const Value *o) {
  switch (o->type) {

    case Value::Type::INT: {
      std::stringstream ss;
      ss << IntValue(o);
      return ss.str();
    }

    case Value::Type::NUMBER: {
      std::stringstream ss;
      ss << NumValue(o);
      return ss.str();
    }

    case Value::Type::BOOL: {
      std::stringstream ss;
      ss << BoolValue(o);
      return ss.str();
    }

    case Value::Type::STRING: {
      return CStringValue(o);
    }

    default:return "";
  }
}

inline String *StringAdd(StringTable *t, Value *a, Value *b) {
  std::string str;
  if (IsString(a)) {
    str = a->str->c_str();
    return t->StringFromUTF8(str += ToCString(b));
  } else {
    str = ToCString(a);
    return t->StringFromUTF8(str += b->str->c_str());
  }
}

inline int ToBool(const Value *o, bool &b) {
    double d1;
    if (Value::Type::BOOL == o->type) {
        b = BoolValue(o);
    }
    else if (Value::Type::INT == o->type) {
        b = IntValue(o);
    }
    else if (Value::Type::NUMBER == o->type) {
        b = NumValue(o);
    }
    else if (ttype(o) == Value::Type::STRING) {
        b = (o->str && o->str->length() > 0) ? true : false;
    }
    else if (Value::Type::VALUE_REF == o->type) {
        return ToBool(o->var, b);
    }
    else if (ToNum(o, d1)) {
        b = d1;
    }
    else if (Value::Type::FUNC == o->type) {
        b = o->f ? true : false;
    }
    else {
        b = ttype(o) == Value::Type::NIL ? false : true;
    }
    return 1;
}

inline std::string ToString(const Value *o) {
    switch (o->type) {
        case Value::Type::BOOL:
            return weex::base::to_string(o->b);
        case Value::Type::INT:
            return weex::base::to_string(o->n);
        case Value::Type::NUMBER:
            return weex::base::to_string(o->n);
        case Value::Type::STRING:
            return o->str->c_str();
        case Value::Type::NIL:
            return "[nil]";
        default:
            return "[not support]";
    }
    return std::string();
}

inline void ArrayCopyFrom(Value &src, Value &dest, int start, int end) {
    Array *st = ValueTo<Array>(&src);
    Array *dt = ValueTo<Array>(&dest);
    auto iter_start = st->items.begin() + start;
    auto iter_end = end > 0 ? st->items.begin() + end : st->items.end();
    for (auto iter = iter_start; iter != iter_end; iter++) {
        GCRetain(&*iter);
    }
    dt->items.insert(dt->items.end(), iter_start, iter_end);
}

inline void ArrayCopy(Value &src, Value &dest) {
    ArrayCopyFrom(src, dest, 0, 0);
}

inline void TableCopy(Value &src, Value &dest) {
    Table *st = ValueTo<Table>(&src);
    Table *dt = ValueTo<Table>(&dest);
    for (auto iter = st->map.begin(); iter != st->map.end(); iter++) {
        GCRetain(&iter->second);
        dt->map[iter->first] = iter->second;
    }
}

inline void TableMapAddAll(Table *src, Table *dest) {
  for (auto iter = src->map.begin(); iter != src->map.end(); iter++) {
    GCRetain(&iter->second);
    dest->map[iter->first] = iter->second;
  }
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
#endif  // CORE_DATA_RENDER_OBJECT_H
