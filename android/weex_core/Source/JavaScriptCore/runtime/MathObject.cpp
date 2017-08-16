/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2007-2008, 2013, 2015-2016 Apple Inc. All Rights Reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"
#include "MathObject.h"

#include "JSCInlines.h"
#include "MathCommon.h"
#include "ObjectPrototype.h"
#include <time.h>
#include <wtf/Assertions.h>
#include <wtf/MathExtras.h>
#include <wtf/Vector.h>

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(MathObject);

EncodedJSValue JSC_HOST_CALL mathProtoFuncACos(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncACosh(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncASin(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncASinh(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncATan(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncATanh(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncATan2(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncCbrt(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncCeil(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncClz32(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncCos(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncCosh(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncExp(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncExpm1(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncFround(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncHypot(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncLog(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncLog1p(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncLog10(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncLog2(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncMax(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncMin(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncPow(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncRandom(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncRound(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncSign(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncSin(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncSinh(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncSqrt(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncTan(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncTanh(ExecState*);
EncodedJSValue JSC_HOST_CALL mathProtoFuncIMul(ExecState*);

const ClassInfo MathObject::s_info = { "Math", &Base::s_info, 0, CREATE_METHOD_TABLE(MathObject) };

MathObject::MathObject(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

void MathObject::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));

    putDirectWithoutTransition(vm, Identifier::fromString(&vm, "E"), jsNumber(exp(1.0)), DontDelete | DontEnum | ReadOnly);
    putDirectWithoutTransition(vm, Identifier::fromString(&vm, "LN2"), jsNumber(log(2.0)), DontDelete | DontEnum | ReadOnly);
    putDirectWithoutTransition(vm, Identifier::fromString(&vm, "LN10"), jsNumber(log(10.0)), DontDelete | DontEnum | ReadOnly);
    putDirectWithoutTransition(vm, Identifier::fromString(&vm, "LOG2E"), jsNumber(1.0 / log(2.0)), DontDelete | DontEnum | ReadOnly);
    putDirectWithoutTransition(vm, Identifier::fromString(&vm, "LOG10E"), jsNumber(0.4342944819032518), DontDelete | DontEnum | ReadOnly);
    putDirectWithoutTransition(vm, Identifier::fromString(&vm, "PI"), jsNumber(piDouble), DontDelete | DontEnum | ReadOnly);
    putDirectWithoutTransition(vm, Identifier::fromString(&vm, "SQRT1_2"), jsNumber(sqrt(0.5)), DontDelete | DontEnum | ReadOnly);
    putDirectWithoutTransition(vm, Identifier::fromString(&vm, "SQRT2"), jsNumber(sqrt(2.0)), DontDelete | DontEnum | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, "Math"), DontEnum | ReadOnly);

    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "abs"), 1, mathProtoFuncAbs, AbsIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "acos"), 1, mathProtoFuncACos, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "asin"), 1, mathProtoFuncASin, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "atan"), 1, mathProtoFuncATan, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "acosh"), 1, mathProtoFuncACosh, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "asinh"), 1, mathProtoFuncASinh, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "atanh"), 1, mathProtoFuncATanh, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "atan2"), 2, mathProtoFuncATan2, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "cbrt"), 1, mathProtoFuncCbrt, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "ceil"), 1, mathProtoFuncCeil, CeilIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "clz32"), 1, mathProtoFuncClz32, Clz32Intrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "cos"), 1, mathProtoFuncCos, CosIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "cosh"), 1, mathProtoFuncCosh, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "exp"), 1, mathProtoFuncExp, ExpIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "expm1"), 1, mathProtoFuncExpm1, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "floor"), 1, mathProtoFuncFloor, FloorIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "fround"), 1, mathProtoFuncFround, FRoundIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "hypot"), 2, mathProtoFuncHypot, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "log"), 1, mathProtoFuncLog, LogIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "log10"), 1, mathProtoFuncLog10, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "log1p"), 1, mathProtoFuncLog1p, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "log2"), 1, mathProtoFuncLog2, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "max"), 2, mathProtoFuncMax, MaxIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "min"), 2, mathProtoFuncMin, MinIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "pow"), 2, mathProtoFuncPow, PowIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "random"), 0, mathProtoFuncRandom, RandomIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "round"), 1, mathProtoFuncRound, RoundIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "sign"), 1, mathProtoFuncSign, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "sin"), 1, mathProtoFuncSin, SinIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "sinh"), 1, mathProtoFuncSinh, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "sqrt"), 1, mathProtoFuncSqrt, SqrtIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "tan"), 1, mathProtoFuncTan, TanIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "tanh"), 1, mathProtoFuncTanh, NoIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "trunc"), 1, mathProtoFuncTrunc, TruncIntrinsic, DontEnum);
    putDirectNativeFunctionWithoutTransition(vm, globalObject, Identifier::fromString(&vm, "imul"), 2, mathProtoFuncIMul, IMulIntrinsic, DontEnum);
}

