/*
 * Copyright (C) 2015 Andy VanWagoner (thetalecrafter@gmail.com)
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
#include "IntlNumberFormatConstructor.h"

#if ENABLE(INTL)

#include "Error.h"
#include "IntlNumberFormat.h"
#include "IntlNumberFormatPrototype.h"
#include "IntlObject.h"
#include "IntlObjectInlines.h"
#include "JSCInlines.h"
#include "Lookup.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(IntlNumberFormatConstructor);

static EncodedJSValue JSC_HOST_CALL IntlNumberFormatConstructorFuncSupportedLocalesOf(ExecState*);

}

#include "IntlNumberFormatConstructor.lut.h"

namespace JSC {

const ClassInfo IntlNumberFormatConstructor::s_info = { "Function", &Base::s_info, &numberFormatConstructorTable, CREATE_METHOD_TABLE(IntlNumberFormatConstructor) };

/* Source for IntlNumberFormatConstructor.lut.h
@begin numberFormatConstructorTable
  supportedLocalesOf             IntlNumberFormatConstructorFuncSupportedLocalesOf             DontEnum|Function 1
@end
*/

IntlNumberFormatConstructor* IntlNumberFormatConstructor::create(VM& vm, Structure* structure, IntlNumberFormatPrototype* numberFormatPrototype, Structure* numberFormatStructure)
{
    IntlNumberFormatConstructor* constructor = new (NotNull, allocateCell<IntlNumberFormatConstructor>(vm.heap)) IntlNumberFormatConstructor(vm, structure);
    constructor->finishCreation(vm, numberFormatPrototype, numberFormatStructure);
    return constructor;
}

Structure* IntlNumberFormatConstructor::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

IntlNumberFormatConstructor::IntlNumberFormatConstructor(VM& vm, Structure* structure)
    : InternalFunction(vm, structure)
{
}

void IntlNumberFormatConstructor::finishCreation(VM& vm, IntlNumberFormatPrototype* numberFormatPrototype, Structure* numberFormatStructure)
{
    Base::finishCreation(vm, ASCIILiteral("NumberFormat"));
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, numberFormatPrototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(0), ReadOnly | DontEnum | DontDelete);
    m_numberFormatStructure.set(vm, this, numberFormatStructure);
}

static EncodedJSValue JSC_HOST_CALL constructIntlNumberFormat(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // 11.1.2 Intl.NumberFormat ([locales [, options]]) (ECMA-402 2.0)
    // 1. If NewTarget is undefined, let newTarget be the active function object, else let newTarget be NewTarget.
    // 2. Let numberFormat be OrdinaryCreateFromConstructor(newTarget, %NumberFormatPrototype%).
    // 3. ReturnIfAbrupt(numberFormat).
    Structure* structure = InternalFunction::createSubclassStructure(state, state->newTarget(), jsCast<IntlNumberFormatConstructor*>(state->jsCallee())->numberFormatStructure());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    IntlNumberFormat* numberFormat = IntlNumberFormat::create(vm, structure);
    ASSERT(numberFormat);

    // 4. Return InitializeNumberFormat(numberFormat, locales, options).
    scope.release();
    numberFormat->initializeNumberFormat(*state, state->argument(0), state->argument(1));
    return JSValue::encode(numberFormat);
}

static EncodedJSValue JSC_HOST_CALL callIntlNumberFormat(ExecState* state)
{
    // 11.1.2 Intl.NumberFormat ([locales [, options]]) (ECMA-402 2.0)
    // 1. If NewTarget is undefined, let newTarget be the active function object, else let newTarget be NewTarget.
    // NewTarget is always undefined when called as a function.

    IntlNumberFormatConstructor* callee = jsCast<IntlNumberFormatConstructor*>(state->jsCallee());

    // FIXME: Workaround to provide compatibility with ECMA-402 1.0 call/apply patterns.
    // https://bugs.webkit.org/show_bug.cgi?id=153679
    return JSValue::encode(constructIntlInstanceWithWorkaroundForLegacyIntlConstructor<IntlNumberFormat>(*state, state->thisValue(), callee, [&] (VM& vm) {
        // 2. Let numberFormat be OrdinaryCreateFromConstructor(newTarget, %NumberFormatPrototype%).
        // 3. ReturnIfAbrupt(numberFormat).
        IntlNumberFormat* numberFormat = IntlNumberFormat::create(vm, callee->numberFormatStructure());
        ASSERT(numberFormat);

        // 4. Return InitializeNumberFormat(numberFormat, locales, options).
        numberFormat->initializeNumberFormat(*state, state->argument(0), state->argument(1));
        return numberFormat;
    }));
}

ConstructType IntlNumberFormatConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructIntlNumberFormat;
    return ConstructType::Host;
}

CallType IntlNumberFormatConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callIntlNumberFormat;
    return CallType::Host;
}

EncodedJSValue JSC_HOST_CALL IntlNumberFormatConstructorFuncSupportedLocalesOf(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // 11.2.2 Intl.NumberFormat.supportedLocalesOf(locales [, options]) (ECMA-402 2.0)

    // 1. Let availableLocales be %NumberFormat%.[[availableLocales]].
    JSGlobalObject* globalObject = state->jsCallee()->globalObject();
    const HashSet<String> availableLocales = globalObject->intlNumberFormatAvailableLocales();

    // 2. Let requestedLocales be CanonicalizeLocaleList(locales).
    Vector<String> requestedLocales = canonicalizeLocaleList(*state, state->argument(0));
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 3. Return SupportedLocales(availableLocales, requestedLocales, options).
    scope.release();
    return JSValue::encode(supportedLocales(*state, availableLocales, requestedLocales, state->argument(1)));
}

void IntlNumberFormatConstructor::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    IntlNumberFormatConstructor* thisObject = jsCast<IntlNumberFormatConstructor*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_numberFormatStructure);
}

} // namespace JSC

#endif // ENABLE(INTL)
