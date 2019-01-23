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

#include "JSExportMacros.h"
#include "PureNaN.h"
#include <functional>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <wtf/Assertions.h>
#include <wtf/Forward.h>
#include <wtf/HashMap.h>
#include <wtf/HashTraits.h>
#include <wtf/MathExtras.h>
#include <wtf/MediaTime.h>
#include <wtf/StdLibExtras.h>
#include <wtf/TriState.h>

namespace JSC {

class AssemblyHelpers;
class ExecState;
class JSCell;
class JSValueSource;
class VM;
class JSGlobalObject;
class JSObject;
class JSString;
class Identifier;
class PropertyName;
class PropertySlot;
class PutPropertySlot;
class Structure;
#if ENABLE(DFG_JIT)
namespace DFG {
class JITCompiler;
class OSRExitCompiler;
class SpeculativeJIT;
}
#endif
#if !ENABLE(JIT)
namespace LLInt {
class CLoop;
}
#endif

struct ClassInfo;
struct DumpContext;
struct Instruction;
struct MethodTable;

template <class T> class WriteBarrierBase;

enum PreferredPrimitiveType { NoPreference, PreferNumber, PreferString };
enum ECMAMode { StrictMode, NotStrictMode };

enum class CallType : unsigned;
struct CallData;
enum class ConstructType : unsigned;
struct ConstructData;

typedef int64_t EncodedJSValue;
    
union EncodedValueDescriptor {
    int64_t asInt64;
#if USE(JSVALUE32_64)
    double asDouble;
#elif USE(JSVALUE64)
    JSCell* ptr;
#endif
        
#if CPU(BIG_ENDIAN)
    struct {
        int32_t tag;
        int32_t payload;
    } asBits;
#else
    struct {
        int32_t payload;
        int32_t tag;
    } asBits;
#endif
};

#define TagOffset (OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag))
#define PayloadOffset (OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload))

#if USE(JSVALUE64)
#define CellPayloadOffset 0
#else
#define CellPayloadOffset PayloadOffset
#endif

enum WhichValueWord {
    TagWord,
    PayloadWord
};

int64_t tryConvertToInt52(double);
bool isInt52(double);

enum class SourceCodeRepresentation {
    Other,
    Integer,
    Double
};

class JSValue {
    friend struct EncodedJSValueHashTraits;
    friend struct EncodedJSValueWithRepresentationHashTraits;
    friend class AssemblyHelpers;
    friend class JIT;
    friend class JITSlowPathCall;
    friend class JITStubs;
    friend class JITStubCall;
    friend class JSInterfaceJIT;
    friend class JSValueSource;
    friend class SpecializedThunkJIT;
#if ENABLE(DFG_JIT)
    friend class DFG::JITCompiler;
    friend class DFG::OSRExitCompiler;
    friend class DFG::SpeculativeJIT;
#endif
#if !ENABLE(JIT)
    friend class LLInt::CLoop;
#endif

public:
#if USE(JSVALUE32_64)
    enum { Int32Tag =        0xffffffff };
    enum { BooleanTag =      0xfffffffe };
    enum { NullTag =         0xfffffffd };
    enum { UndefinedTag =    0xfffffffc };
    enum { CellTag =         0xfffffffb };
    enum { EmptyValueTag =   0xfffffffa };
    enum { DeletedValueTag = 0xfffffff9 };

    enum { LowestTag =  DeletedValueTag };
#endif

    static EncodedJSValue encode(JSValue);
    static JSValue decode(EncodedJSValue);

    enum JSNullTag { JSNull };
    enum JSUndefinedTag { JSUndefined };
    enum JSTrueTag { JSTrue };
    enum JSFalseTag { JSFalse };
    enum EncodeAsDoubleTag { EncodeAsDouble };

    JSValue();
    JSValue(JSNullTag);
    JSValue(JSUndefinedTag);
    JSValue(JSTrueTag);
    JSValue(JSFalseTag);
    JSValue(JSCell* ptr);
    JSValue(const JSCell* ptr);

    // Numbers
    JSValue(EncodeAsDoubleTag, double);
    explicit JSValue(double);
    explicit JSValue(char);
    explicit JSValue(unsigned char);
    explicit JSValue(short);
    explicit JSValue(unsigned short);
    explicit JSValue(int);
    explicit JSValue(unsigned);
    explicit JSValue(long);
    explicit JSValue(unsigned long);
    explicit JSValue(long long);
    explicit JSValue(unsigned long long);

