/*
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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
#include "BytecodeIntrinsicRegistry.h"

#include "ArrayIteratorPrototype.h"
#include "BuiltinNames.h"
#include "BytecodeGenerator.h"
#include "JSCJSValueInlines.h"
#include "JSGeneratorFunction.h"
#include "JSModuleLoader.h"
#include "JSPromise.h"
#include "Nodes.h"
#include "StrongInlines.h"

namespace JSC {

#define INITIALIZE_BYTECODE_INTRINSIC_NAMES_TO_SET(name) m_bytecodeIntrinsicMap.add(vm.propertyNames->builtinNames().name##PrivateName().impl(), &BytecodeIntrinsicNode::emit_intrinsic_##name);

BytecodeIntrinsicRegistry::BytecodeIntrinsicRegistry(VM& vm)
    : m_vm(vm)
    , m_bytecodeIntrinsicMap()
{
    JSC_COMMON_BYTECODE_INTRINSIC_FUNCTIONS_EACH_NAME(INITIALIZE_BYTECODE_INTRINSIC_NAMES_TO_SET)
    JSC_COMMON_BYTECODE_INTRINSIC_CONSTANTS_EACH_NAME(INITIALIZE_BYTECODE_INTRINSIC_NAMES_TO_SET)

    m_undefined.set(m_vm, jsUndefined());
    m_Infinity.set(m_vm, jsDoubleNumber(std::numeric_limits<double>::infinity()));
    m_iterationKindKey.set(m_vm, jsNumber(IterateKey));
    m_iterationKindValue.set(m_vm, jsNumber(IterateValue));
    m_iterationKindKeyValue.set(m_vm, jsNumber(IterateKeyValue));
    m_MAX_ARRAY_INDEX.set(m_vm, jsNumber(MAX_ARRAY_INDEX));
    m_MAX_STRING_LENGTH.set(m_vm, jsNumber(JSString::MaxLength));
    m_MAX_SAFE_INTEGER.set(m_vm, jsDoubleNumber(maxSafeInteger()));
    m_ModuleFetch.set(m_vm, jsNumber(static_cast<unsigned>(JSModuleLoader::Status::Fetch)));
    m_ModuleInstantiate.set(m_vm, jsNumber(static_cast<unsigned>(JSModuleLoader::Status::Instantiate)));
    m_ModuleSatisfy.set(m_vm, jsNumber(static_cast<unsigned>(JSModuleLoader::Status::Satisfy)));
    m_ModuleLink.set(m_vm, jsNumber(static_cast<unsigned>(JSModuleLoader::Status::Link)));
    m_ModuleReady.set(m_vm, jsNumber(static_cast<unsigned>(JSModuleLoader::Status::Ready)));
    m_promiseStatePending.set(m_vm, jsNumber(static_cast<unsigned>(JSPromise::Status::Pending)));
    m_promiseStateFulfilled.set(m_vm, jsNumber(static_cast<unsigned>(JSPromise::Status::Fulfilled)));
    m_promiseStateRejected.set(m_vm, jsNumber(static_cast<unsigned>(JSPromise::Status::Rejected)));
    m_GeneratorResumeModeNormal.set(m_vm, jsNumber(static_cast<int32_t>(JSGeneratorFunction::GeneratorResumeMode::NormalMode)));
    m_GeneratorResumeModeThrow.set(m_vm, jsNumber(static_cast<int32_t>(JSGeneratorFunction::GeneratorResumeMode::ThrowMode)));
    m_GeneratorResumeModeReturn.set(m_vm, jsNumber(static_cast<int32_t>(JSGeneratorFunction::GeneratorResumeMode::ReturnMode)));
    m_GeneratorStateCompleted.set(m_vm, jsNumber(static_cast<int32_t>(JSGeneratorFunction::GeneratorState::Completed)));
    m_GeneratorStateExecuting.set(m_vm, jsNumber(static_cast<int32_t>(JSGeneratorFunction::GeneratorState::Executing)));
}

BytecodeIntrinsicNode::EmitterType BytecodeIntrinsicRegistry::lookup(const Identifier& ident) const
{
    if (!m_vm.propertyNames->isPrivateName(ident))
        return nullptr;
    auto iterator = m_bytecodeIntrinsicMap.find(ident.impl());
    if (iterator == m_bytecodeIntrinsicMap.end())
        return nullptr;
    return iterator->value;
}

#define JSC_DECLARE_BYTECODE_INTRINSIC_CONSTANT_GENERATORS(name) \
    JSValue BytecodeIntrinsicRegistry::name##Value(BytecodeGenerator&) \
    { \
        return m_##name.get(); \
    }
    JSC_COMMON_BYTECODE_INTRINSIC_CONSTANTS_EACH_NAME(JSC_DECLARE_BYTECODE_INTRINSIC_CONSTANT_GENERATORS)
#undef JSC_DECLARE_BYTECODE_INTRINSIC_CONSTANT_GENERATORS

} // namespace JSC

