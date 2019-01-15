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
#pragma once

#if ENABLE(INTL)

#include "BuiltinNames.h"
#include "IntlObject.h"
#include "JSObject.h"

namespace JSC {

template<typename IntlInstance, typename Constructor, typename Factory>
JSValue constructIntlInstanceWithWorkaroundForLegacyIntlConstructor(ExecState& state, JSValue thisValue, Constructor* callee, Factory factory)
{
    // FIXME: Workaround to provide compatibility with ECMA-402 1.0 call/apply patterns.
    // https://bugs.webkit.org/show_bug.cgi?id=153679
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!jsDynamicCast<IntlInstance*>(vm, thisValue)) {
        JSValue prototype = callee->getDirect(vm, vm.propertyNames->prototype);
        bool hasInstance = JSObject::defaultHasInstance(&state, thisValue, prototype);
        RETURN_IF_EXCEPTION(scope, JSValue());
        if (hasInstance) {
            JSObject* thisObject = thisValue.toObject(&state);
            RETURN_IF_EXCEPTION(scope, JSValue());

            IntlInstance* instance = factory(vm);
            RETURN_IF_EXCEPTION(scope, JSValue());

            thisObject->putDirect(vm, vm.propertyNames->builtinNames().intlSubstituteValuePrivateName(), instance);
            return thisObject;
        }
    }
    scope.release();
    return factory(vm);
}

} // namespace JSC

#endif // ENABLE(INTL)
