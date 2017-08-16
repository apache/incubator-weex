/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include "FunctionRareData.h"

#include "JSCInlines.h"

namespace JSC {

const ClassInfo FunctionRareData::s_info = { "FunctionRareData", 0, 0, CREATE_METHOD_TABLE(FunctionRareData) };

FunctionRareData* FunctionRareData::create(VM& vm)
{
    FunctionRareData* rareData = new (NotNull, allocateCell<FunctionRareData>(vm.heap)) FunctionRareData(vm);
    rareData->finishCreation(vm);
    return rareData;
}

void FunctionRareData::destroy(JSCell* cell)
{
    FunctionRareData* rareData = static_cast<FunctionRareData*>(cell);
    rareData->FunctionRareData::~FunctionRareData();
}

Structure* FunctionRareData::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
}

void FunctionRareData::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    FunctionRareData* rareData = jsCast<FunctionRareData*>(cell);

    rareData->m_objectAllocationProfile.visitAggregate(visitor);
    rareData->m_internalFunctionAllocationProfile.visitAggregate(visitor);
    visitor.append(rareData->m_boundFunctionStructure);
}

FunctionRareData::FunctionRareData(VM& vm)
    : Base(vm, vm.functionRareDataStructure.get())
    , m_objectAllocationProfile()
    // We initialize blind so that changes to the prototype after function creation but before
    // the optimizer kicks in don't disable optimizations. Once the optimizer kicks in, the
    // watchpoint will start watching and any changes will both force deoptimization and disable
    // future attempts to optimize. This is necessary because we are guaranteed that the
    // allocation profile is changed exactly once prior to optimizations kicking in. We could be
    // smarter and count the number of times the prototype is clobbered and only optimize if it
    // was clobbered exactly once, but that seems like overkill. In almost all cases it will be
    // clobbered once, and if it's clobbered more than once, that will probably only occur
    // before we started optimizing, anyway.
    , m_objectAllocationProfileWatchpoint(ClearWatchpoint)
{
}

FunctionRareData::~FunctionRareData()
{
}

void FunctionRareData::initializeObjectAllocationProfile(VM& vm, JSGlobalObject* globalObject, JSObject* prototype, size_t inlineCapacity)
{
    m_objectAllocationProfile.initialize(vm, globalObject, this, prototype, inlineCapacity);
}

void FunctionRareData::clear(const char* reason)
{
    m_objectAllocationProfile.clear();
    m_internalFunctionAllocationProfile.clear();
    m_objectAllocationProfileWatchpoint.fireAll(*vm(), reason);
}

}
