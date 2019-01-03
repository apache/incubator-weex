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

#if ENABLE(JIT)

#include "CallData.h"
#include "Intrinsic.h"
#include "MacroAssemblerCodeRef.h"
#include "ThunkGenerator.h"
#include "Weak.h"
#include "WeakHandleOwner.h"
#include <tuple>
#include <wtf/HashMap.h>
#include <wtf/ThreadingPrimitives.h>
#include <wtf/text/StringHash.h>

namespace JSC {
namespace DOMJIT {
class Signature;
}

class VM;
class NativeExecutable;

class JITThunks final : private WeakHandleOwner {
    WTF_MAKE_FAST_ALLOCATED;
public:
    JITThunks();
    virtual ~JITThunks();

    MacroAssemblerCodePtr ctiNativeCall(VM*);
    MacroAssemblerCodePtr ctiNativeConstruct(VM*);
    MacroAssemblerCodePtr ctiNativeTailCall(VM*);    
    MacroAssemblerCodePtr ctiNativeTailCallWithoutSavedTags(VM*);    

    MacroAssemblerCodeRef ctiStub(VM*, ThunkGenerator);
    MacroAssemblerCodeRef existingCTIStub(ThunkGenerator);

    NativeExecutable* hostFunctionStub(VM*, NativeFunction, NativeFunction constructor, const String& name);
    NativeExecutable* hostFunctionStub(VM*, NativeFunction, NativeFunction constructor, ThunkGenerator, Intrinsic, const DOMJIT::Signature*, const String& name);
    NativeExecutable* hostFunctionStub(VM*, NativeFunction, ThunkGenerator, Intrinsic, const String& name);

    void clearHostFunctionStubs();

private:
    void finalize(Handle<Unknown>, void* context) override;
    
    typedef HashMap<ThunkGenerator, MacroAssemblerCodeRef> CTIStubMap;
    CTIStubMap m_ctiStubMap;

    typedef std::tuple<NativeFunction, NativeFunction, String> HostFunctionKey;

    struct HostFunctionHash {
        static unsigned hash(const HostFunctionKey& key)
        {
            unsigned hash = WTF::pairIntHash(hashPointer(std::get<0>(key)), hashPointer(std::get<1>(key)));
            if (!std::get<2>(key).isNull())
                hash = WTF::pairIntHash(hash, DefaultHash<String>::Hash::hash(std::get<2>(key)));
            return hash;
        }
        static bool equal(const HostFunctionKey& a, const HostFunctionKey& b)
        {
            return (std::get<0>(a) == std::get<0>(b)) && (std::get<1>(a) == std::get<1>(b)) && (std::get<2>(a) == std::get<2>(b));
        }
        static const bool safeToCompareToEmptyOrDeleted = true;

    private:
        static inline unsigned hashPointer(NativeFunction p)
        {
            return DefaultHash<NativeFunction>::Hash::hash(p);
        }
    };

    struct HostFunctionHashTrait : WTF::GenericHashTraits<HostFunctionKey> {
        static const bool emptyValueIsZero = true;
        static EmptyValueType emptyValue() { return std::make_tuple(nullptr, nullptr, String()); }

        static void constructDeletedValue(HostFunctionKey& slot) { std::get<0>(slot) = reinterpret_cast<NativeFunction>(-1); }
        static bool isDeletedValue(const HostFunctionKey& value) { return std::get<0>(value) == reinterpret_cast<NativeFunction>(-1); }
    };
    
    typedef HashMap<HostFunctionKey, Weak<NativeExecutable>, HostFunctionHash, HostFunctionHashTrait> HostFunctionStubMap;
    std::unique_ptr<HostFunctionStubMap> m_hostFunctionStubMap;
    Lock m_lock;
};

} // namespace JSC

#endif // ENABLE(JIT)