    explicit operator bool() const;
    bool operator==(const JSValue& other) const;
    bool operator!=(const JSValue& other) const;

    bool isInt32() const;
    bool isUInt32() const;
    bool isDouble() const;
    bool isTrue() const;
    bool isFalse() const;

    int32_t asInt32() const;
    uint32_t asUInt32() const;
    int64_t asAnyInt() const;
    double asDouble() const;
    bool asBoolean() const;
    double asNumber() const;
    
    int32_t asInt32ForArithmetic() const; // Boolean becomes an int, but otherwise like asInt32().

    // Querying the type.
    bool isEmpty() const;
    bool isFunction() const;
    bool isFunction(CallType&, CallData&) const;
    bool isCallable(CallType&, CallData&) const;
    bool isConstructor() const;
    bool isConstructor(ConstructType&, ConstructData&) const;
    bool isUndefined() const;
    bool isNull() const;
    bool isUndefinedOrNull() const;
    bool isBoolean() const;
    bool isAnyInt() const;
    bool isNumber() const;
    bool isString() const;
    bool isSymbol() const;
    bool isPrimitive() const;
    bool isGetterSetter() const;
    bool isCustomGetterSetter() const;
    bool isObject() const;
    bool inherits(VM&, const ClassInfo*) const;
    const ClassInfo* classInfoOrNull(VM&) const;
        
    // Extracting the value.
    bool getString(ExecState*, WTF::String&) const;
    WTF::String getString(ExecState*) const; // null string if not a string
    JSObject* getObject() const; // 0 if not an object

    // Extracting integer values.
    bool getUInt32(uint32_t&) const;
        
    // Basic conversions.
    JSValue toPrimitive(ExecState*, PreferredPrimitiveType = NoPreference) const;
    bool getPrimitiveNumber(ExecState*, double& number, JSValue&);

    bool toBoolean(ExecState*) const;
    TriState pureToBoolean() const;

    // toNumber conversion is expected to be side effect free if an exception has
    // been set in the ExecState already.
    double toNumber(ExecState*) const;

    // toNumber conversion if it can be done without side effects.
    std::optional<double> toNumberFromPrimitive() const;

    JSString* toString(ExecState*) const; // On exception, this returns the empty string.
    JSString* toStringOrNull(ExecState*) const; // On exception, this returns null, to make exception checks faster.
    Identifier toPropertyKey(ExecState*) const;
    WTF::String toWTFString(ExecState*) const;
    JSObject* toObject(ExecState*) const;
    JSObject* toObject(ExecState*, JSGlobalObject*) const;

    // Integer conversions.
    JS_EXPORT_PRIVATE double toInteger(ExecState*) const;
    JS_EXPORT_PRIVATE double toIntegerPreserveNaN(ExecState*) const;
    int32_t toInt32(ExecState*) const;
    uint32_t toUInt32(ExecState*) const;
    uint32_t toIndex(ExecState*, const char* errorName) const;
    double toLength(ExecState*) const;

    // Floating point conversions (this is a convenience function for WebCore;
    // single precision float is not a representation used in JS or JSC).
    float toFloat(ExecState* exec) const { return static_cast<float>(toNumber(exec)); }

    // Object operations, with the toObject operation included.
    JSValue get(ExecState*, PropertyName) const;
    JSValue get(ExecState*, PropertyName, PropertySlot&) const;
    JSValue get(ExecState*, unsigned propertyName) const;
    JSValue get(ExecState*, unsigned propertyName, PropertySlot&) const;
    JSValue get(ExecState*, uint64_t propertyName) const;

    bool getPropertySlot(ExecState*, PropertyName, PropertySlot&) const;
    template<typename CallbackWhenNoException> typename std::result_of<CallbackWhenNoException(bool, PropertySlot&)>::type getPropertySlot(ExecState*, PropertyName, CallbackWhenNoException) const;
    template<typename CallbackWhenNoException> typename std::result_of<CallbackWhenNoException(bool, PropertySlot&)>::type getPropertySlot(ExecState*, PropertyName, PropertySlot&, CallbackWhenNoException) const;

    bool put(ExecState*, PropertyName, JSValue, PutPropertySlot&);
    bool putInline(ExecState*, PropertyName, JSValue, PutPropertySlot&);
    JS_EXPORT_PRIVATE bool putToPrimitive(ExecState*, PropertyName, JSValue, PutPropertySlot&);
    JS_EXPORT_PRIVATE bool putToPrimitiveByIndex(ExecState*, unsigned propertyName, JSValue, bool shouldThrow);
    bool putByIndex(ExecState*, unsigned propertyName, JSValue, bool shouldThrow);

