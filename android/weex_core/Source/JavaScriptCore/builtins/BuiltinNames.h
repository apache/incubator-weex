/*
 * Copyright (C) 2014, 2016 Apple Inc. All rights reserved.
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

#include "BuiltinUtils.h"
#include "BytecodeIntrinsicRegistry.h"
#include "CommonIdentifiers.h"
#include "JSCBuiltins.h"

namespace JSC {

#define JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(macro) \
    JSC_COMMON_BYTECODE_INTRINSIC_FUNCTIONS_EACH_NAME(macro) \
    JSC_COMMON_BYTECODE_INTRINSIC_CONSTANTS_EACH_NAME(macro) \
    macro(arrayIteratorNextIndex) \
    macro(arrayIterationKind) \
    macro(arrayIteratorNext) \
    macro(arrayIteratorIsDone) \
    macro(arrayIteratorKind) \
    macro(charCodeAt) \
    macro(isView) \
    macro(iteratedObject) \
    macro(iteratedString) \
    macro(stringIteratorNextIndex) \
    macro(promise) \
    macro(fulfillmentHandler) \
    macro(rejectionHandler) \
    macro(index) \
    macro(deferred) \
    macro(countdownHolder) \
    macro(Object) \
    macro(ownEnumerablePropertyKeys) \
    macro(Number) \
    macro(Array) \
    macro(ArrayBuffer) \
    macro(String) \
    macro(RegExp) \
    macro(Map) \
    macro(Promise) \
    macro(Reflect) \
    macro(InternalPromise) \
    macro(abs) \
    macro(floor) \
    macro(trunc) \
    macro(create) \
    macro(defineProperty) \
    macro(getPrototypeOf) \
    macro(getOwnPropertyDescriptor) \
    macro(getOwnPropertyNames) \
    macro(ownKeys) \
    macro(Error) \
    macro(RangeError) \
    macro(TypeError) \
    macro(typedArrayLength) \
    macro(typedArraySort) \
    macro(typedArrayGetOriginalConstructor) \
    macro(typedArraySubarrayCreate) \
    macro(BuiltinLog) \
    macro(homeObject) \
    macro(getTemplateObject) \
    macro(templateRegistryKey) \
    macro(enqueueJob) \
    macro(promiseState) \
    macro(promiseReactions) \
    macro(promiseResult) \
    macro(onFulfilled) \
    macro(onRejected) \
    macro(push) \
    macro(repeatCharacter) \
    macro(capabilities) \
    macro(starDefault) \
    macro(InspectorInstrumentation) \
    macro(get) \
    macro(set) \
    macro(shift) \
    macro(allocateTypedArray) \
    macro(Int8Array) \
    macro(Int16Array) \
    macro(Int32Array) \
    macro(Uint8Array) \
    macro(Uint8ClampedArray) \
    macro(Uint16Array) \
    macro(Uint32Array) \
    macro(Float32Array) \
    macro(Float64Array) \
    macro(exec) \
    macro(generator) \
    macro(generatorNext) \
    macro(generatorState) \
    macro(generatorFrame) \
    macro(generatorValue) \
    macro(generatorThis) \
    macro(generatorResumeMode) \
    macro(Collator) \
    macro(DateTimeFormat) \
    macro(NumberFormat) \
    macro(intlSubstituteValue) \
    macro(thisTimeValue) \
    macro(thisNumberValue) \
    macro(newTargetLocal) \
    macro(derivedConstructor) \
    macro(isTypedArrayView) \
    macro(isBoundFunction) \
    macro(hasInstanceBoundFunction) \
    macro(instanceOf) \
    macro(isArraySlow) \
    macro(isArrayConstructor) \
    macro(isConstructor) \
    macro(isDerivedConstructor) \
    macro(concatMemcpy) \
    macro(appendMemcpy) \
    macro(predictFinalLengthFromArgumunts) \
    macro(print) \
    macro(regExpCreate) \
    macro(SetIterator) \
    macro(setIteratorNext) \
    macro(replaceUsingRegExp) \
    macro(replaceUsingStringSearch) \
    macro(MapIterator) \
    macro(mapIteratorNext) \
    macro(regExpBuiltinExec) \
    macro(regExpMatchFast) \
    macro(regExpProtoFlagsGetter) \
    macro(regExpProtoGlobalGetter) \
    macro(regExpProtoIgnoreCaseGetter) \
    macro(regExpProtoMultilineGetter) \
    macro(regExpProtoSourceGetter) \
    macro(regExpProtoStickyGetter) \
    macro(regExpProtoUnicodeGetter) \
    macro(regExpPrototypeSymbolReplace) \
    macro(regExpReplaceFast) \
    macro(regExpSearchFast) \
    macro(regExpSplitFast) \
    macro(regExpTestFast) \
    macro(stringIncludesInternal) \
    macro(stringSplitFast) \
    macro(stringSubstrInternal) \
    macro(toObject) \
    macro(makeBoundFunction) \
    macro(hasOwnLengthProperty) \
    macro(importModule) \
    macro(WebAssembly) \
    macro(Module) \
    macro(Instance) \
    macro(Memory) \
    macro(Table) \
    macro(CompileError) \
    macro(LinkError) \
    macro(RuntimeError) \


#define INITIALIZE_PRIVATE_TO_PUBLIC_ENTRY(name) m_privateToPublicMap.add(m_##name##PrivateName.impl(), &m_##name);
#define INITIALIZE_PUBLIC_TO_PRIVATE_ENTRY(name) m_publicToPrivateMap.add(m_##name.impl(), &m_##name##PrivateName);

// We commandeer the publicToPrivateMap to allow us to convert private symbol names into the appropriate symbol.
// e.g. @iteratorSymbol points to Symbol.iterator in this map rather than to a an actual private name.
// FIXME: This is a weird hack and we shouldn't need to do this.
#define INITIALIZE_SYMBOL_PUBLIC_TO_PRIVATE_ENTRY(name) m_publicToPrivateMap.add(m_##name##SymbolPrivateIdentifier.impl(), &m_##name##Symbol);

class BuiltinNames {
    WTF_MAKE_NONCOPYABLE(BuiltinNames); WTF_MAKE_FAST_ALLOCATED;
    
public:
    // We treat the dollarVM name as a special case below for $vm (because CommonIdentifiers does not
    // yet support the $ character).

    BuiltinNames(VM* vm, CommonIdentifiers* commonIdentifiers)
        : m_emptyIdentifier(commonIdentifiers->emptyIdentifier)
        JSC_FOREACH_BUILTIN_FUNCTION_NAME(INITIALIZE_BUILTIN_NAMES)
        JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(INITIALIZE_BUILTIN_NAMES)
        JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_WELL_KNOWN_SYMBOL(INITIALIZE_BUILTIN_SYMBOLS)
        , m_dollarVMName(Identifier::fromString(vm, "$vm"))
        , m_dollarVMPrivateName(Identifier::fromUid(PrivateName(PrivateName::Description, ASCIILiteral("PrivateSymbol.$vm"))))
    {
        JSC_FOREACH_BUILTIN_FUNCTION_NAME(INITIALIZE_PRIVATE_TO_PUBLIC_ENTRY)
        JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(INITIALIZE_PRIVATE_TO_PUBLIC_ENTRY)
        JSC_FOREACH_BUILTIN_FUNCTION_NAME(INITIALIZE_PUBLIC_TO_PRIVATE_ENTRY)
        JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(INITIALIZE_PUBLIC_TO_PRIVATE_ENTRY)
        JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_WELL_KNOWN_SYMBOL(INITIALIZE_SYMBOL_PUBLIC_TO_PRIVATE_ENTRY)
        m_privateToPublicMap.add(m_dollarVMPrivateName.impl(), &m_dollarVMName);
        m_publicToPrivateMap.add(m_dollarVMName.impl(), &m_dollarVMPrivateName);
    }

    bool isPrivateName(SymbolImpl& uid) const;
    bool isPrivateName(UniquedStringImpl& uid) const;
    bool isPrivateName(const Identifier&) const;
    const Identifier* lookUpPrivateName(const Identifier&) const;
    const Identifier& lookUpPublicName(const Identifier&) const;
    
    void appendExternalName(const Identifier& publicName, const Identifier& privateName);

    JSC_FOREACH_BUILTIN_FUNCTION_NAME(DECLARE_BUILTIN_IDENTIFIER_ACCESSOR)
    JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(DECLARE_BUILTIN_IDENTIFIER_ACCESSOR)
    JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_WELL_KNOWN_SYMBOL(DECLARE_BUILTIN_SYMBOL_ACCESSOR)
    const JSC::Identifier& dollarVMPublicName() const { return m_dollarVMName; }
    const JSC::Identifier& dollarVMPrivateName() const { return m_dollarVMPrivateName; }

private:
    Identifier m_emptyIdentifier;
    JSC_FOREACH_BUILTIN_FUNCTION_NAME(DECLARE_BUILTIN_NAMES)
    JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_PROPERTY_NAME(DECLARE_BUILTIN_NAMES)
    JSC_COMMON_PRIVATE_IDENTIFIERS_EACH_WELL_KNOWN_SYMBOL(DECLARE_BUILTIN_SYMBOLS)
    const JSC::Identifier m_dollarVMName;
    const JSC::Identifier m_dollarVMPrivateName;
    typedef HashMap<RefPtr<UniquedStringImpl>, const Identifier*, IdentifierRepHash> BuiltinNamesMap;
    BuiltinNamesMap m_publicToPrivateMap;
    BuiltinNamesMap m_privateToPublicMap;
};

inline bool BuiltinNames::isPrivateName(SymbolImpl& uid) const
{
    return m_privateToPublicMap.contains(&uid);
}

inline bool BuiltinNames::isPrivateName(UniquedStringImpl& uid) const
{
    if (!uid.isSymbol())
        return false;
    return m_privateToPublicMap.contains(&uid);
}

inline bool BuiltinNames::isPrivateName(const Identifier& ident) const
{
    if (ident.isNull())
        return false;
    return isPrivateName(*ident.impl());
}

inline const Identifier* BuiltinNames::lookUpPrivateName(const Identifier& ident) const
{
    auto iter = m_publicToPrivateMap.find(ident.impl());
    if (iter != m_publicToPrivateMap.end())
        return iter->value;
    return 0;
}

inline const Identifier& BuiltinNames::lookUpPublicName(const Identifier& ident) const
{
    auto iter = m_privateToPublicMap.find(ident.impl());
    if (iter != m_privateToPublicMap.end())
        return *iter->value;
    return m_emptyIdentifier;
}

inline void BuiltinNames::appendExternalName(const Identifier& publicName, const Identifier& privateName)
{
#ifndef NDEBUG
    for (const auto& key : m_publicToPrivateMap.keys())
        ASSERT(publicName.string() != *key);
#endif

    m_privateToPublicMap.add(privateName.impl(), &publicName);
    m_publicToPrivateMap.add(publicName.impl(), &privateName);
}

} // namespace JSC