// ------------------------------ Functions --------------------------------

EncodedJSValue JSC_HOST_CALL mathProtoFuncAbs(ExecState* exec)
{
    return JSValue::encode(jsNumber(fabs(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncACos(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(acos(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncASin(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(asin(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncATan(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(atan(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncATan2(ExecState* exec)
{
    double arg0 = exec->argument(0).toNumber(exec);
    double arg1 = exec->argument(1).toNumber(exec);
    return JSValue::encode(jsDoubleNumber(atan2(arg0, arg1)));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncCeil(ExecState* exec)
{
    return JSValue::encode(jsNumber(ceil(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncClz32(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    uint32_t value = exec->argument(0).toUInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(JSValue(clz32(value)));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncCos(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(cos(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncExp(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(exp(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncFloor(ExecState* exec)
{
    return JSValue::encode(jsNumber(floor(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncHypot(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    unsigned argsCount = exec->argumentCount();
    double max = 0;
    Vector<double, 8> args;
    args.reserveInitialCapacity(argsCount);
    for (unsigned i = 0; i < argsCount; ++i) {
        args.uncheckedAppend(exec->uncheckedArgument(i).toNumber(exec));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        if (std::isinf(args[i]))
            return JSValue::encode(jsDoubleNumber(+std::numeric_limits<double>::infinity()));
        max = std::max(fabs(args[i]), max);
    }
    if (!max)
        max = 1;
    // Kahan summation algorithm significantly reduces the numerical error in the total obtained.
    double sum = 0;
    double compensation = 0;
    for (double argument : args) {
        double scaledArgument = argument / max;
        double summand = scaledArgument * scaledArgument - compensation;
        double preliminary = sum + summand;
        compensation = (preliminary - sum) - summand;
        sum = preliminary;
    }
    return JSValue::encode(jsDoubleNumber(sqrt(sum) * max));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncLog(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(log(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncMax(ExecState* exec)
{
    unsigned argsCount = exec->argumentCount();
    double result = -std::numeric_limits<double>::infinity();
    for (unsigned k = 0; k < argsCount; ++k) {
        double val = exec->uncheckedArgument(k).toNumber(exec);
        if (std::isnan(val)) {
            result = PNaN;
        } else if (val > result || (!val && !result && !std::signbit(val)))
            result = val;
    }
    return JSValue::encode(jsNumber(result));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncMin(ExecState* exec)
{
    unsigned argsCount = exec->argumentCount();
    double result = +std::numeric_limits<double>::infinity();
    for (unsigned k = 0; k < argsCount; ++k) {
        double val = exec->uncheckedArgument(k).toNumber(exec);
        if (std::isnan(val)) {
            result = PNaN;
        } else if (val < result || (!val && !result && std::signbit(val)))
            result = val;
    }
    return JSValue::encode(jsNumber(result));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncPow(ExecState* exec)
{
    // ECMA 15.8.2.1.13

    double arg = exec->argument(0).toNumber(exec);
    double arg2 = exec->argument(1).toNumber(exec);

    return JSValue::encode(JSValue(operationMathPow(arg, arg2)));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncRandom(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(exec->lexicalGlobalObject()->weakRandomNumber()));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncRound(ExecState* exec)
{
    return JSValue::encode(jsNumber(jsRound(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncSign(ExecState* exec)
{
    double arg = exec->argument(0).toNumber(exec);
    if (std::isnan(arg))
        return JSValue::encode(jsNaN());
    if (!arg)
        return JSValue::encode(std::signbit(arg) ? jsNumber(-0.0) : jsNumber(0));
    return JSValue::encode(jsNumber(std::signbit(arg) ? -1 : 1));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncSin(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(sin(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncSqrt(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(sqrt(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncTan(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(tan(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncIMul(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    int32_t left = exec->argument(0).toInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    int32_t right = exec->argument(1).toInt32(exec);
    return JSValue::encode(jsNumber(left * right));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncACosh(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(acosh(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncASinh(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(asinh(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncATanh(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(atanh(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncCbrt(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(cbrt(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncCosh(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(cosh(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncExpm1(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(expm1(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncFround(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(static_cast<float>(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncLog1p(ExecState* exec)
{
    double value = exec->argument(0).toNumber(exec);
    if (value == 0)
        return JSValue::encode(jsDoubleNumber(value));
    return JSValue::encode(jsDoubleNumber(log1p(value)));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncLog10(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(log10(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncLog2(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(log2(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncSinh(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(sinh(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncTanh(ExecState* exec)
{
    return JSValue::encode(jsDoubleNumber(tanh(exec->argument(0).toNumber(exec))));
}

EncodedJSValue JSC_HOST_CALL mathProtoFuncTrunc(ExecState*exec)
{
    return JSValue::encode(jsNumber(exec->argument(0).toIntegerPreserveNaN(exec)));
}

} // namespace JSC
