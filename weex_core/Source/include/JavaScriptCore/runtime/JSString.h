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

#include "CallFrame.h"
#include "CommonIdentifiers.h"
#include "Identifier.h"
#include "PropertyDescriptor.h"
#include "PropertySlot.h"
#include "Structure.h"
#include <array>
#include <wtf/text/StringView.h>

namespace JSC {

class JSString;
class JSRopeString;
class LLIntOffsetsExtractor;

JSString* jsEmptyString(VM*);
JSString* jsEmptyString(ExecState*);
JSString* jsString(VM*, const String&); // returns empty string if passed null string
JSString* jsString(ExecState*, const String&); // returns empty string if passed null string

JSString* jsSingleCharacterString(VM*, UChar);
JSString* jsSingleCharacterString(ExecState*, UChar);
JSString* jsSubstring(VM*, const String&, unsigned offset, unsigned length);
JSString* jsSubstring(ExecState*, const String&, unsigned offset, unsigned length);

// Non-trivial strings are two or more characters long.
// These functions are faster than just calling jsString.
JSString* jsNontrivialString(VM*, const String&);
JSString* jsNontrivialString(ExecState*, const String&);
JSString* jsNontrivialString(ExecState*, String&&);

// Should be used for strings that are owned by an object that will
// likely outlive the JSValue this makes, such as the parse tree or a
// DOM object that contains a String
JSString* jsOwnedString(VM*, const String&);
JSString* jsOwnedString(ExecState*, const String&);

JSRopeString* jsStringBuilder(VM*);

bool isJSString(JSCell*);
bool isJSString(JSValue);
JSString* asString(JSValue);

struct StringViewWithUnderlyingString {
    StringView view;
    String underlyingString;
};

class JSString : public JSCell {
public:
    friend class JIT;
    friend class VM;
    friend class SpecializedThunkJIT;
    friend class JSRopeString;
    friend class MarkStack;
    friend class SlotVisitor;
    friend struct ThunkHelpers;

    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | InterceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero | StructureIsImmortal | OverridesToThis;

    static const bool needsDestruction = true;
    static void destroy(JSCell*);
    
    // We specialize the string subspace to get the fastest possible sweep. This wouldn't be
    // necessary if JSString didn't have a destructor.
    template<typename>
    static Subspace* subspaceFor(VM& vm)
    {
        return &vm.stringSpace;
    }
    
    static const unsigned MaxLength = std::numeric_limits<int32_t>::max();
    
private:
    JSString(VM& vm, Ref<StringImpl>&& value)
        : JSCell(vm, vm.stringStructure.get())
        , m_flags(0)
        , m_value(WTFMove(value))
    {
    }

    JSString(VM& vm)
        : JSCell(vm, vm.stringStructure.get())
        , m_flags(0)
    {
    }

    void finishCreation(VM& vm, size_t length)
    {
        ASSERT(!m_value.isNull());
        Base::finishCreation(vm);
        setLength(length);
        setIs8Bit(m_value.impl()->is8Bit());
    }

    void finishCreation(VM& vm, size_t length, size_t cost)
    {
        ASSERT(!m_value.isNull());
        Base::finishCreation(vm);
        setLength(length);
        setIs8Bit(m_value.impl()->is8Bit());
        Heap::heap(this)->reportExtraMemoryAllocated(cost);
    }

protected:
    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
        setLength(0);
        setIs8Bit(true);
    }

public:
    static JSString* create(VM& vm, Ref<StringImpl>&& value)
    {
        unsigned length = value->length();
        size_t cost = value->cost();
        JSString* newString = new (NotNull, allocateCell<JSString>(vm.heap)) JSString(vm, WTFMove(value));
        newString->finishCreation(vm, length, cost);
        return newString;
    }
    static JSString* createHasOtherOwner(VM& vm, Ref<StringImpl>&& value)
    {
        size_t length = value->length();
        JSString* newString = new (NotNull, allocateCell<JSString>(vm.heap)) JSString(vm, WTFMove(value));
        newString->finishCreation(vm, length);
        return newString;
    }

    Identifier toIdentifier(ExecState*) const;
    AtomicString toAtomicString(ExecState*) const;
    RefPtr<AtomicStringImpl> toExistingAtomicString(ExecState*) const;

    StringViewWithUnderlyingString viewWithUnderlyingString(ExecState&) const;

