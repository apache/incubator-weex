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
