/*
 * Copyright (C) 2015 Andy VanWagoner (thetalecrafter@gmail.com)
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "IntlNumberFormatPrototype.h"

#if ENABLE(INTL)

#include "BuiltinNames.h"
#include "Error.h"
#include "IntlNumberFormat.h"
#include "JSBoundFunction.h"
#include "JSCInlines.h"
#include "JSObjectInlines.h"

namespace JSC {

static EncodedJSValue JSC_HOST_CALL IntlNumberFormatPrototypeGetterFormat(ExecState*);
static EncodedJSValue JSC_HOST_CALL IntlNumberFormatPrototypeFuncResolvedOptions(ExecState*);

}

#include "IntlNumberFormatPrototype.lut.h"

namespace JSC {

const ClassInfo IntlNumberFormatPrototype::s_info = { "Object", &IntlNumberFormat::s_info, &numberFormatPrototypeTable, CREATE_METHOD_TABLE(IntlNumberFormatPrototype) };

/* Source for IntlNumberFormatPrototype.lut.h
@begin numberFormatPrototypeTable
  format           IntlNumberFormatPrototypeGetterFormat         DontEnum|Accessor
  resolvedOptions  IntlNumberFormatPrototypeFuncResolvedOptions  DontEnum|Function 0
@end
*/

IntlNumberFormatPrototype* IntlNumberFormatPrototype::create(VM& vm, JSGlobalObject*, Structure* structure)
{
    IntlNumberFormatPrototype* object = new (NotNull, allocateCell<IntlNumberFormatPrototype>(vm.heap)) IntlNumberFormatPrototype(vm, structure);
    object->finishCreation(vm, structure);
    return object;
}

Structure* IntlNumberFormatPrototype::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

IntlNumberFormatPrototype::IntlNumberFormatPrototype(VM& vm, Structure* structure)
    : IntlNumberFormat(vm, structure)
{
}

void IntlNumberFormatPrototype::finishCreation(VM& vm, Structure*)
{
    Base::finishCreation(vm);
}

static EncodedJSValue JSC_HOST_CALL IntlNumberFormatFuncFormatNumber(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // 11.3.4 Format Number Functions (ECMA-402 2.0)
    // 1. Let nf be the this value.
    // 2. Assert: Type(nf) is Object and nf has an [[initializedNumberFormat]] internal slot whose value  true.
    IntlNumberFormat* numberFormat = jsCast<IntlNumberFormat*>(state->thisValue());

    // 3. If value is not provided, let value be undefined.
    // 4. Let x be ToNumber(value).
    double number = state->argument(0).toNumber(state);
    // 5. ReturnIfAbrupt(x).
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 6. Return FormatNumber(nf, x).
    scope.release();
    return JSValue::encode(numberFormat->formatNumber(*state, number));
}

EncodedJSValue JSC_HOST_CALL IntlNumberFormatPrototypeGetterFormat(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 11.3.3 Intl.NumberFormat.prototype.format (ECMA-402 2.0)
    // 1. Let nf be this NumberFormat object.
    IntlNumberFormat* nf = jsDynamicCast<IntlNumberFormat*>(vm, state->thisValue());

    // FIXME: Workaround to provide compatibility with ECMA-402 1.0 call/apply patterns.
    // https://bugs.webkit.org/show_bug.cgi?id=153679
    if (!nf) {
        JSValue value = state->thisValue().get(state, vm.propertyNames->builtinNames().intlSubstituteValuePrivateName());
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        nf = jsDynamicCast<IntlNumberFormat*>(vm, value);
    }

    if (!nf)
        return JSValue::encode(throwTypeError(state, scope, ASCIILiteral("Intl.NumberFormat.prototype.format called on value that's not an object initialized as a NumberFormat")));
    
    JSBoundFunction* boundFormat = nf->boundFormat();
    // 2. If nf.[[boundFormat]] is undefined,
    if (!boundFormat) {
        JSGlobalObject* globalObject = nf->globalObject();
        // a. Let F be a new built-in function object as defined in 11.3.4.
        // b. The value of F’s length property is 1.
        JSFunction* targetObject = JSFunction::create(vm, globalObject, 1, ASCIILiteral("format"), IntlNumberFormatFuncFormatNumber, NoIntrinsic);
        // c. Let bf be BoundFunctionCreate(F, «this value»).
        boundFormat = JSBoundFunction::create(vm, state, globalObject, targetObject, nf, nullptr, 1, ASCIILiteral("format"));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        // d. Set nf.[[boundFormat]] to bf.
        nf->setBoundFormat(vm, boundFormat);
    }
    // 3. Return nf.[[boundFormat]].
    return JSValue::encode(boundFormat);
}

EncodedJSValue JSC_HOST_CALL IntlNumberFormatPrototypeFuncResolvedOptions(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 11.3.5 Intl.NumberFormat.prototype.resolvedOptions() (ECMA-402 2.0)
    IntlNumberFormat* numberFormat = jsDynamicCast<IntlNumberFormat*>(vm, state->thisValue());

    // FIXME: Workaround to provide compatibility with ECMA-402 1.0 call/apply patterns.
    // https://bugs.webkit.org/show_bug.cgi?id=153679
    if (!numberFormat) {
        JSValue value = state->thisValue().get(state, vm.propertyNames->builtinNames().intlSubstituteValuePrivateName());
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        numberFormat = jsDynamicCast<IntlNumberFormat*>(vm, value);
    }

    if (!numberFormat)
        return JSValue::encode(throwTypeError(state, scope, ASCIILiteral("Intl.NumberFormat.prototype.resolvedOptions called on value that's not an object initialized as a NumberFormat")));

    scope.release();
    return JSValue::encode(numberFormat->resolvedOptions(*state));
}

} // namespace JSC

#endif // ENABLE(INTL)
