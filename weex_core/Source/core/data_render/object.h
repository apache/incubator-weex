//
// Created by chad on 2018/7/18.
//

#ifndef DATA_RENDER_OBJECT_H
#define DATA_RENDER_OBJECT_H

//#include <math.h>
#include "vm.h"
#include "string_table.h"

#define CommonHeader GCObject* gc


#define LOGE(...) ((void)0)

#define INT_OP(op, v1, v2) CAST_U2S((CAST_S2U(v1)) op CAST_S2U(v2))
#define NUM_OP(op, d1, d2) ((d1) op (d2))
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

    size_t sizearray;  /* size of 'array' array */
    size_t sizenode;
    Value *array;  /* array part */
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

    enum Type {
        NIL, INT, NUMBER, BOOL, FUNC, CFUNC, STRING, CPTR, TABLE
    };

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
            default:
                break;
        }
        return false;
    }
};

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

inline bool NumEq(const double &d1, const double &d2) {
    return d1 == d2;
}

inline bool NumLT(const double &d1, const double &d2) {
    return d1 < d2;
}

inline int Number2Int(const double &n, int64_t &p) {
    if (n >= MININTEGER && n < -MININTEGER) {
        p = n;
        return 1;
    }
    return 0;
}


inline double NumUnm(const double &d) {
    return -d;
}

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

inline bool IsNil(const Value *o) { return nullptr == o || Value::Type::NIL == o->type; }

inline void SetNil(Value *o) { if (!o) { o->type = Value::Type::NIL;} }

inline int IntMod(const int &a, const int &b) {
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

inline bool IsNumber(const Value *o) { return Value::Type::NUMBER == o->type; }

inline bool IsBool(const Value *o) { return Value::Type::BOOL == o->type; }

inline bool IsTable(const Value *o) { return Value::Type::TABLE == o->type; }

inline bool IsString(const Value *o) { return Value::Type::STRING == o->type; }

inline int64_t IntValue(const Value *o) { return o->i; }

inline double NumValue(const Value *o) { return o->n; }

inline bool BoolValue(const Value *o) { return (Value::Type::BOOL == o->type) ? o->b : false; }

inline String* StringValue(const Value *o) { return IsString(o) ? o->str : NULL; }

inline char* CStringValue(const Value *o) { return (IsString(o) && NULL != o->str) ? o->str->c_str() : NULL; }

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
    return IsNumber(o) ? (n = NumValue(o), 1) : ToNumber_(o, n);
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

inline void SetIValue(Value *o, int iv) {
    o->type = Value::Type::INT;
    o->i = iv;
}

inline void SetDValue(Value *o, double d) {
    o->type = Value::Type::NUMBER;
    o->n = d;
}

inline void SetBValue(Value *o, bool b) {
    o->type = Value::Type::BOOL;
    o->b = b;
}

inline void SetTabValue(Value *v, GCObject *o) {
    v->type = Value::Type::TABLE;
    v->gc = o;
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
}
}
}
#endif //DATA_RENDER_OBJECT_H
