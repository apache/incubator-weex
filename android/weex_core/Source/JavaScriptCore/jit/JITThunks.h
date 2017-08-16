/*
 * Copyright (C) 2012, 2013, 2016 Apple Inc. All rights reserved.
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
