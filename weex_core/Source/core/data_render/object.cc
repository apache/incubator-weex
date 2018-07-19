//
// Created by chad on 2018/7/19.
//

#include "core/data_render/object.h"

namespace weex {
namespace core {
namespace data_render {

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

}
}
}