/*
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
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
#include "TemplateRegistry.h"

#include "BuiltinNames.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "JSTemplateRegistryKey.h"
#include "ObjectConstructor.h"
#include "TemplateRegistryKey.h"
#include "WeakGCMapInlines.h"

namespace JSC {

TemplateRegistry::TemplateRegistry(VM& vm)
    : m_templateMap(vm)
{
}

JSArray* TemplateRegistry::getTemplateObject(ExecState* exec, JSTemplateRegistryKey* templateKeyObject)
{
    auto& templateKey = templateKeyObject->templateRegistryKey();
    JSArray* cached = m_templateMap.get(&templateKey);
    if (cached)
        return cached;

    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    unsigned count = templateKey.cookedStrings().size();
    JSArray* templateObject = constructEmptyArray(exec, nullptr, count);
    RETURN_IF_EXCEPTION(scope, nullptr);
    JSArray* rawObject = constructEmptyArray(exec, nullptr, count);
    RETURN_IF_EXCEPTION(scope, nullptr);

    for (unsigned index = 0; index < count; ++index) {
        auto cooked = templateKey.cookedStrings()[index];
        if (cooked)
            templateObject->putDirectIndex(exec, index, jsString(exec, cooked.value()), ReadOnly | DontDelete, PutDirectIndexLikePutDirect);
        else
            templateObject->putDirectIndex(exec, index, jsUndefined(), ReadOnly | DontDelete, PutDirectIndexLikePutDirect);
        RETURN_IF_EXCEPTION(scope, nullptr);

        rawObject->putDirectIndex(exec, index, jsString(exec, templateKey.rawStrings()[index]), ReadOnly | DontDelete, PutDirectIndexLikePutDirect);
        RETURN_IF_EXCEPTION(scope, nullptr);
    }

    objectConstructorFreeze(exec, rawObject);
    ASSERT(!scope.exception());

    templateObject->putDirect(vm, vm.propertyNames->raw, rawObject, ReadOnly | DontEnum | DontDelete);

    // Template JSArray hold the reference to JSTemplateRegistryKey to make TemplateRegistryKey pointer live until this JSArray is collected.
    // TemplateRegistryKey pointer is used for TemplateRegistry's key.
    templateObject->putDirect(vm, vm.propertyNames->builtinNames().templateRegistryKeyPrivateName(), templateKeyObject, ReadOnly | DontEnum | DontDelete);

    objectConstructorFreeze(exec, templateObject);
    ASSERT(!scope.exception());

    m_templateMap.set(&templateKey, templateObject);

    return templateObject;
}


} // namespace JSC
