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

#pragma once

#include "Identifier.h"
#include <wtf/Noncopyable.h>

namespace JSC {

class CommonIdentifiers;
class BytecodeGenerator;
class BytecodeIntrinsicNode;
class RegisterID;
class Identifier;

#define JSC_COMMON_BYTECODE_INTRINSIC_FUNCTIONS_EACH_NAME(macro) \
    macro(argument) \
    macro(argumentCount) \
    macro(assert) \
    macro(isObject) \
    macro(isJSArray) \
    macro(isProxyObject) \
    macro(isDerivedArray) \
    macro(isRegExpObject) \
    macro(isMap) \
    macro(isSet) \
    macro(tailCallForwardArguments) \
    macro(throwTypeError) \
    macro(throwRangeError) \
    macro(throwOutOfMemoryError) \
    macro(tryGetById) \
    macro(putByValDirect) \
    macro(toNumber) \
    macro(toString) \
    macro(newArrayWithSize) \

#define JSC_COMMON_BYTECODE_INTRINSIC_CONSTANTS_EACH_NAME(macro) \
    macro(undefined) \
    macro(Infinity) \
    macro(iterationKindKey) \
    macro(iterationKindValue) \
    macro(iterationKindKeyValue) \
    macro(MAX_ARRAY_INDEX) \
    macro(MAX_STRING_LENGTH) \
    macro(MAX_SAFE_INTEGER) \
    macro(ModuleFetch) \
    macro(ModuleTranslate) \
    macro(ModuleInstantiate) \
    macro(ModuleSatisfy) \
    macro(ModuleLink) \
    macro(ModuleReady) \
    macro(promiseStatePending) \
    macro(promiseStateFulfilled) \
    macro(promiseStateRejected) \
    macro(GeneratorResumeModeNormal) \
    macro(GeneratorResumeModeThrow) \
    macro(GeneratorResumeModeReturn) \
    macro(GeneratorStateCompleted) \
    macro(GeneratorStateExecuting) \


class BytecodeIntrinsicRegistry {
    WTF_MAKE_NONCOPYABLE(BytecodeIntrinsicRegistry);
public:
    explicit BytecodeIntrinsicRegistry(VM&);

    typedef RegisterID* (BytecodeIntrinsicNode::* EmitterType)(BytecodeGenerator&, RegisterID*);

    EmitterType lookup(const Identifier&) const;

#define JSC_DECLARE_BYTECODE_INTRINSIC_CONSTANT_GENERATORS(name) JSValue name##Value(BytecodeGenerator&);
    JSC_COMMON_BYTECODE_INTRINSIC_CONSTANTS_EACH_NAME(JSC_DECLARE_BYTECODE_INTRINSIC_CONSTANT_GENERATORS)
#undef JSC_DECLARE_BYTECODE_INTRINSIC_CONSTANT_GENERATORS

private:
    VM& m_vm;
    HashMap<RefPtr<UniquedStringImpl>, EmitterType, IdentifierRepHash> m_bytecodeIntrinsicMap;

#define JSC_DECLARE_BYTECODE_INTRINSIC_CONSTANT_GENERATORS(name) Strong<Unknown> m_##name;
    JSC_COMMON_BYTECODE_INTRINSIC_CONSTANTS_EACH_NAME(JSC_DECLARE_BYTECODE_INTRINSIC_CONSTANT_GENERATORS)
#undef JSC_DECLARE_BYTECODE_INTRINSIC_CONSTANT_GENERATORS
};

} // namespace JSC
