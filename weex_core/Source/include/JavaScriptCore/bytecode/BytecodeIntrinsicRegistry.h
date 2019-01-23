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
