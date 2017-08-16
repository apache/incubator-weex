/*
 * Copyright (C) 2014, 2015 Apple Inc. All Rights Reserved.
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
#include "TypeSet.h"

#include "InspectorProtocolObjects.h"
#include "JSCInlines.h"
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/Vector.h>

namespace JSC {

TypeSet::TypeSet()
    : m_isOverflown(false)
    , m_seenTypes(TypeNothing)
{
}

void TypeSet::addTypeInformation(RuntimeType type, RefPtr<StructureShape>&& passedNewShape, Structure* structure)
{
    m_seenTypes = m_seenTypes | type;

    if (structure && passedNewShape && !runtimeTypeIsPrimitive(type)) {
        Ref<StructureShape> newShape = passedNewShape.releaseNonNull();
        if (!m_structureSet.contains(structure)) {
            {
                ConcurrentJSLocker locker(m_lock);
                m_structureSet.add(structure);
            }
            // Make one more pass making sure that: 
            // - We don't have two instances of the same shape. (Same shapes may have different Structures).
            // - We don't have two shapes that share the same prototype chain. If these shapes share the same 
            //   prototype chain, they will be merged into one shape.
            String hash = newShape->propertyHash();
            for (auto& seenShape : m_structureHistory) {
                if (seenShape->propertyHash() == hash)
                    return;
                if (seenShape->hasSamePrototypeChain(newShape.get())) {
                    seenShape = StructureShape::merge(seenShape.copyRef(), WTFMove(newShape));
                    return;
                }
            }

            if (m_structureHistory.size() < 100) {
                m_structureHistory.append(WTFMove(newShape));
                return;
            }
            if (!m_isOverflown)
                m_isOverflown = true;
        }
    }
}

void TypeSet::invalidateCache()
{
    ConcurrentJSLocker locker(m_lock);
    auto keepMarkedStructuresFilter = [] (Structure* structure) -> bool { return Heap::isMarked(structure); };
    m_structureSet.genericFilter(keepMarkedStructuresFilter);
}

String TypeSet::dumpTypes() const
{
    if (m_seenTypes == TypeNothing)
        return ASCIILiteral("(Unreached Statement)");

    StringBuilder seen;

    if (m_seenTypes & TypeFunction)
        seen.appendLiteral("Function ");
    if (m_seenTypes & TypeUndefined)
        seen.appendLiteral("Undefined ");
    if (m_seenTypes & TypeNull)
        seen.appendLiteral("Null ");
    if (m_seenTypes & TypeBoolean)
        seen.appendLiteral("Boolean ");
    if (m_seenTypes & TypeAnyInt)
        seen.appendLiteral("AnyInt ");
    if (m_seenTypes & TypeNumber)
        seen.appendLiteral("Number ");
    if (m_seenTypes & TypeString)
        seen.appendLiteral("String ");
    if (m_seenTypes & TypeObject)
        seen.appendLiteral("Object ");
    if (m_seenTypes & TypeSymbol)
        seen.appendLiteral("Symbol ");

    for (const auto& shape : m_structureHistory) {
        seen.append(shape->m_constructorName);
        seen.append(' ');
    }

    if (m_structureHistory.size()) 
        seen.appendLiteral("\nStructures:[ ");
    for (const auto& shape : m_structureHistory) {
        seen.append(shape->stringRepresentation());
        seen.append(' ');
    }
    if (m_structureHistory.size())
        seen.append(']');

    if (m_structureHistory.size()) {
        seen.appendLiteral("\nLeast Common Ancestor: ");
        seen.append(leastCommonAncestor());
    }

    return seen.toString();
}

bool TypeSet::doesTypeConformTo(RuntimeTypeMask test) const
{
    // This function checks if our seen types conform  to the types described by the test bitstring. (i.e we haven't seen more types than test).
    // We are <= to those types if ANDing with the bitstring doesn't zero out any of our bits.

    // For example:

    // 0b0110 (seen)
    // 0b1111 (test)
    // ------ (AND)
    // 0b0110 == seen

    // 0b0110 (seen)
    // 0b0010 (test)
    // ------ (AND)
    // 0b0010 != seen

    return m_seenTypes != TypeNothing && (m_seenTypes & test) == m_seenTypes;
}

String TypeSet::displayName() const
{
    if (m_seenTypes == TypeNothing)
        return emptyString();

    if (m_structureHistory.size() && doesTypeConformTo(TypeObject | TypeNull | TypeUndefined)) {
        String ctorName = leastCommonAncestor(); 

        if (doesTypeConformTo(TypeObject))
            return ctorName;
        if (doesTypeConformTo(TypeObject | TypeNull | TypeUndefined))
            return ctorName + '?';
    }

    // The order of these checks are important. For example, if a value is only a function, it conforms to TypeFunction, but it also conforms to TypeFunction | TypeNull.
    // Therefore, more specific types must be checked first.

    if (doesTypeConformTo(TypeFunction))
        return ASCIILiteral("Function");
    if (doesTypeConformTo(TypeUndefined))
        return ASCIILiteral("Undefined");
    if (doesTypeConformTo(TypeNull))
        return ASCIILiteral("Null");
    if (doesTypeConformTo(TypeBoolean))
        return ASCIILiteral("Boolean");
    if (doesTypeConformTo(TypeAnyInt))
        return ASCIILiteral("Integer");
    if (doesTypeConformTo(TypeNumber | TypeAnyInt))
        return ASCIILiteral("Number");
    if (doesTypeConformTo(TypeString))
        return ASCIILiteral("String");
    if (doesTypeConformTo(TypeSymbol))
        return ASCIILiteral("Symbol");

    if (doesTypeConformTo(TypeNull | TypeUndefined))
        return ASCIILiteral("(?)");

    if (doesTypeConformTo(TypeFunction | TypeNull | TypeUndefined))
        return ASCIILiteral("Function?");
    if (doesTypeConformTo(TypeBoolean | TypeNull | TypeUndefined))
        return ASCIILiteral("Boolean?");
    if (doesTypeConformTo(TypeAnyInt | TypeNull | TypeUndefined))
        return ASCIILiteral("Integer?");
    if (doesTypeConformTo(TypeNumber | TypeAnyInt | TypeNull | TypeUndefined))
        return ASCIILiteral("Number?");
    if (doesTypeConformTo(TypeString | TypeNull | TypeUndefined))
        return ASCIILiteral("String?");
    if (doesTypeConformTo(TypeSymbol | TypeNull | TypeUndefined))
        return ASCIILiteral("Symbol?");
   
    if (doesTypeConformTo(TypeObject | TypeFunction | TypeString))
        return ASCIILiteral("Object");
    if (doesTypeConformTo(TypeObject | TypeFunction | TypeString | TypeNull | TypeUndefined))
        return ASCIILiteral("Object?");

    return ASCIILiteral("(many)");
}

String TypeSet::leastCommonAncestor() const
{
    return StructureShape::leastCommonAncestor(m_structureHistory);
}

Ref<Inspector::Protocol::Array<Inspector::Protocol::Runtime::StructureDescription>> TypeSet::allStructureRepresentations() const
{
    auto description = Inspector::Protocol::Array<Inspector::Protocol::Runtime::StructureDescription>::create();

    for (auto& shape : m_structureHistory)
        description->addItem(shape->inspectorRepresentation());

    return description;
}

Ref<Inspector::Protocol::Runtime::TypeSet> TypeSet::inspectorTypeSet() const
{
    return Inspector::Protocol::Runtime::TypeSet::create()
        .setIsFunction((m_seenTypes & TypeFunction) != TypeNothing)
        .setIsUndefined((m_seenTypes & TypeUndefined) != TypeNothing)
        .setIsNull((m_seenTypes & TypeNull) != TypeNothing)
        .setIsBoolean((m_seenTypes & TypeBoolean) != TypeNothing)
        .setIsInteger((m_seenTypes & TypeAnyInt) != TypeNothing)
        .setIsNumber((m_seenTypes & TypeNumber) != TypeNothing)
        .setIsString((m_seenTypes & TypeString) != TypeNothing)
        .setIsObject((m_seenTypes & TypeObject) != TypeNothing)
        .setIsSymbol((m_seenTypes & TypeSymbol) != TypeNothing)
        .release();
}

String TypeSet::toJSONString() const
{
    // This returns a JSON string representing an Object with the following properties:
    //     displayTypeName: 'String'
    //     primitiveTypeNames: 'Array<String>'
    //     structures: 'Array<JSON<StructureShape>>'

    StringBuilder json;
    json.append('{');

    json.appendLiteral("\"displayTypeName\":");
    json.append('"');
    json.append(displayName());
    json.append('"');
    json.append(',');

    json.appendLiteral("\"primitiveTypeNames\":");
    json.append('[');
    bool hasAnItem = false;
    if (m_seenTypes & TypeUndefined) {
        hasAnItem = true;
        json.appendLiteral("\"Undefined\"");
    }
    if (m_seenTypes & TypeNull) {
        if (hasAnItem)
            json.append(',');
        hasAnItem = true;
        json.appendLiteral("\"Null\"");
    }
    if (m_seenTypes & TypeBoolean) {
        if (hasAnItem)
            json.append(',');
        hasAnItem = true;
        json.appendLiteral("\"Boolean\"");
    }
    if (m_seenTypes & TypeAnyInt) {
        if (hasAnItem)
            json.append(',');
        hasAnItem = true;
        json.appendLiteral("\"Integer\"");
    }
    if (m_seenTypes & TypeNumber) {
        if (hasAnItem)
            json.append(',');
        hasAnItem = true;
        json.appendLiteral("\"Number\"");
    }
    if (m_seenTypes & TypeString) {
        if (hasAnItem)
            json.append(',');
        hasAnItem = true;
        json.appendLiteral("\"String\"");
    }
    if (m_seenTypes & TypeSymbol) {
        if (hasAnItem)
            json.append(',');
        hasAnItem = true;
        json.appendLiteral("\"Symbol\"");
    }
    json.append(']');

    json.append(',');

    json.appendLiteral("\"structures\":");
    json.append('[');
    hasAnItem = false;
    for (size_t i = 0; i < m_structureHistory.size(); i++) {
        if (hasAnItem)
            json.append(',');
        hasAnItem = true;
        json.append(m_structureHistory[i]->toJSONString());
    }
    json.append(']');

    json.append('}');
    return json.toString();
}

StructureShape::StructureShape()
    : m_proto(nullptr)
    , m_propertyHash(nullptr)
    , m_final(false)
    , m_isInDictionaryMode(false)
{
}

void StructureShape::markAsFinal()
{
    ASSERT(!m_final);
    m_final = true;
}

void StructureShape::addProperty(UniquedStringImpl& uid)
{
    ASSERT(!m_final);
    m_fields.add(&uid);
}

String StructureShape::propertyHash()
{
    ASSERT(m_final);
    if (m_propertyHash)
        return *m_propertyHash;

    StringBuilder builder;
    builder.append(':');
    builder.append(m_constructorName);
    builder.append(':');
    for (auto& key : m_fields) {
        String property = key.get();
        property.replace(":", "\\:"); // Ensure that hash({"foo:", "bar"}) != hash({"foo", ":bar"}) because we're using colons as a separator and colons are legal characters in field names in JS.
        builder.append(property);
    }

    if (m_proto) {
        builder.append(':');
        builder.appendLiteral("__proto__");
        builder.append(m_proto->propertyHash());
    }

    m_propertyHash = std::make_unique<String>(builder.toString());
    return *m_propertyHash;
}

String StructureShape::leastCommonAncestor(const Vector<Ref<StructureShape>>& shapes)
{
    if (shapes.isEmpty())
        return emptyString();

    StructureShape* origin = shapes[0].ptr();
    for (size_t i = 1; i < shapes.size(); i++) {
        bool foundLUB = false;
        while (!foundLUB) {
            StructureShape* check = shapes[i].ptr();
            String curCtorName = origin->m_constructorName;
            while (check) {
                if (check->m_constructorName == curCtorName) {
                    foundLUB = true;
                    break;
                }
                check = check->m_proto.get();
            }
            if (!foundLUB) {
                // This is unlikely to happen, because we usually bottom out at "Object", but there are some sets of Objects
                // that may cause this behavior. We fall back to "Object" because it's our version of Top.
                if (!origin->m_proto)
                    return ASCIILiteral("Object");
                origin = origin->m_proto.get();
            }
        }

        if (origin->m_constructorName == "Object")
            break;
    }

    return origin->m_constructorName;
}

String StructureShape::stringRepresentation()
{
    StringBuilder representation;
    RefPtr<StructureShape> curShape = this;

    representation.append('{');
    while (curShape) {
        for (auto it = curShape->m_fields.begin(), end = curShape->m_fields.end(); it != end; ++it) {
            String prop((*it).get());
            representation.append(prop);
            representation.appendLiteral(", ");
        }

        if (curShape->m_proto) {
            representation.appendLiteral("__proto__ [");
            representation.append(curShape->m_proto->m_constructorName);
            representation.appendLiteral("], ");
        }

        curShape = curShape->m_proto;
    }

    if (representation.length() >= 3)
        representation.resize(representation.length() - 2);

    representation.append('}');

    return representation.toString();
}

String StructureShape::toJSONString() const
{
    // This returns a JSON string representing an Object with the following properties:
    //     constructorName: 'String'
    //     fields: 'Array<String>'
    //     optionalFields: 'Array<String>'
    //     proto: 'JSON<StructureShape> | null'

    StringBuilder json;
    json.append('{');

    json.appendLiteral("\"constructorName\":");
    json.append('"');
    json.append(m_constructorName);
    json.append('"');
    json.append(',');

    json.appendLiteral("\"isInDictionaryMode\":");
    if (m_isInDictionaryMode)
        json.appendLiteral("true");
    else
        json.appendLiteral("false");
    json.append(',');

    json.appendLiteral("\"fields\":");
    json.append('[');
    bool hasAnItem = false;
    for (auto it = m_fields.begin(), end = m_fields.end(); it != end; ++it) {
        if (hasAnItem)
            json.append(',');
        hasAnItem = true;

        String fieldName((*it).get());
        json.append('"');
        json.append(fieldName);
        json.append('"');
    }
    json.append(']');
    json.append(',');

    json.appendLiteral("\"optionalFields\":");
    json.append('[');
    hasAnItem = false;
    for (auto it = m_optionalFields.begin(), end = m_optionalFields.end(); it != end; ++it) {
        if (hasAnItem)
            json.append(',');
        hasAnItem = true;

        String fieldName((*it).get());
        json.append('"');
        json.append(fieldName);
        json.append('"');
    }
    json.append(']');
    json.append(',');

    json.appendLiteral("\"proto\":");
    if (m_proto)
        json.append(m_proto->toJSONString());
    else
        json.appendLiteral("null");

    json.append('}');

    return json.toString();
}

Ref<Inspector::Protocol::Runtime::StructureDescription> StructureShape::inspectorRepresentation()
{
    auto base = Inspector::Protocol::Runtime::StructureDescription::create().release();
    Ref<Inspector::Protocol::Runtime::StructureDescription> currentObject = base.copyRef();
    RefPtr<StructureShape> currentShape(this);

    while (currentShape) {
        auto fields = Inspector::Protocol::Array<String>::create();
        auto optionalFields = Inspector::Protocol::Array<String>::create();
        for (auto field : currentShape->m_fields)
            fields->addItem(field.get());
        for (auto field : currentShape->m_optionalFields)
            optionalFields->addItem(field.get());

        currentObject->setFields(&fields.get());
        currentObject->setOptionalFields(&optionalFields.get());
        currentObject->setConstructorName(currentShape->m_constructorName);
        currentObject->setIsImprecise(currentShape->m_isInDictionaryMode);

        if (currentShape->m_proto) {
            auto nextObject = Inspector::Protocol::Runtime::StructureDescription::create().release();
            currentObject->setPrototypeStructure(&nextObject.get());
            currentObject = WTFMove(nextObject);
        }

        currentShape = currentShape->m_proto;
    }

    return base;
}

bool StructureShape::hasSamePrototypeChain(const StructureShape& otherRef)
{
    const StructureShape* self = this;
    const StructureShape* other = &otherRef;
    while (self && other) {
        if (self->m_constructorName != other->m_constructorName)
            return false;
        self = self->m_proto.get();
        other = other->m_proto.get();
    }

    return !self && !other;
}

Ref<StructureShape> StructureShape::merge(Ref<StructureShape>&& a, Ref<StructureShape>&& b)
{
    ASSERT(a->hasSamePrototypeChain(b.get()));

    auto merged = StructureShape::create();
    for (auto field : a->m_fields) {
        if (b->m_fields.contains(field))
            merged->m_fields.add(field);
        else
            merged->m_optionalFields.add(field);
    }

    for (auto field : b->m_fields) {
        if (!merged->m_fields.contains(field)) {
            auto addResult = merged->m_optionalFields.add(field);
            ASSERT_UNUSED(addResult, addResult.isNewEntry);
        }
    }

    for (auto field : a->m_optionalFields)
        merged->m_optionalFields.add(field);
    for (auto field : b->m_optionalFields)
        merged->m_optionalFields.add(field);

    ASSERT(a->m_constructorName == b->m_constructorName);
    merged->setConstructorName(a->m_constructorName);

    if (a->m_proto) {
        RELEASE_ASSERT(b->m_proto);
        merged->setProto(StructureShape::merge(*a->m_proto, *b->m_proto));
    }

    merged->markAsFinal();

    return merged;
}

void StructureShape::enterDictionaryMode()
{
    m_isInDictionaryMode = true;
}

} // namespace JSC