    JSValue toThis(ExecState*, ECMAMode) const;

    static bool equal(ExecState*, JSValue v1, JSValue v2);
    static bool equalSlowCase(ExecState*, JSValue v1, JSValue v2);
    static bool equalSlowCaseInline(ExecState*, JSValue v1, JSValue v2);
    static bool strictEqual(ExecState*, JSValue v1, JSValue v2);
    static bool strictEqualSlowCase(ExecState*, JSValue v1, JSValue v2);
    static bool strictEqualSlowCaseInline(ExecState*, JSValue v1, JSValue v2);
    static TriState pureStrictEqual(JSValue v1, JSValue v2);

    bool isCell() const;
    JSCell* asCell() const;
    JS_EXPORT_PRIVATE bool isValidCallee();

    Structure* structureOrNull() const;
    JSValue structureOrUndefined() const;

    JS_EXPORT_PRIVATE void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;
    void dumpInContextAssumingStructure(PrintStream&, DumpContext*, Structure*) const;
    void dumpForBacktrace(PrintStream&) const;

    JS_EXPORT_PRIVATE JSObject* synthesizePrototype(ExecState*) const;
    bool requireObjectCoercible(ExecState*) const;

    // Constants used for Int52. Int52 isn't part of JSValue right now, but JSValues may be
    // converted to Int52s and back again.
    static constexpr const unsigned numberOfInt52Bits = 52;
    static constexpr const int64_t notInt52 = static_cast<int64_t>(1) << numberOfInt52Bits;
    static constexpr const unsigned int52ShiftAmount = 12;
    
    static ptrdiff_t offsetOfPayload() { return OBJECT_OFFSETOF(JSValue, u.asBits.payload); }
    static ptrdiff_t offsetOfTag() { return OBJECT_OFFSETOF(JSValue, u.asBits.tag); }

#if USE(JSVALUE32_64)
    /*
     * On 32-bit platforms USE(JSVALUE32_64) should be defined, and we use a NaN-encoded
     * form for immediates.
     *
     * The encoding makes use of unused NaN space in the IEEE754 representation.  Any value
     * with the top 13 bits set represents a QNaN (with the sign bit set).  QNaN values
     * can encode a 51-bit payload.  Hardware produced and C-library payloads typically
     * have a payload of zero.  We assume that non-zero payloads are available to encode
     * pointer and integer values.  Since any 64-bit bit pattern where the top 15 bits are
     * all set represents a NaN with a non-zero payload, we can use this space in the NaN
     * ranges to encode other values (however there are also other ranges of NaN space that
     * could have been selected).
     *
     * For JSValues that do not contain a double value, the high 32 bits contain the tag
     * values listed in the enums below, which all correspond to NaN-space. In the case of
     * cell, integer and bool values the lower 32 bits (the 'payload') contain the pointer
     * integer or boolean value; in the case of all other tags the payload is 0.
     */
    uint32_t tag() const;
    int32_t payload() const;

#if !ENABLE(JIT)
    // This should only be used by the LLInt C Loop interpreter who needs
    // synthesize JSValue from its "register"s holding tag and payload
    // values.
    explicit JSValue(int32_t tag, int32_t payload);
#endif

#elif USE(JSVALUE64)
    /*
     * On 64-bit platforms USE(JSVALUE64) should be defined, and we use a NaN-encoded
     * form for immediates.
     *
     * The encoding makes use of unused NaN space in the IEEE754 representation.  Any value
     * with the top 13 bits set represents a QNaN (with the sign bit set).  QNaN values
     * can encode a 51-bit payload.  Hardware produced and C-library payloads typically
     * have a payload of zero.  We assume that non-zero payloads are available to encode
     * pointer and integer values.  Since any 64-bit bit pattern where the top 15 bits are
     * all set represents a NaN with a non-zero payload, we can use this space in the NaN
     * ranges to encode other values (however there are also other ranges of NaN space that
     * could have been selected).
     *
     * This range of NaN space is represented by 64-bit numbers begining with the 16-bit
     * hex patterns 0xFFFE and 0xFFFF - we rely on the fact that no valid double-precision
     * numbers will fall in these ranges.
     *
     * The top 16-bits denote the type of the encoded JSValue:
     *
     *     Pointer {  0000:PPPP:PPPP:PPPP
     *              / 0001:****:****:****
     *     Double  {         ...
     *              \ FFFE:****:****:****
     *     Integer {  FFFF:0000:IIII:IIII
     *
     * The scheme we have implemented encodes double precision values by performing a
     * 64-bit integer addition of the value 2^48 to the number. After this manipulation
     * no encoded double-precision value will begin with the pattern 0x0000 or 0xFFFF.
     * Values must be decoded by reversing this operation before subsequent floating point
     * operations may be peformed.
     *
     * 32-bit signed integers are marked with the 16-bit tag 0xFFFF.
     *
     * The tag 0x0000 denotes a pointer, or another form of tagged immediate. Boolean,
     * null and undefined values are represented by specific, invalid pointer values:
     *
     *     False:     0x06
     *     True:      0x07
     *     Undefined: 0x0a
     *     Null:      0x02
     *
     * These values have the following properties:
     * - Bit 1 (TagBitTypeOther) is set for all four values, allowing real pointers to be
     *   quickly distinguished from all immediate values, including these invalid pointers.
     * - With bit 3 is masked out (TagBitUndefined) Undefined and Null share the
     *   same value, allowing null & undefined to be quickly detected.
     *
     * No valid JSValue will have the bit pattern 0x0, this is used to represent array
     * holes, and as a C++ 'no value' result (e.g. JSValue() has an internal value of 0).
     */

