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

#include "JSArray.h"
#include "JSCellInlines.h"
#include "Structure.h"

namespace JSC {

inline IndexingType JSArray::mergeIndexingTypeForCopying(IndexingType other)
{
    IndexingType type = indexingType();
    if (!(type & IsArray && other & IsArray))
        return NonArray;

    if (hasAnyArrayStorage(type) || hasAnyArrayStorage(other))
        return NonArray;

    if (type == ArrayWithUndecided)
        return other;

    if (other == ArrayWithUndecided)
        return type;

    // We can memcpy an Int32 and a Contiguous into a Contiguous array since
    // both share the same memory layout for Int32 numbers.
    if ((type == ArrayWithInt32 || type == ArrayWithContiguous)
        && (other == ArrayWithInt32 || other == ArrayWithContiguous)) {
        if (other == ArrayWithContiguous)
            return other;
        return type;
    }

    if (type != other)
        return NonArray;

    return type;
}

inline bool JSArray::canFastCopy(VM& vm, JSArray* otherArray)
{
    if (hasAnyArrayStorage(indexingType()) || hasAnyArrayStorage(otherArray->indexingType()))
        return false;
    // FIXME: We should have a watchpoint for indexed properties on Array.prototype and Object.prototype
    // instead of walking the prototype chain. https://bugs.webkit.org/show_bug.cgi?id=155592
    if (structure(vm)->holesMustForwardToPrototype(vm)
        || otherArray->structure(vm)->holesMustForwardToPrototype(vm))
        return false;
    return true;
}

ALWAYS_INLINE unsigned getLength(ExecState* exec, JSObject* obj)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    if (isJSArray(obj))
        return jsCast<JSArray*>(obj)->length();

    JSValue lengthValue = obj->get(exec, vm.propertyNames->length);
    RETURN_IF_EXCEPTION(scope, UINT_MAX);
    scope.release();
    return lengthValue.toUInt32(exec);
}

ALWAYS_INLINE double toLength(ExecState* exec, JSObject* obj)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    if (isJSArray(obj))
        return jsCast<JSArray*>(obj)->length();

    JSValue lengthValue = obj->get(exec, vm.propertyNames->length);
    RETURN_IF_EXCEPTION(scope, PNaN);
    scope.release();
    return lengthValue.toLength(exec);
}

ALWAYS_INLINE bool JSArray::isIteratorProtocolFastAndNonObservable()
{
    return globalObject()->isArrayIteratorProtocolFastAndNonObservable();
}

} // namespace JSC
