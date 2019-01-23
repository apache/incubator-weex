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

#include "JSGenericTypedArrayViewPrototype.h"

namespace JSC {
    
template<typename ViewClass>
JSGenericTypedArrayViewPrototype<ViewClass>::JSGenericTypedArrayViewPrototype(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

template<typename ViewClass>
void JSGenericTypedArrayViewPrototype<ViewClass>::finishCreation(
    VM& vm, JSGlobalObject*)
{
    Base::finishCreation(vm);
    
    ASSERT(inherits(vm, info()));

    putDirect(vm, vm.propertyNames->BYTES_PER_ELEMENT, jsNumber(ViewClass::elementSize), DontEnum | ReadOnly | DontDelete);

}

template<typename ViewClass>
JSGenericTypedArrayViewPrototype<ViewClass>*
JSGenericTypedArrayViewPrototype<ViewClass>::create(
    VM& vm, JSGlobalObject* globalObject, Structure* structure)
{
    JSGenericTypedArrayViewPrototype* prototype =
        new (NotNull, allocateCell<JSGenericTypedArrayViewPrototype>(vm.heap))
        JSGenericTypedArrayViewPrototype(vm, structure);
    prototype->finishCreation(vm, globalObject);
    return prototype;
}

template<typename ViewClass>
Structure* JSGenericTypedArrayViewPrototype<ViewClass>::createStructure(
    VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(
        vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

} // namespace JSC