    // These values are #defines since using static const integers here is a ~1% regression!

    // This value is 2^48, used to encode doubles such that the encoded value will begin
    // with a 16-bit pattern within the range 0x0001..0xFFFE.
    #define DoubleEncodeOffset 0x1000000000000ll
    // If all bits in the mask are set, this indicates an integer number,
    // if any but not all are set this value is a double precision number.
    #define TagTypeNumber 0xffff000000000000ll

    // All non-numeric (bool, null, undefined) immediates have bit 2 set.
    #define TagBitTypeOther 0x2ll
    #define TagBitBool      0x4ll
    #define TagBitUndefined 0x8ll
    // Combined integer value for non-numeric immediates.
    #define ValueFalse     (TagBitTypeOther | TagBitBool | false)
    #define ValueTrue      (TagBitTypeOther | TagBitBool | true)
    #define ValueUndefined (TagBitTypeOther | TagBitUndefined)
    #define ValueNull      (TagBitTypeOther)

    // TagMask is used to check for all types of immediate values (either number or 'other').
    #define TagMask (TagTypeNumber | TagBitTypeOther)

    // These special values are never visible to JavaScript code; Empty is used to represent
    // Array holes, and for uninitialized JSValues. Deleted is used in hash table code.
    // These values would map to cell types in the JSValue encoding, but not valid GC cell
    // pointer should have either of these values (Empty is null, deleted is at an invalid
    // alignment for a GC cell, and in the zero page).
    #define ValueEmpty   0x0ll
    #define ValueDeleted 0x4ll
#endif

private:
    template <class T> JSValue(WriteBarrierBase<T>);

    enum HashTableDeletedValueTag { HashTableDeletedValue };
    JSValue(HashTableDeletedValueTag);

    inline const JSValue asValue() const { return *this; }
    JS_EXPORT_PRIVATE double toNumberSlowCase(ExecState*) const;
    JS_EXPORT_PRIVATE JSString* toStringSlowCase(ExecState*, bool returnEmptyStringOnError) const;
    JS_EXPORT_PRIVATE WTF::String toWTFStringSlowCase(ExecState*) const;
    JS_EXPORT_PRIVATE JSObject* toObjectSlowCase(ExecState*, JSGlobalObject*) const;
    JS_EXPORT_PRIVATE JSValue toThisSlowCase(ExecState*, ECMAMode) const;

