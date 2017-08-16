/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "WasmSignature.h"

#if ENABLE(WEBASSEMBLY)

#include "VM.h"
#include <wtf/FastMalloc.h>
#include <wtf/HashFunctions.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace Wasm {

namespace {
const bool verbose = false;
}

const constexpr SignatureIndex Signature::invalidIndex;

String Signature::toString() const
{
    String result(makeString(returnType()));
    result.append(" (");
    for (SignatureArgCount arg = 0; arg < argumentCount(); ++arg) {
        if (arg)
            result.append(", ");
        result.append(makeString(argument(arg)));
    }
    result.append(')');
    return result;
}

void Signature::dump(PrintStream& out) const
{
    out.print(toString());
}

unsigned Signature::hash() const
{
    uint32_t sizeToHash = allocatedSize(argumentCount()) / sizeof(allocationSizeRoundsUpTo);
    // Assumes over-allocated memory was zero-initialized, and rounded-up to allocationSizeRoundsUpTo so that a wider hash can be performed.
    ASSERT(sizeToHash * sizeof(allocationSizeRoundsUpTo) == allocatedSize(argumentCount()));
    unsigned accumulator = 0xa1bcedd8u;
    const auto* pos = reinterpret_cast<const allocationSizeRoundsUpTo*>(this);
    for (uint32_t i = 0; i < sizeToHash; ++i)
        accumulator = WTF::pairIntHash(accumulator, WTF::IntHash<allocationSizeRoundsUpTo>::hash(*pos));
    return accumulator;
}

Signature* Signature::create(SignatureArgCount argumentCount)
{
    // Hashing relies on allocation zero-initializing trailing elements.
    auto allocated = tryFastCalloc(allocatedSize(argumentCount), 1);
    Signature* signature;
    if (!allocated.getValue(signature))
        return nullptr;
    new (signature) Signature(argumentCount);
    return signature;
}

Signature* Signature::createInvalid()
{
    Signature* signature = create(0);
    RELEASE_ASSERT(signature);
    new (signature) Signature(std::numeric_limits<SignatureArgCount>::max());
    return signature;
}

void Signature::destroy(Signature* signature)
{
    fastFree(signature);
}

SignatureInformation::~SignatureInformation()
{
    for (size_t i = 0; i < m_signatures.size(); ++i)
        Signature::destroy(m_signatures[i]);
}

SignatureInformation::SignatureInformation()
{
    // The zeroth entry is an invalid signature, to match invalidIndex.
    ASSERT(!Signature::invalidIndex);
    Signature* invalidSignature = Signature::createInvalid();
    auto addResult = m_signatureMap.add(SignatureHash { invalidSignature }, Signature::invalidIndex);
    RELEASE_ASSERT(addResult.isNewEntry);
    ASSERT(Signature::invalidIndex == addResult.iterator->value);
    m_signatures.append(invalidSignature);
}

SignatureInformation* SignatureInformation::get(VM* vm)
{
    std::call_once(vm->m_wasmSignatureInformationOnceFlag, [vm] {
        vm->m_wasmSignatureInformation = std::unique_ptr<SignatureInformation>(new SignatureInformation());
    });
    return vm->m_wasmSignatureInformation.get();
}

SignatureIndex SignatureInformation::adopt(VM* vm, Signature* signature)
{
    SignatureInformation* info = get(vm);
    LockHolder lock(info->m_lock);

    SignatureIndex nextValue = info->m_signatures.size();
    auto addResult = info->m_signatureMap.add(SignatureHash { signature }, nextValue);
    if (addResult.isNewEntry) {
        ASSERT(nextValue == addResult.iterator->value);
        if (verbose)
            dataLogLn("Adopt new signature ", *signature, " with index ", addResult.iterator->value, " hash: ", signature->hash());
        info->m_signatures.append(signature);
        return nextValue;
    }
    if (verbose)
        dataLogLn("Existing signature ", *signature, " with index ", addResult.iterator->value, " hash: ", signature->hash());
    Signature::destroy(signature);
    ASSERT(addResult.iterator->value != Signature::invalidIndex);
    return addResult.iterator->value;
}

const Signature* SignatureInformation::get(VM* vm, SignatureIndex index)
{
    ASSERT(index != Signature::invalidIndex);
    SignatureInformation* info = get(vm);
    LockHolder lock(info->m_lock);

    if (verbose)
        dataLogLn("Got signature ", *info->m_signatures.at(index), " at index ", index);
    return info->m_signatures.at(index);
}

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