    inline bool equal(ExecState*, JSString* other) const;
    const String& value(ExecState*) const;
    const String& tryGetValue() const;
    const StringImpl* tryGetValueImpl() const;
    ALWAYS_INLINE unsigned length() const { return m_length; }
    ALWAYS_INLINE static bool isValidLength(size_t length)
    {
        // While length is of type unsigned, the runtime and compilers are all
        // expecting that m_length is a positive value <= INT_MAX.
        // FIXME: Look into making the max length UINT_MAX to match StringImpl's max length.
        // https://bugs.webkit.org/show_bug.cgi?id=163955
        return length <= std::numeric_limits<int32_t>::max();
    }

    JSValue toPrimitive(ExecState*, PreferredPrimitiveType) const;
    bool toBoolean() const { return !!length(); }
    bool getPrimitiveNumber(ExecState*, double& number, JSValue&) const;
    JSObject* toObject(ExecState*, JSGlobalObject*) const;
    double toNumber(ExecState*) const;

    bool getStringPropertySlot(ExecState*, PropertyName, PropertySlot&);
    bool getStringPropertySlot(ExecState*, unsigned propertyName, PropertySlot&);
    bool getStringPropertyDescriptor(ExecState*, PropertyName, PropertyDescriptor&);

    bool canGetIndex(unsigned i) { return i < length(); }
    JSString* getIndex(ExecState*, unsigned);

    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    static size_t offsetOfLength() { return OBJECT_OFFSETOF(JSString, m_length); }
    static size_t offsetOfFlags() { return OBJECT_OFFSETOF(JSString, m_flags); }
    static size_t offsetOfValue() { return OBJECT_OFFSETOF(JSString, m_value); }

    DECLARE_EXPORT_INFO;

    static void dumpToStream(const JSCell*, PrintStream&);
    static size_t estimatedSize(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

    enum {
        Is8Bit = 1u
    };

protected:
    friend class JSValue;

    JS_EXPORT_PRIVATE bool equalSlowCase(ExecState*, JSString* other) const;
    bool isRope() const { return m_value.isNull(); }
    bool isSubstring() const;
    bool is8Bit() const { return m_flags & Is8Bit; }
    void setIs8Bit(bool flag) const
    {
        if (flag)
            m_flags |= Is8Bit;
        else
            m_flags &= ~Is8Bit;
    }

    ALWAYS_INLINE void setLength(unsigned length)
    {
        RELEASE_ASSERT(isValidLength(length));
        m_length = length;
    }

private:
    mutable unsigned m_flags;

    // A string is represented either by a String or a rope of fibers.
    unsigned m_length;
    mutable String m_value;

    friend class LLIntOffsetsExtractor;

    static JSValue toThis(JSCell*, ExecState*, ECMAMode);

    String& string() { ASSERT(!isRope()); return m_value; }
    StringView unsafeView(ExecState&) const;

    friend JSString* jsString(ExecState*, JSString*, JSString*);
    friend JSString* jsSubstring(ExecState*, JSString*, unsigned offset, unsigned length);
};

// NOTE: This class cannot override JSString's destructor. JSString's destructor is called directly
// from JSStringSubspace::
class JSRopeString final : public JSString {
    friend class JSString;

    friend JSRopeString* jsStringBuilder(VM*);

public:
    class RopeBuilder {
    public:
        RopeBuilder(VM& vm)
            : m_vm(vm)
            , m_jsString(jsStringBuilder(&vm))
            , m_index(0)
        {
        }

        bool append(JSString* jsString)
        {
            if (m_index == JSRopeString::s_maxInternalRopeLength)
                expand();
            if (static_cast<int32_t>(m_jsString->length() + jsString->length()) < 0) {
                m_jsString = nullptr;
                return false;
            }
            m_jsString->append(m_vm, m_index++, jsString);
            return true;
        }

        JSRopeString* release()
        {
            RELEASE_ASSERT(m_jsString);
            JSRopeString* tmp = m_jsString;
            m_jsString = 0;
            return tmp;
        }

        unsigned length() const { return m_jsString->length(); }

    private:
        void expand();

        VM& m_vm;
        JSRopeString* m_jsString;
        size_t m_index;
    };

private:
    ALWAYS_INLINE JSRopeString(VM& vm)
        : JSString(vm)
    {
    }

