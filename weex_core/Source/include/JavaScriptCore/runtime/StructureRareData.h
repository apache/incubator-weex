/*
 * Copyright (C) 2013, 2014 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "ClassInfo.h"
#include "JSCell.h"
#include "JSTypeInfo.h"
#include "PropertyOffset.h"
#include "PropertySlot.h"

namespace JSC {

class JSPropertyNameEnumerator;
class Structure;
class ObjectToStringAdaptiveStructureWatchpoint;
class ObjectToStringAdaptiveInferredPropertyValueWatchpoint;

class StructureRareData final : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static StructureRareData* create(VM&, Structure*);

    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    static void visitChildren(JSCell*, SlotVisitor&);

    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);

    Structure* previousID() const;
    void setPreviousID(VM&, Structure*);
    void clearPreviousID();

    JSString* objectToStringValue() const;
    void setObjectToStringValue(ExecState*, VM&, Structure* baseStructure, JSString* value, PropertySlot toStringTagSymbolSlot);

    JSPropertyNameEnumerator* cachedPropertyNameEnumerator() const;
    void setCachedPropertyNameEnumerator(VM&, JSPropertyNameEnumerator*);

    DECLARE_EXPORT_INFO;

private:
    friend class Structure;
    friend class ObjectToStringAdaptiveStructureWatchpoint;
    friend class ObjectToStringAdaptiveInferredPropertyValueWatchpoint;

    void clearObjectToStringValue();

    StructureRareData(VM&, Structure*);

    WriteBarrier<Structure> m_previous;
    WriteBarrier<JSString> m_objectToStringValue;
    WriteBarrier<JSPropertyNameEnumerator> m_cachedPropertyNameEnumerator;
    
    typedef HashMap<PropertyOffset, RefPtr<WatchpointSet>, WTF::IntHash<PropertyOffset>, WTF::UnsignedWithZeroKeyHashTraits<PropertyOffset>> PropertyWatchpointMap;
    std::unique_ptr<PropertyWatchpointMap> m_replacementWatchpointSets;
    Bag<ObjectToStringAdaptiveStructureWatchpoint> m_objectToStringAdaptiveWatchpointSet;
    std::unique_ptr<ObjectToStringAdaptiveInferredPropertyValueWatchpoint> m_objectToStringAdaptiveInferredValueWatchpoint;
    bool m_giveUpOnObjectToStringValueCache;
};

} // namespace JSC