    EncodedValueDescriptor u;
};

typedef IntHash<EncodedJSValue> EncodedJSValueHash;

#if USE(JSVALUE32_64)
struct EncodedJSValueHashTraits : HashTraits<EncodedJSValue> {
    static const bool emptyValueIsZero = false;
    static EncodedJSValue emptyValue() { return JSValue::encode(JSValue()); }
    static void constructDeletedValue(EncodedJSValue& slot) { slot = JSValue::encode(JSValue(JSValue::HashTableDeletedValue)); }
    static bool isDeletedValue(EncodedJSValue value) { return value == JSValue::encode(JSValue(JSValue::HashTableDeletedValue)); }
};
#else
struct EncodedJSValueHashTraits : HashTraits<EncodedJSValue> {
    static void constructDeletedValue(EncodedJSValue& slot) { slot = JSValue::encode(JSValue(JSValue::HashTableDeletedValue)); }
    static bool isDeletedValue(EncodedJSValue value) { return value == JSValue::encode(JSValue(JSValue::HashTableDeletedValue)); }
};
#endif

typedef std::pair<EncodedJSValue, SourceCodeRepresentation> EncodedJSValueWithRepresentation;

struct EncodedJSValueWithRepresentationHashTraits : HashTraits<EncodedJSValueWithRepresentation> {
    static const bool emptyValueIsZero = false;
    static EncodedJSValueWithRepresentation emptyValue() { return std::make_pair(JSValue::encode(JSValue()), SourceCodeRepresentation::Other); }
    static void constructDeletedValue(EncodedJSValueWithRepresentation& slot) { slot = std::make_pair(JSValue::encode(JSValue(JSValue::HashTableDeletedValue)), SourceCodeRepresentation::Other); }
    static bool isDeletedValue(EncodedJSValueWithRepresentation value) { return value == std::make_pair(JSValue::encode(JSValue(JSValue::HashTableDeletedValue)), SourceCodeRepresentation::Other); }
};

struct EncodedJSValueWithRepresentationHash {
    static unsigned hash(const EncodedJSValueWithRepresentation& value)
    {
        return WTF::pairIntHash(EncodedJSValueHash::hash(value.first), IntHash<SourceCodeRepresentation>::hash(value.second));
    }
    static bool equal(const EncodedJSValueWithRepresentation& a, const EncodedJSValueWithRepresentation& b)
    {
        return a == b;
    }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

// Stand-alone helper functions.
inline JSValue jsNull()
{
    return JSValue(JSValue::JSNull);
}

inline JSValue jsUndefined()
{
    return JSValue(JSValue::JSUndefined);
}

inline JSValue jsTDZValue()
{
    return JSValue();
}

inline JSValue jsBoolean(bool b)
{
    return b ? JSValue(JSValue::JSTrue) : JSValue(JSValue::JSFalse);
}

ALWAYS_INLINE JSValue jsDoubleNumber(double d)
{
    ASSERT(JSValue(JSValue::EncodeAsDouble, d).isNumber());
    return JSValue(JSValue::EncodeAsDouble, d);
}

ALWAYS_INLINE JSValue jsNumber(double d)
{
    ASSERT(JSValue(d).isNumber());
    return JSValue(d);
}

ALWAYS_INLINE JSValue jsNumber(MediaTime t)
{
    return jsNumber(t.toDouble());
}

ALWAYS_INLINE JSValue jsNumber(char i)
{
    return JSValue(i);
}

ALWAYS_INLINE JSValue jsNumber(unsigned char i)
{
    return JSValue(i);
}

ALWAYS_INLINE JSValue jsNumber(short i)
{
    return JSValue(i);
}

ALWAYS_INLINE JSValue jsNumber(unsigned short i)
{
    return JSValue(i);
}

ALWAYS_INLINE JSValue jsNumber(int i)
{
    return JSValue(i);
}

ALWAYS_INLINE JSValue jsNumber(unsigned i)
{
    return JSValue(i);
}

ALWAYS_INLINE JSValue jsNumber(long i)
{
    return JSValue(i);
}

ALWAYS_INLINE JSValue jsNumber(unsigned long i)
{
    return JSValue(i);
}

ALWAYS_INLINE JSValue jsNumber(long long i)
{
    return JSValue(i);
}

ALWAYS_INLINE JSValue jsNumber(unsigned long long i)
{
    return JSValue(i);
}

ALWAYS_INLINE EncodedJSValue encodedJSUndefined()
{
    return JSValue::encode(jsUndefined());
}

ALWAYS_INLINE EncodedJSValue encodedJSValue()
{
    return JSValue::encode(JSValue());
}

inline bool operator==(const JSValue a, const JSCell* b) { return a == JSValue(b); }
inline bool operator==(const JSCell* a, const JSValue b) { return JSValue(a) == b; }

inline bool operator!=(const JSValue a, const JSCell* b) { return a != JSValue(b); }
inline bool operator!=(const JSCell* a, const JSValue b) { return JSValue(a) != b; }


bool isThisValueAltered(const PutPropertySlot&, JSObject* baseObject);

// See section 7.2.9: https://tc39.github.io/ecma262/#sec-samevalue
bool sameValue(ExecState*, JSValue a, JSValue b);

} // namespace JSC