    void finishCreation(VM& vm, JSString* s1, JSString* s2)
    {
        Base::finishCreation(vm);
        ASSERT(!sumOverflows<int32_t>(s1->length(), s2->length()));
        setLength(s1->length() + s2->length());
        setIs8Bit(s1->is8Bit() && s2->is8Bit());
        setIsSubstring(false);
        fiber(0).set(vm, this, s1);
        fiber(1).set(vm, this, s2);
        fiber(2).clear();
    }

    void finishCreation(VM& vm, JSString* s1, JSString* s2, JSString* s3)
    {
        Base::finishCreation(vm);
        ASSERT(!sumOverflows<int32_t>(s1->length(), s2->length(), s3->length()));
        setLength(s1->length() + s2->length() + s3->length());
        setIs8Bit(s1->is8Bit() && s2->is8Bit() &&  s3->is8Bit());
        setIsSubstring(false);
        fiber(0).set(vm, this, s1);
        fiber(1).set(vm, this, s2);
        fiber(2).set(vm, this, s3);
    }

    void finishCreation(VM& vm, ExecState* exec, JSString* base, unsigned offset, unsigned length)
    {
        Base::finishCreation(vm);
        RELEASE_ASSERT(!sumOverflows<int32_t>(offset, length));
        RELEASE_ASSERT(offset + length <= base->length());
        setLength(length);
        setIs8Bit(base->is8Bit());
        setIsSubstring(true);
        if (base->isSubstring()) {
            JSRopeString* baseRope = jsCast<JSRopeString*>(base);
            substringBase().set(vm, this, baseRope->substringBase().get());
            substringOffset() = baseRope->substringOffset() + offset;
        } else {
            substringBase().set(vm, this, base);
            substringOffset() = offset;

            // For now, let's not allow substrings with a rope base.
            // Resolve non-substring rope bases so we don't have to deal with it.
            // FIXME: Evaluate if this would be worth adding more branches.
            if (base->isRope())
                jsCast<JSRopeString*>(base)->resolveRope(exec);
        }
    }

    ALWAYS_INLINE void finishCreationSubstringOfResolved(VM& vm, JSString* base, unsigned offset, unsigned length)
    {
        Base::finishCreation(vm);
        RELEASE_ASSERT(!sumOverflows<int32_t>(offset, length));
        RELEASE_ASSERT(offset + length <= base->length());
        setLength(length);
        setIs8Bit(base->is8Bit());
        setIsSubstring(true);
        substringBase().set(vm, this, base);
        substringOffset() = offset;
    }

    void finishCreation(VM& vm)
    {
        JSString::finishCreation(vm);
        setIsSubstring(false);
        fiber(0).clear();
        fiber(1).clear();
        fiber(2).clear();
    }

    void append(VM& vm, size_t index, JSString* jsString)
    {
        fiber(index).set(vm, this, jsString);
        setLength(length() + jsString->length());
        setIs8Bit(is8Bit() && jsString->is8Bit());
    }

    static JSRopeString* createNull(VM& vm)
    {
        JSRopeString* newString = new (NotNull, allocateCell<JSRopeString>(vm.heap)) JSRopeString(vm);
        newString->finishCreation(vm);
        return newString;
    }

public:
    static JSString* create(VM& vm, ExecState* exec, JSString* base, unsigned offset, unsigned length)
    {
        JSRopeString* newString = new (NotNull, allocateCell<JSRopeString>(vm.heap)) JSRopeString(vm);
        newString->finishCreation(vm, exec, base, offset, length);
        return newString;
    }

    ALWAYS_INLINE static JSString* createSubstringOfResolved(VM& vm, GCDeferralContext* deferralContext, JSString* base, unsigned offset, unsigned length)
    {
        JSRopeString* newString = new (NotNull, allocateCell<JSRopeString>(vm.heap, deferralContext)) JSRopeString(vm);
        newString->finishCreationSubstringOfResolved(vm, base, offset, length);
        return newString;
    }

    ALWAYS_INLINE static JSString* createSubstringOfResolved(VM& vm, JSString* base, unsigned offset, unsigned length)
    {
        return createSubstringOfResolved(vm, nullptr, base, offset, length);
    }

    void visitFibers(SlotVisitor&);

    static ptrdiff_t offsetOfFibers() { return OBJECT_OFFSETOF(JSRopeString, u); }

