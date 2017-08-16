/*
 * Copyright (C) 2013 Apple, Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ArrayIteratorPrototype.h"

#include "ArrayIteratorPrototype.lut.h"
#include "IteratorOperations.h"
#include "JSCInlines.h"
#include "JSCJSValueInlines.h"
#include "JSCellInlines.h"
#include "JSGlobalObject.h"
#include "ObjectConstructor.h"
#include "StructureInlines.h"

namespace JSC {

const ClassInfo ArrayIteratorPrototype::s_info = { "Array Iterator", &Base::s_info, &arrayIteratorPrototypeTable, CREATE_METHOD_TABLE(ArrayIteratorPrototype) };

/* Source for ArrayIteratorPrototype.lut.h
@begin arrayIteratorPrototypeTable
  next      JSBuiltin  DontEnum|Function 0
@end
*/

void ArrayIteratorPrototype::finishCreation(VM& vm, JSGlobalObject*)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    putDirectWithoutTransition(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, "Array Iterator"), DontEnum | ReadOnly);
    vm.prototypeMap.addPrototype(this);
}

// ------------------------------ Array Functions ----------------------------

} // namespace JSC
