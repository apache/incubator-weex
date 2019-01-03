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

#include "JSArrayBufferView.h"
#include "ThrowScope.h"
#include "ToNativeFromValue.h"

namespace JSC {

JS_EXPORT_PRIVATE const ClassInfo* getInt8ArrayClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getInt16ArrayClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getInt32ArrayClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getUint8ArrayClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getUint8ClampedArrayClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getUint16ArrayClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getUint32ArrayClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getFloat32ArrayClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getFloat64ArrayClassInfo();

// A typed array view is our representation of a typed array object as seen
// from JavaScript. For example:
//
// var o = new Int8Array(100);
//
// Here, 'o' points to a JSGenericTypedArrayView<int8_t>.
//
// Views contain five fields:
//
//     Structure* S     // from JSCell
//     Butterfly* B     // from JSObject
//     ElementType* V
//     uint32_t L
//     TypedArrayMode M
//
// These fields take up a total of four pointer-width words. FIXME: Make
// it take less words!
//
// B is usually unused but may stored some additional "overflow" data for
// one of the modes. V always points to the base of the typed array's data,
// and may point to either GC-managed copied space, or data in the C heap;
// which of those things it points to is governed by the mode although for
// simple accesses to the view you can just read from the pointer either
// way. M specifies the mode of the view. L is the length, in units that
// depend on the view's type.

// The JSGenericTypedArrayView is templatized by an Adaptor that controls
// the element type and how it's converted; it should obey the following
// interface; I use int8_t as an example:
//
// struct Adaptor {
//     typedef int8_t Type;
//     typedef Int8Array ViewType;
//     typedef JSInt8Array JSViewType;
//     static int8_t toNativeFromInt32(int32_t);
//     static int8_t toNativeFromUint32(uint32_t);
//     static int8_t toNativeFromDouble(double);
//     static JSValue toJSValue(int8_t);
//     static double toDouble(int8_t);
//     template<T> static T::Type convertTo(uint8_t);
// };

enum class CopyType {
    LeftToRight,
    Unobservable,
};

static const char* const typedArrayBufferHasBeenDetachedErrorMessage = "Underlying ArrayBuffer has been detached from the view";

template<typename Adaptor>
class JSGenericTypedArrayView : public JSArrayBufferView {
public:
    typedef JSArrayBufferView Base;
    typedef typename Adaptor::Type ElementType;

    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetPropertyNames | OverridesGetOwnPropertySlot | InterceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero;

    static const unsigned elementSize = sizeof(typename Adaptor::Type);
    
protected:
    JSGenericTypedArrayView(VM&, ConstructionContext&);
    
public:
    static JSGenericTypedArrayView* create(ExecState*, Structure*, unsigned length);
    static JSGenericTypedArrayView* createWithFastVector(ExecState*, Structure*, unsigned length, void* vector);
    static JSGenericTypedArrayView* createUninitialized(ExecState*, Structure*, unsigned length);
    static JSGenericTypedArrayView* create(ExecState*, Structure*, RefPtr<ArrayBuffer>&&, unsigned byteOffset, unsigned length);
    static JSGenericTypedArrayView* create(VM&, Structure*, RefPtr<typename Adaptor::ViewType>&& impl);
    static JSGenericTypedArrayView* create(Structure*, JSGlobalObject*, RefPtr<typename Adaptor::ViewType>&& impl);
    
    unsigned byteLength() const { return m_length * sizeof(typename Adaptor::Type); }
    size_t byteSize() const { return sizeOf(m_length, sizeof(typename Adaptor::Type)); }
    
    const typename Adaptor::Type* typedVector() const
    {
        return bitwise_cast<const typename Adaptor::Type*>(vector());
    }
    typename Adaptor::Type* typedVector()
    {
        return bitwise_cast<typename Adaptor::Type*>(vector());
    }

    // These methods are meant to match indexed access methods that JSObject
    // supports - hence the slight redundancy.
    bool canGetIndexQuickly(unsigned i)
    {
        return i < m_length;
    }
    bool canSetIndexQuickly(unsigned i)
    {
        return i < m_length;
    }
    
    typename Adaptor::Type getIndexQuicklyAsNativeValue(unsigned i)
    {
        ASSERT(i < m_length);
        return typedVector()[i];
    }
    
    double getIndexQuicklyAsDouble(unsigned i)
    {
        return Adaptor::toDouble(getIndexQuicklyAsNativeValue(i));
    }
    