    static const unsigned s_maxInternalRopeLength = 3;

private:
    static JSString* create(VM& vm, JSString* s1, JSString* s2)
    {
        JSRopeString* newString = new (NotNull, allocateCell<JSRopeString>(vm.heap)) JSRopeString(vm);
        newString->finishCreation(vm, s1, s2);
        return newString;
    }
    static JSString* create(VM& vm, JSString* s1, JSString* s2, JSString* s3)
    {
        JSRopeString* newString = new (NotNull, allocateCell<JSRopeString>(vm.heap)) JSRopeString(vm);
        newString->finishCreation(vm, s1, s2, s3);
        return newString;
    }

    friend JSValue jsStringFromRegisterArray(ExecState*, Register*, unsigned);
    friend JSValue jsStringFromArguments(ExecState*, JSValue);

    JS_EXPORT_PRIVATE void resolveRope(ExecState*) const;
    JS_EXPORT_PRIVATE void resolveRopeToAtomicString(ExecState*) const;
    JS_EXPORT_PRIVATE RefPtr<AtomicStringImpl> resolveRopeToExistingAtomicString(ExecState*) const;
    void resolveRopeSlowCase8(LChar*) const;
    void resolveRopeSlowCase(UChar*) const;
    void outOfMemory(ExecState*) const;
    void resolveRopeInternal8(LChar*) const;
    void resolveRopeInternal8NoSubstring(LChar*) const;
    void resolveRopeInternal16(UChar*) const;
    void resolveRopeInternal16NoSubstring(UChar*) const;
    void clearFibers() const;
    StringView unsafeView(ExecState&) const;
    StringViewWithUnderlyingString viewWithUnderlyingString(ExecState&) const;

    WriteBarrierBase<JSString>& fiber(unsigned i) const
    {
        ASSERT(!isSubstring());
        ASSERT(i < s_maxInternalRopeLength);
        return u[i].string;
    }

    WriteBarrierBase<JSString>& substringBase() const
    {
        return u[1].string;
    }

    uintptr_t& substringOffset() const
    {
        return u[2].number;
    }

    static uintptr_t notSubstringSentinel()
    {
        return 0;
    }

    static uintptr_t substringSentinel()
    {
        return 1;
    }

    bool isSubstring() const
    {
        return u[0].number == substringSentinel();
    }

    void setIsSubstring(bool isSubstring)
    {
        u[0].number = isSubstring ? substringSentinel() : notSubstringSentinel();
    }

    mutable union {
        uintptr_t number;
        WriteBarrierBase<JSString> string;
    } u[s_maxInternalRopeLength];


