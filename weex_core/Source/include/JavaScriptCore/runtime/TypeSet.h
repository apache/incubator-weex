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

#include "ConcurrentJSLock.h"
#include "RuntimeType.h"
#include "StructureSet.h"
#include <wtf/HashSet.h>
#include <wtf/RefCounted.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/text/WTFString.h>
#include <wtf/Vector.h>

namespace Inspector {
namespace Protocol  {
template<typename T> class Array;

namespace Runtime {
class StructureDescription;
class TypeSet;
}

}
}

namespace JSC {

class StructureShape : public RefCounted<StructureShape> {
    friend class TypeSet;

public:
    StructureShape();

    static Ref<StructureShape> create() { return adoptRef(*new StructureShape); }
    String propertyHash();
    void markAsFinal();
    void addProperty(UniquedStringImpl&);
    String stringRepresentation();
    String toJSONString() const;
    Ref<Inspector::Protocol::Runtime::StructureDescription> inspectorRepresentation();
    void setConstructorName(String name) { m_constructorName = (name.isEmpty() ? ASCIILiteral("Object") : name); }
    String constructorName() { return m_constructorName; }
    void setProto(Ref<StructureShape>&& shape) { m_proto = WTFMove(shape); }
    void enterDictionaryMode();

private:
    static String leastCommonAncestor(const Vector<Ref<StructureShape>>&);
    static Ref<StructureShape> merge(Ref<StructureShape>&&, Ref<StructureShape>&&);
    bool hasSamePrototypeChain(const StructureShape&);

    HashSet<RefPtr<UniquedStringImpl>, IdentifierRepHash> m_fields;
    HashSet<RefPtr<UniquedStringImpl>, IdentifierRepHash> m_optionalFields;
    RefPtr<StructureShape> m_proto;
    std::unique_ptr<String> m_propertyHash;
    String m_constructorName;
    bool m_final;
    bool m_isInDictionaryMode;
};

class TypeSet : public ThreadSafeRefCounted<TypeSet> {

public:
    static Ref<TypeSet> create() { return adoptRef(*new TypeSet); }
    TypeSet();
    void addTypeInformation(RuntimeType, RefPtr<StructureShape>&&, Structure*);
    void invalidateCache();
    String dumpTypes() const;
    String displayName() const;
    Ref<Inspector::Protocol::Array<Inspector::Protocol::Runtime::StructureDescription>> allStructureRepresentations() const;
    String toJSONString() const;
    bool isOverflown() const { return m_isOverflown; }
    String leastCommonAncestor() const;
    Ref<Inspector::Protocol::Runtime::TypeSet> inspectorTypeSet() const;
    bool isEmpty() const { return m_seenTypes == TypeNothing; }
    bool doesTypeConformTo(RuntimeTypeMask test) const;
    RuntimeTypeMask seenTypes() const { return m_seenTypes; }
    StructureSet structureSet(const ConcurrentJSLocker&) const { return m_structureSet; }

    ConcurrentJSLock m_lock;
private:
    bool m_isOverflown;
    RuntimeTypeMask m_seenTypes;
    Vector<Ref<StructureShape>> m_structureHistory;
    StructureSet m_structureSet;
};

} // namespace JSC