    JSValue getIndexQuickly(unsigned i)
    {
        return Adaptor::toJSValue(getIndexQuicklyAsNativeValue(i));
    }
    
    void setIndexQuicklyToNativeValue(unsigned i, typename Adaptor::Type value)
    {
        ASSERT(i < m_length);
        typedVector()[i] = value;
    }
    
    void setIndexQuicklyToDouble(unsigned i, double value)
    {
        setIndexQuicklyToNativeValue(i, toNativeFromValue<Adaptor>(jsNumber(value)));
    }
    
    void setIndexQuickly(unsigned i, JSValue value)
    {
        ASSERT(!value.isObject());
        setIndexQuicklyToNativeValue(i, toNativeFromValue<Adaptor>(value));
    }
    
    bool setIndex(ExecState* exec, unsigned i, JSValue jsValue)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);

        typename Adaptor::Type value = toNativeFromValue<Adaptor>(exec, jsValue);
        RETURN_IF_EXCEPTION(scope, false);

        if (isNeutered()) {
            throwTypeError(exec, scope, ASCIILiteral(typedArrayBufferHasBeenDetachedErrorMessage));
            return false;
        }

        if (i >= m_length)
            return false;

        setIndexQuicklyToNativeValue(i, value);
        return true;
    }

    static ElementType toAdaptorNativeFromValue(ExecState* exec, JSValue jsValue) { return toNativeFromValue<Adaptor>(exec, jsValue); }

    static std::optional<ElementType> toAdaptorNativeFromValueWithoutCoercion(JSValue jsValue) { return toNativeFromValueWithoutCoercion<Adaptor>(jsValue); }

    void sort()
    {
        RELEASE_ASSERT(!isNeutered());
        switch (Adaptor::typeValue) {
        case TypeFloat32:
            sortFloat<int32_t>();
            break;
        case TypeFloat64:
            sortFloat<int64_t>();
            break;
        default: {
            ElementType* array = typedVector();
            std::sort(array, array + m_length);
            break;
        }
        }
    }

    bool canAccessRangeQuickly(unsigned offset, unsigned length)
    {
        return offset <= m_length
            && offset + length <= m_length
            // check overflow
            && offset + length >= offset;
    }
    
    // Like canSetQuickly, except: if it returns false, it will throw the
    // appropriate exception.
    bool validateRange(ExecState*, unsigned offset, unsigned length);

    // Returns true if successful, and false on error; if it returns false
    // then it will have thrown an exception.
    bool set(ExecState*, unsigned offset, JSObject*, unsigned objectOffset, unsigned length, CopyType type = CopyType::Unobservable);
    
    RefPtr<typename Adaptor::ViewType> possiblySharedTypedImpl();
    RefPtr<typename Adaptor::ViewType> unsharedTypedImpl();

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(typeForTypedArrayType(Adaptor::typeValue), StructureFlags), info(), NonArray);
    }
    
    static const ClassInfo s_info; // This is never accessed directly, since that would break linkage on some compilers.
    
    static const ClassInfo* info()
    {
        switch (Adaptor::typeValue) {
        case TypeInt8:
            return getInt8ArrayClassInfo();
        case TypeInt16:
            return getInt16ArrayClassInfo();
        case TypeInt32:
            return getInt32ArrayClassInfo();
        case TypeUint8:
            return getUint8ArrayClassInfo();
        case TypeUint8Clamped:
            return getUint8ClampedArrayClassInfo();
        case TypeUint16:
            return getUint16ArrayClassInfo();
        case TypeUint32:
            return getUint32ArrayClassInfo();
        case TypeFloat32:
            return getFloat32ArrayClassInfo();
        case TypeFloat64:
            return getFloat64ArrayClassInfo();
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return 0;
        }
    }
    
    ArrayBuffer* existingBuffer();

    static const TypedArrayType TypedArrayStorageType = Adaptor::typeValue;

    // This is the default DOM unwrapping. It calls toUnsharedNativeTypedView().
    static RefPtr<typename Adaptor::ViewType> toWrapped(VM&, JSValue);
    