    friend JSString* jsString(ExecState*, JSString*, JSString*);
    friend JSString* jsString(ExecState*, JSString*, JSString*, JSString*);
    friend JSString* jsString(ExecState*, const String&, const String&, const String&);
};

JS_EXPORT_PRIVATE JSString* jsStringWithCacheSlowCase(VM&, StringImpl&);

inline const StringImpl* JSString::tryGetValueImpl() const
{
    return m_value.impl();
}

inline JSString* asString(JSValue value)
{
    ASSERT(value.asCell()->isString());
    return jsCast<JSString*>(value.asCell());
}

// This MUST NOT GC.
inline JSString* jsEmptyString(VM* vm)
{
    return vm->smallStrings.emptyString();
}

ALWAYS_INLINE JSString* jsSingleCharacterString(VM* vm, UChar c)
{
    if (c <= maxSingleCharacterString)
        return vm->smallStrings.singleCharacterString(c);
    return JSString::create(*vm, StringImpl::create(&c, 1));
}

inline JSString* jsNontrivialString(VM* vm, const String& s)
{
    ASSERT(s.length() > 1);
    return JSString::create(*vm, *s.impl());
}

inline JSString* jsNontrivialString(VM* vm, String&& s)
{
    ASSERT(s.length() > 1);
    return JSString::create(*vm, s.releaseImpl().releaseNonNull());
}

ALWAYS_INLINE Identifier JSString::toIdentifier(ExecState* exec) const
{
    return Identifier::fromString(exec, toAtomicString(exec));
}

ALWAYS_INLINE AtomicString JSString::toAtomicString(ExecState* exec) const
{
    if (isRope())
        static_cast<const JSRopeString*>(this)->resolveRopeToAtomicString(exec);
    return AtomicString(m_value);
}

ALWAYS_INLINE RefPtr<AtomicStringImpl> JSString::toExistingAtomicString(ExecState* exec) const
{
    if (isRope())
        return static_cast<const JSRopeString*>(this)->resolveRopeToExistingAtomicString(exec);
    if (m_value.impl()->isAtomic())
        return static_cast<AtomicStringImpl*>(m_value.impl());
    return AtomicStringImpl::lookUp(m_value.impl());
}

inline const String& JSString::value(ExecState* exec) const
{
    if (isRope())
        static_cast<const JSRopeString*>(this)->resolveRope(exec);
    return m_value;
}

inline const String& JSString::tryGetValue() const
{
    if (isRope())
        static_cast<const JSRopeString*>(this)->resolveRope(0);
    return m_value;
}

inline JSString* JSString::getIndex(ExecState* exec, unsigned i)
{
    ASSERT(canGetIndex(i));
    return jsSingleCharacterString(exec, unsafeView(*exec)[i]);
}

inline JSString* jsString(VM* vm, const String& s)
{
    int size = s.length();
    if (!size)
        return vm->smallStrings.emptyString();
    if (size == 1) {
        UChar c = s.characterAt(0);
        if (c <= maxSingleCharacterString)
            return vm->smallStrings.singleCharacterString(c);
    }
    return JSString::create(*vm, *s.impl());
}

inline JSString* jsSubstring(VM& vm, ExecState* exec, JSString* s, unsigned offset, unsigned length)
{
    ASSERT(offset <= static_cast<unsigned>(s->length()));
    ASSERT(length <= static_cast<unsigned>(s->length()));
    ASSERT(offset + length <= static_cast<unsigned>(s->length()));
    if (!length)
        return vm.smallStrings.emptyString();
    if (!offset && length == s->length())
        return s;
    return JSRopeString::create(vm, exec, s, offset, length);
}

inline JSString* jsSubstringOfResolved(VM& vm, GCDeferralContext* deferralContext, JSString* s, unsigned offset, unsigned length)
{
    ASSERT(offset <= static_cast<unsigned>(s->length()));
    ASSERT(length <= static_cast<unsigned>(s->length()));
    ASSERT(offset + length <= static_cast<unsigned>(s->length()));
    if (!length)
        return vm.smallStrings.emptyString();
    if (!offset && length == s->length())
        return s;
    return JSRopeString::createSubstringOfResolved(vm, deferralContext, s, offset, length);
}

inline JSString* jsSubstringOfResolved(VM& vm, JSString* s, unsigned offset, unsigned length)
{
    return jsSubstringOfResolved(vm, nullptr, s, offset, length);
}

inline JSString* jsSubstring(ExecState* exec, JSString* s, unsigned offset, unsigned length)
{
    return jsSubstring(exec->vm(), exec, s, offset, length);
}

inline JSString* jsSubstring(VM* vm, const String& s, unsigned offset, unsigned length)
{
    ASSERT(offset <= static_cast<unsigned>(s.length()));
    ASSERT(length <= static_cast<unsigned>(s.length()));
    ASSERT(offset + length <= static_cast<unsigned>(s.length()));
    if (!length)
        return vm->smallStrings.emptyString();
    if (length == 1) {
        UChar c = s.characterAt(offset);
        if (c <= maxSingleCharacterString)
            return vm->smallStrings.singleCharacterString(c);
    }
    return JSString::createHasOtherOwner(*vm, StringImpl::createSubstringSharingImpl(*s.impl(), offset, length));
}

inline JSString* jsOwnedString(VM* vm, const String& s)
{
    int size = s.length();
    if (!size)
        return vm->smallStrings.emptyString();
    if (size == 1) {
        UChar c = s.characterAt(0);
        if (c <= maxSingleCharacterString)
            return vm->smallStrings.singleCharacterString(c);
    }
    return JSString::createHasOtherOwner(*vm, *s.impl());
}

inline JSRopeString* jsStringBuilder(VM* vm)
{
    return JSRopeString::createNull(*vm);
}

inline JSString* jsEmptyString(ExecState* exec) { return jsEmptyString(&exec->vm()); }
inline JSString* jsString(ExecState* exec, const String& s) { return jsString(&exec->vm(), s); }
inline JSString* jsSingleCharacterString(ExecState* exec, UChar c) { return jsSingleCharacterString(&exec->vm(), c); }
inline JSString* jsSubstring(ExecState* exec, const String& s, unsigned offset, unsigned length) { return jsSubstring(&exec->vm(), s, offset, length); }
inline JSString* jsNontrivialString(ExecState* exec, const String& s) { return jsNontrivialString(&exec->vm(), s); }
inline JSString* jsNontrivialString(ExecState* exec, String&& s) { return jsNontrivialString(&exec->vm(), WTFMove(s)); }
inline JSString* jsOwnedString(ExecState* exec, const String& s) { return jsOwnedString(&exec->vm(), s); }

ALWAYS_INLINE JSString* jsStringWithCache(ExecState* exec, const String& s)
{
    VM& vm = exec->vm();
    StringImpl* stringImpl = s.impl();
    if (!stringImpl || !stringImpl->length())
        return jsEmptyString(&vm);

    if (stringImpl->length() == 1) {
        UChar singleCharacter = (*stringImpl)[0u];
        if (singleCharacter <= maxSingleCharacterString)
            return vm.smallStrings.singleCharacterString(static_cast<unsigned char>(singleCharacter));
    }

    if (JSString* lastCachedString = vm.lastCachedString.get()) {
        if (lastCachedString->tryGetValueImpl() == stringImpl)
            return lastCachedString;
    }

    return jsStringWithCacheSlowCase(vm, *stringImpl);
}

ALWAYS_INLINE bool JSString::getStringPropertySlot(ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    if (propertyName == exec->propertyNames().length) {
        slot.setValue(this, DontEnum | DontDelete | ReadOnly, jsNumber(length()));
        return true;
    }

    std::optional<uint32_t> index = parseIndex(propertyName);
    if (index && index.value() < length()) {
        slot.setValue(this, DontDelete | ReadOnly, getIndex(exec, index.value()));
        return true;
    }

    return false;
}

ALWAYS_INLINE bool JSString::getStringPropertySlot(ExecState* exec, unsigned propertyName, PropertySlot& slot)
{
    if (propertyName < length()) {
        slot.setValue(this, DontDelete | ReadOnly, getIndex(exec, propertyName));
        return true;
    }

    return false;
}

inline bool isJSString(JSCell* cell)
{
    return cell->type() == StringType;
}

inline bool isJSString(JSValue v)
{
    return v.isCell() && isJSString(v.asCell());
}

ALWAYS_INLINE StringView JSRopeString::unsafeView(ExecState& state) const
{
    if (isSubstring()) {
        if (is8Bit())
            return StringView(substringBase()->m_value.characters8() + substringOffset(), length());
        return StringView(substringBase()->m_value.characters16() + substringOffset(), length());
    }
    resolveRope(&state);
    return m_value;
}

ALWAYS_INLINE StringViewWithUnderlyingString JSRopeString::viewWithUnderlyingString(ExecState& state) const
{
    if (isSubstring()) {
        auto& base = substringBase()->m_value;
        if (is8Bit())
            return { { base.characters8() + substringOffset(), length() }, base };
        return { { base.characters16() + substringOffset(), length() }, base };
    }
    resolveRope(&state);
    return { m_value, m_value };
}

ALWAYS_INLINE StringView JSString::unsafeView(ExecState& state) const
{
    if (isRope())
        return static_cast<const JSRopeString*>(this)->unsafeView(state);
    return m_value;
}

ALWAYS_INLINE StringViewWithUnderlyingString JSString::viewWithUnderlyingString(ExecState& state) const
{
    if (isRope())
        return static_cast<const JSRopeString&>(*this).viewWithUnderlyingString(state);
    return { m_value, m_value };
}

inline bool JSString::isSubstring() const
{
    return isRope() && static_cast<const JSRopeString*>(this)->isSubstring();
}

// --- JSValue inlines ----------------------------

inline bool JSValue::toBoolean(ExecState* exec) const
{
    if (isInt32())
        return asInt32();
    if (isDouble())
        return asDouble() > 0.0 || asDouble() < 0.0; // false for NaN
    if (isCell())
        return asCell()->toBoolean(exec);
    return isTrue(); // false, null, and undefined all convert to false.
}

inline JSString* JSValue::toString(ExecState* exec) const
{
    if (isString())
        return asString(asCell());
    bool returnEmptyStringOnError = true;
    return toStringSlowCase(exec, returnEmptyStringOnError);
}

inline JSString* JSValue::toStringOrNull(ExecState* exec) const
{
    if (isString())
        return asString(asCell());
    bool returnEmptyStringOnError = false;
    return toStringSlowCase(exec, returnEmptyStringOnError);
}

inline String JSValue::toWTFString(ExecState* exec) const
{
    if (isString())
        return static_cast<JSString*>(asCell())->value(exec);
    return toWTFStringSlowCase(exec);
}

} // namespace JSC
