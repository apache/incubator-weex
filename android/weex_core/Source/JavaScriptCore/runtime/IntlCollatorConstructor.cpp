/*
 * Copyright (C) 2015 Andy VanWagoner (thetalecrafter@gmail.com)
 * Copyright (C) 2015 Sukolsak Sakshuwong (sukolsak@gmail.com)
 * Copyright (C) 2016 Apple Inc. All Rights Reserved.
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
#include "IntlCollatorConstructor.h"

#if ENABLE(INTL)

#include "Error.h"
#include "IntlCollator.h"
#include "IntlCollatorPrototype.h"
#include "IntlObject.h"
#include "JSCInlines.h"
#include "Lookup.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(IntlCollatorConstructor);

static EncodedJSValue JSC_HOST_CALL IntlCollatorConstructorFuncSupportedLocalesOf(ExecState*);

}

#include "IntlCollatorConstructor.lut.h"

namespace JSC {

const ClassInfo IntlCollatorConstructor::s_info = { "Function", &InternalFunction::s_info, &collatorConstructorTable, CREATE_METHOD_TABLE(IntlCollatorConstructor) };

/* Source for IntlCollatorConstructor.lut.h
@begin collatorConstructorTable
  supportedLocalesOf             IntlCollatorConstructorFuncSupportedLocalesOf             DontEnum|Function 1
@end
*/

IntlCollatorConstructor* IntlCollatorConstructor::create(VM& vm, Structure* structure, IntlCollatorPrototype* collatorPrototype, Structure* collatorStructure)
{
    IntlCollatorConstructor* constructor = new (NotNull, allocateCell<IntlCollatorConstructor>(vm.heap)) IntlCollatorConstructor(vm, structure);
    constructor->finishCreation(vm, collatorPrototype, collatorStructure);
    return constructor;
}

Structure* IntlCollatorConstructor::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

IntlCollatorConstructor::IntlCollatorConstructor(VM& vm, Structure* structure)
    : InternalFunction(vm, structure)
{
}

void IntlCollatorConstructor::finishCreation(VM& vm, IntlCollatorPrototype* collatorPrototype, Structure* collatorStructure)
{
    Base::finishCreation(vm, ASCIILiteral("Collator"));
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, collatorPrototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(0), ReadOnly | DontEnum | DontDelete);
    m_collatorStructure.set(vm, this, collatorStructure);
}

static EncodedJSValue JSC_HOST_CALL constructIntlCollator(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // 10.1.2 Intl.Collator ([locales [, options]]) (ECMA-402 2.0)
    // 1. If NewTarget is undefined, let newTarget be the active function object, else let newTarget be NewTarget.
    // 2. Let collator be OrdinaryCreateFromConstructor(newTarget, %CollatorPrototype%).
    // 3. ReturnIfAbrupt(collator).
    Structure* structure = InternalFunction::createSubclassStructure(state, state->newTarget(), jsCast<IntlCollatorConstructor*>(state->jsCallee())->collatorStructure());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    IntlCollator* collator = IntlCollator::create(vm, structure);
    ASSERT(collator);

    // 4. Return InitializeCollator(collator, locales, options).
    scope.release();
    collator->initializeCollator(*state, state->argument(0), state->argument(1));
    return JSValue::encode(collator);
}

static EncodedJSValue JSC_HOST_CALL callIntlCollator(ExecState* state)
{
    // 10.1.2 Intl.Collator ([locales [, options]]) (ECMA-402 2.0)
    // 1. If NewTarget is undefined, let newTarget be the active function object, else let newTarget be NewTarget.
    // NewTarget is always undefined when called as a function.

    VM& vm = state->vm();
    IntlCollatorConstructor* callee = jsCast<IntlCollatorConstructor*>(state->jsCallee());

    // FIXME: Collator does not get the workaround for ECMA-402 1.0 compatibility.
    // https://bugs.webkit.org/show_bug.cgi?id=153679

    // 2. Let collator be OrdinaryCreateFromConstructor(newTarget, %CollatorPrototype%).
    // 3. ReturnIfAbrupt(collator).
    IntlCollator* collator = IntlCollator::create(vm, callee->collatorStructure());
    ASSERT(collator);

    // 4. Return InitializeCollator(collator, locales, options).
    collator->initializeCollator(*state, state->argument(0), state->argument(1));
    return JSValue::encode(collator);
}

ConstructType IntlCollatorConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructIntlCollator;
    return ConstructType::Host;
}

CallType IntlCollatorConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callIntlCollator;
    return CallType::Host;
}

EncodedJSValue JSC_HOST_CALL IntlCollatorConstructorFuncSupportedLocalesOf(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // 10.2.2 Intl.Collator.supportedLocalesOf(locales [, options]) (ECMA-402 2.0)

    // 1. Let requestedLocales be CanonicalizeLocaleList(locales).
    Vector<String> requestedLocales = canonicalizeLocaleList(*state, state->argument(0));

    // 2. ReturnIfAbrupt(requestedLocales).
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 3. Return SupportedLocales(%Collator%.[[availableLocales]], requestedLocales, options).
    JSGlobalObject* globalObject = state->jsCallee()->globalObject();
    scope.release();
    return JSValue::encode(supportedLocales(*state, globalObject->intlCollatorAvailableLocales(), requestedLocales, state->argument(1)));
}

void IntlCollatorConstructor::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    IntlCollatorConstructor* thisObject = jsCast<IntlCollatorConstructor*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_collatorStructure);
}

} // namespace JSC

#endif // ENABLE(INTL)