protected:
    friend struct TypedArrayClassInfos;

    static EncodedJSValue throwNeuteredTypedArrayTypeError(ExecState*, EncodedJSValue, PropertyName);

    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);
    static bool deleteProperty(JSCell*, ExecState*, PropertyName);

    static bool getOwnPropertySlotByIndex(JSObject*, ExecState*, unsigned propertyName, PropertySlot&);
    static bool putByIndex(JSCell*, ExecState*, unsigned propertyName, JSValue, bool shouldThrow);
    static bool deletePropertyByIndex(JSCell*, ExecState*, unsigned propertyName);
    
    static void getOwnPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);

    static size_t estimatedSize(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

    // Allocates the full-on native buffer and moves data into the C heap if
    // necessary. Note that this never allocates in the GC heap.
    static ArrayBuffer* slowDownAndWasteMemory(JSArrayBufferView*);
    static RefPtr<ArrayBufferView> getTypedArrayImpl(JSArrayBufferView*);

private:
    // Returns true if successful, and false on error; it will throw on error.
    template<typename OtherAdaptor>
    bool setWithSpecificType(
        ExecState*, unsigned offset, JSGenericTypedArrayView<OtherAdaptor>*,
        unsigned objectOffset, unsigned length, CopyType);

    // The ECMA 6 spec states that floating point Typed Arrays should have the following ordering:
    //
    // -Inifinity < negative finite numbers < -0.0 < 0.0 < positive finite numbers < Infinity < NaN
    // Note: regardless of the sign or exact representation of a NaN it is greater than all other values.
    //
    // An interesting fact about IEEE 754 floating point numbers is that have an adjacent representation
    // i.e. for any finite floating point x there does not exist a finite floating point y such that
    // ((float) ((int) x + 1)) > y > x (where int represents a signed bit integer with the same number
    // of bits as float). Thus, if we have an array of floating points if we view it as an
    // array of signed bit integers it will sort in the format we desire. Note, denormal
    // numbers fit this property as they are floating point numbers with a exponent field of all
    // zeros so they will be closer to the signed zeros than any normalized number.
    //
    // All the processors we support, however, use twos complement. Fortunately, if you compare a signed
    // bit number as if it were twos complement the result will be correct assuming both numbers are not
    // negative. e.g.
    //
    //    - <=> - = reversed (-30 > -20 = true)
    //    + <=> + = ordered (30 > 20 = true)
    //    - <=> + = ordered (-30 > 20 = false)
    //    + <=> - = ordered (30 > -20 = true)
    //
    // For NaN, we normalize the NaN to a peticular representation; the sign bit is 0, all exponential bits
    // are 1 and only the MSB of the mantissa is 1. So, NaN is recognized as the largest integral numbers.

    void purifyArray()
    {
        ElementType* array = typedVector();
        for (unsigned i = 0; i < m_length; i++)
            array[i] = purifyNaN(array[i]);
    }

    template<typename IntegralType>
    static bool ALWAYS_INLINE sortComparison(IntegralType a, IntegralType b)
    {
        if (a >= 0 || b >= 0)
            return a < b;
        return a > b;
    }

    template<typename IntegralType>
    void sortFloat()
    {
        ASSERT(sizeof(IntegralType) == sizeof(ElementType));

        // Since there might be another view that sets the bits of
        // our floats to NaNs with negative sign bits we need to
        // purify the array.
        // We use a separate function here to avoid the strict aliasing rule.
        // We could use a union but ASAN seems to frown upon that.
        purifyArray();

        IntegralType* array = reinterpret_cast_ptr<IntegralType*>(typedVector());
        std::sort(array, array + m_length, sortComparison<IntegralType>);

    }

};

template<typename Adaptor>
inline RefPtr<typename Adaptor::ViewType> toPossiblySharedNativeTypedView(VM& vm, JSValue value)
{
    typename Adaptor::JSViewType* wrapper = jsDynamicCast<typename Adaptor::JSViewType*>(vm, value);
    if (!wrapper)
        return nullptr;
    return wrapper->possiblySharedTypedImpl();
}

template<typename Adaptor>
inline RefPtr<typename Adaptor::ViewType> toUnsharedNativeTypedView(VM& vm, JSValue value)
{
    RefPtr<typename Adaptor::ViewType> result = toPossiblySharedNativeTypedView<Adaptor>(vm, value);
    if (!result || result->isShared())
        return nullptr;
    return result;
}

template<typename Adaptor>
RefPtr<typename Adaptor::ViewType> JSGenericTypedArrayView<Adaptor>::toWrapped(VM& vm, JSValue value)
{
    return JSC::toUnsharedNativeTypedView<Adaptor>(vm, value);
}

} // namespace JSC
