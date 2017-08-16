/*
 * Copyright (C) 2012, 2016 Apple Inc. All rights reserved.
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
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
#include "SymbolConstructor.h"

#include "Error.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "Symbol.h"
#include "SymbolPrototype.h"
#include <wtf/text/SymbolRegistry.h>

namespace JSC {

static EncodedJSValue JSC_HOST_CALL symbolConstructorFor(ExecState*);
static EncodedJSValue JSC_HOST_CALL symbolConstructorKeyFor(ExecState*);

}

#include "SymbolConstructor.lut.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(SymbolConstructor);

const ClassInfo SymbolConstructor::s_info = { "Function", &Base::s_info, &symbolConstructorTable, CREATE_METHOD_TABLE(SymbolConstructor) };

/* Source for SymbolConstructor.lut.h
@begin symbolConstructorTable
  for       symbolConstructorFor       DontEnum|Function 1
  keyFor    symbolConstructorKeyFor    DontEnum|Function 1
@end
*/

SymbolConstructor::SymbolConstructor(VM& vm, Structure* structure)
    : InternalFunction(vm, structure)
{
}

#define INITIALIZE_WELL_KNOWN_SYMBOLS(name) \
    putDirectWithoutTransition(vm, Identifier::fromString(&vm, #name), Symbol::create(vm, static_cast<SymbolImpl&>(*vm.propertyNames->name##Symbol.impl())), DontEnum | DontDelete | ReadOnly);

void SymbolConstructor::finishCreation(VM& vm, SymbolPrototype* prototype)
{
    Base::finishCreation(vm, prototype->classInfo(vm)->className);
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(0), DontDelete | ReadOnly | DontEnum);

    JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_WELL_KNOWN_SYMBOL(INITIALIZE_WELL_KNOWN_SYMBOLS)
}

// ------------------------------ Functions ---------------------------

static EncodedJSValue JSC_HOST_CALL callSymbol(ExecState* exec)
{
    JSValue description = exec->argument(0);
    if (description.isUndefined())
        return JSValue::encode(Symbol::create(exec->vm()));
    return JSValue::encode(Symbol::create(exec, description.toString(exec)));
}

ConstructType SymbolConstructor::getConstructData(JSCell*, ConstructData&)
{
    return ConstructType::None;
}

CallType SymbolConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callSymbol;
    return CallType::Host;
}

EncodedJSValue JSC_HOST_CALL symbolConstructorFor(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSString* stringKey = exec->argument(0).toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    String string = stringKey->value(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    return JSValue::encode(Symbol::create(exec->vm(), exec->vm().symbolRegistry().symbolForKey(string)));
}

const char* const SymbolKeyForTypeError = "Symbol.keyFor requires that the first argument be a symbol";

EncodedJSValue JSC_HOST_CALL symbolConstructorKeyFor(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue symbolValue = exec->argument(0);
    if (!symbolValue.isSymbol())
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral(SymbolKeyForTypeError)));

    SymbolImpl& uid = asSymbol(symbolValue)->privateName().uid();
    if (!uid.symbolRegistry())
        return JSValue::encode(jsUndefined());

    ASSERT(uid.symbolRegistry() == &vm.symbolRegistry());
    return JSValue::encode(jsString(exec, vm.symbolRegistry().keyForSymbol(uid)));
}

} // namespace JSC
