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
#include "IntlCollatorPrototype.h"

#if ENABLE(INTL)

#include "Error.h"
#include "IntlCollator.h"
#include "JSBoundFunction.h"
#include "JSCInlines.h"

namespace JSC {

static EncodedJSValue JSC_HOST_CALL IntlCollatorPrototypeGetterCompare(ExecState*);
static EncodedJSValue JSC_HOST_CALL IntlCollatorPrototypeFuncResolvedOptions(ExecState*);

}

#include "IntlCollatorPrototype.lut.h"

namespace JSC {

const ClassInfo IntlCollatorPrototype::s_info = { "Object", &IntlCollator::s_info, &collatorPrototypeTable, CREATE_METHOD_TABLE(IntlCollatorPrototype) };

/* Source for IntlCollatorPrototype.lut.h
@begin collatorPrototypeTable
  compare          IntlCollatorPrototypeGetterCompare        DontEnum|Accessor
  resolvedOptions  IntlCollatorPrototypeFuncResolvedOptions  DontEnum|Function 0
@end
*/

IntlCollatorPrototype* IntlCollatorPrototype::create(VM& vm, JSGlobalObject*, Structure* structure)
{
    IntlCollatorPrototype* object = new (NotNull, allocateCell<IntlCollatorPrototype>(vm.heap)) IntlCollatorPrototype(vm, structure);
    object->finishCreation(vm);
    return object;
}

Structure* IntlCollatorPrototype::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

IntlCollatorPrototype::IntlCollatorPrototype(VM& vm, Structure* structure)
    : IntlCollator(vm, structure)
{
}

void IntlCollatorPrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
}

static EncodedJSValue JSC_HOST_CALL IntlCollatorFuncCompare(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // 10.3.4 Collator Compare Functions (ECMA-402 2.0)
    // 1. Let collator be the this value.
    // 2. Assert: Type(collator) is Object and collator has an [[initializedCollator]] internal slot whose value is true.
    IntlCollator* collator = jsCast<IntlCollator*>(state->thisValue());

    // 3. If x is not provided, let x be undefined.
    // 4. If y is not provided, let y be undefined.
    // 5. Let X be ToString(x).
    JSString* x = state->argument(0).toString(state);
    // 6. ReturnIfAbrupt(X).
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 7. Let Y be ToString(y).
    JSString* y = state->argument(1).toString(state);
    // 8. ReturnIfAbrupt(Y).
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 9. Return CompareStrings(collator, X, Y).
    auto xViewWithString = x->viewWithUnderlyingString(*state);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    auto yViewWithString = y->viewWithUnderlyingString(*state);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(collator->compareStrings(*state, xViewWithString.view, yViewWithString.view));
}

EncodedJSValue JSC_HOST_CALL IntlCollatorPrototypeGetterCompare(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 10.3.3 Intl.Collator.prototype.compare (ECMA-402 2.0)
    // 1. Let collator be this Collator object.
    IntlCollator* collator = jsDynamicCast<IntlCollator*>(vm, state->thisValue());
    if (!collator)
        return JSValue::encode(throwTypeError(state, scope, ASCIILiteral("Intl.Collator.prototype.compare called on value that's not an object initialized as a Collator")));

    JSBoundFunction* boundCompare = collator->boundCompare();
    // 2. If collator.[[boundCompare]] is undefined,
    if (!boundCompare) {
        JSGlobalObject* globalObject = collator->globalObject();
        // a. Let F be a new built-in function object as defined in 11.3.4.
        // b. The value of F’s length property is 2.
        JSFunction* targetObject = JSFunction::create(vm, globalObject, 2, ASCIILiteral("compare"), IntlCollatorFuncCompare, NoIntrinsic);

        // c. Let bc be BoundFunctionCreate(F, «this value»).
        boundCompare = JSBoundFunction::create(vm, state, globalObject, targetObject, collator, nullptr, 2, ASCIILiteral("compare"));
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        // d. Set collator.[[boundCompare]] to bc.
        collator->setBoundCompare(vm, boundCompare);
    }
    // 3. Return collator.[[boundCompare]].
    return JSValue::encode(boundCompare);
}

EncodedJSValue JSC_HOST_CALL IntlCollatorPrototypeFuncResolvedOptions(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 10.3.5 Intl.Collator.prototype.resolvedOptions() (ECMA-402 2.0)
    IntlCollator* collator = jsDynamicCast<IntlCollator*>(vm, state->thisValue());
    if (!collator)
        return JSValue::encode(throwTypeError(state, scope, ASCIILiteral("Intl.Collator.prototype.resolvedOptions called on value that's not an object initialized as a Collator")));

    scope.release();
    return JSValue::encode(collator->resolvedOptions(*state));
}

} // namespace JSC

#endif // ENABLE(INTL)
