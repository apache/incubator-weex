/*
 * Copyright (C) 2008, 2016 Apple Inc. All Rights Reserved.
 * Copyright (C) 2013 Patrick Gansterer <paroga@paroga.com>
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

#ifndef WTF_StdLibExtras_h
#define WTF_StdLibExtras_h

#include <chrono>
#include <cstring>
#include <memory>
#include <wtf/Assertions.h>
#include <wtf/CheckedArithmetic.h>
#include <wtf/Compiler.h>

// This was used to declare and define a static local variable (static T;) so that
//  it was leaked so that its destructors were not called at exit.
// Newly written code should use static NeverDestroyed<T> instead.
#ifndef DEPRECATED_DEFINE_STATIC_LOCAL
#define DEPRECATED_DEFINE_STATIC_LOCAL(type, name, arguments) \
    static type& name = *new type arguments
#endif

// Use this macro to declare and define a debug-only global variable that may have a
// non-trivial constructor and destructor. When building with clang, this will suppress
// warnings about global constructors and exit-time destructors.
#define DEFINE_GLOBAL_FOR_LOGGING(type, name, arguments) \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"") \
    _Pragma("clang diagnostic ignored \"-Wexit-time-destructors\"") \
    static type name arguments; \
    _Pragma("clang diagnostic pop")

#ifndef NDEBUG
#if COMPILER(CLANG)
#define DEFINE_DEBUG_ONLY_GLOBAL(type, name, arguments) DEFINE_GLOBAL_FOR_LOGGING(type, name, arguments)
#else
#define DEFINE_DEBUG_ONLY_GLOBAL(type, name, arguments) \
    static type name arguments;
#endif // COMPILER(CLANG)
#else
#define DEFINE_DEBUG_ONLY_GLOBAL(type, name, arguments)
#endif // NDEBUG

// OBJECT_OFFSETOF: Like the C++ offsetof macro, but you can use it with classes.
// The magic number 0x4000 is insignificant. We use it to avoid using NULL, since
// NULL can cause compiler problems, especially in cases of multiple inheritance.
#define OBJECT_OFFSETOF(class, field) (reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<class*>(0x4000)->field)) - 0x4000)

#define CAST_OFFSET(from, to) (reinterpret_cast<uintptr_t>(static_cast<to>((reinterpret_cast<from>(0x4000)))) - 0x4000)

// STRINGIZE: Can convert any value to quoted string, even expandable macros
#define STRINGIZE(exp) #exp
#define STRINGIZE_VALUE_OF(exp) STRINGIZE(exp)

// WTF_CONCAT: concatenate two symbols into one, even expandable macros
#define WTF_CONCAT_INTERNAL_DONT_USE(a, b) a ## b
#define WTF_CONCAT(a, b) WTF_CONCAT_INTERNAL_DONT_USE(a, b)


/*
 * The reinterpret_cast<Type1*>([pointer to Type2]) expressions - where
 * sizeof(Type1) > sizeof(Type2) - cause the following warning on ARM with GCC:
 * increases required alignment of target type.
 *
 * An implicit or an extra static_cast<void*> bypasses the warning.
 * For more info see the following bugzilla entries:
 * - https://bugs.webkit.org/show_bug.cgi?id=38045
 * - http://gcc.gnu.org/bugzilla/show_bug.cgi?id=43976
 */
#if (CPU(ARM) || CPU(MIPS)) && COMPILER(GCC_OR_CLANG)
template<typename Type>
inline bool isPointerTypeAlignmentOkay(Type* ptr)
{
    return !(reinterpret_cast<intptr_t>(ptr) % __alignof__(Type));
}

template<typename TypePtr>
inline TypePtr reinterpret_cast_ptr(void* ptr)
{
    ASSERT(isPointerTypeAlignmentOkay(reinterpret_cast<TypePtr>(ptr)));
    return reinterpret_cast<TypePtr>(ptr);
}

template<typename TypePtr>
inline TypePtr reinterpret_cast_ptr(const void* ptr)
{
    ASSERT(isPointerTypeAlignmentOkay(reinterpret_cast<TypePtr>(ptr)));
    return reinterpret_cast<TypePtr>(ptr);
}
#else
template<typename Type>
inline bool isPointerTypeAlignmentOkay(Type*)
{
    return true;
}
#define reinterpret_cast_ptr reinterpret_cast
#endif

namespace WTF {

enum CheckMoveParameterTag { CheckMoveParameter };

static const size_t KB = 1024;
static const size_t MB = 1024 * 1024;
static const size_t GB = 1024 * 1024 * 1024;

inline bool isPointerAligned(void* p)
{
    return !((intptr_t)(p) & (sizeof(char*) - 1));
}

inline bool is8ByteAligned(void* p)
{
    return !((uintptr_t)(p) & (sizeof(double) - 1));
}

/*
 * C++'s idea of a reinterpret_cast lacks sufficient cojones.
 */
template<typename ToType, typename FromType>
inline ToType bitwise_cast(FromType from)
{
    static_assert(sizeof(FromType) == sizeof(ToType), "bitwise_cast size of FromType and ToType must be equal!");
#if COMPILER_SUPPORTS(BUILTIN_IS_TRIVIALLY_COPYABLE)
    // Not all recent STL implementations support the std::is_trivially_copyable type trait. Work around this by only checking on toolchains which have the equivalent compiler intrinsic.
    static_assert(__is_trivially_copyable(ToType), "bitwise_cast of non-trivially-copyable type!");
    static_assert(__is_trivially_copyable(FromType), "bitwise_cast of non-trivially-copyable type!");
#endif
    typename std::remove_const<ToType>::type to { };
    std::memcpy(&to, &from, sizeof(to));
    return to;
}

template<typename ToType, typename FromType>
inline ToType safeCast(FromType value)
{
    RELEASE_ASSERT(isInBounds<ToType>(value));
    return static_cast<ToType>(value);
}

// Returns a count of the number of bits set in 'bits'.
inline size_t bitCount(unsigned bits)
{
    bits = bits - ((bits >> 1) & 0x55555555);
    bits = (bits & 0x33333333) + ((bits >> 2) & 0x33333333);
    return (((bits + (bits >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}

inline size_t bitCount(uint64_t bits)
{
    return bitCount(static_cast<unsigned>(bits)) + bitCount(static_cast<unsigned>(bits >> 32));
}

// Macro that returns a compile time constant with the length of an array, but gives an error if passed a non-array.
template<typename T, size_t Size> char (&ArrayLengthHelperFunction(T (&)[Size]))[Size];
// GCC needs some help to deduce a 0 length array.
#if COMPILER(GCC_OR_CLANG)
template<typename T> char (&ArrayLengthHelperFunction(T (&)[0]))[0];
#endif
#define WTF_ARRAY_LENGTH(array) sizeof(::WTF::ArrayLengthHelperFunction(array))

ALWAYS_INLINE constexpr size_t roundUpToMultipleOfImpl0(size_t remainderMask, size_t x)
{
    return (x + remainderMask) & ~remainderMask;
}

ALWAYS_INLINE constexpr size_t roundUpToMultipleOfImpl(size_t divisor, size_t x)
{
    return roundUpToMultipleOfImpl0(divisor - 1, x);
}

// Efficient implementation that takes advantage of powers of two.
inline size_t roundUpToMultipleOf(size_t divisor, size_t x)
{
    ASSERT(divisor && !(divisor & (divisor - 1)));
    return roundUpToMultipleOfImpl(divisor, x);
}

template<size_t divisor> inline constexpr size_t roundUpToMultipleOf(size_t x)
{
    static_assert(divisor && !(divisor & (divisor - 1)), "divisor must be a power of two!");
    return roundUpToMultipleOfImpl(divisor, x);
}

enum BinarySearchMode {
    KeyMustBePresentInArray,
    KeyMightNotBePresentInArray,
    ReturnAdjacentElementIfKeyIsNotPresent
};

template<typename ArrayElementType, typename KeyType, typename ArrayType, typename ExtractKey, BinarySearchMode mode>
inline ArrayElementType* binarySearchImpl(ArrayType& array, size_t size, KeyType key, const ExtractKey& extractKey = ExtractKey())
{
    size_t offset = 0;
    while (size > 1) {
        size_t pos = (size - 1) >> 1;
        KeyType val = extractKey(&array[offset + pos]);
        
        if (val == key)
            return &array[offset + pos];
        // The item we are looking for is smaller than the item being check; reduce the value of 'size',
        // chopping off the right hand half of the array.
        if (key < val)
            size = pos;
        // Discard all values in the left hand half of the array, up to and including the item at pos.
        else {
            size -= (pos + 1);
            offset += (pos + 1);
        }

        ASSERT(mode != KeyMustBePresentInArray || size);
    }
    
    if (mode == KeyMightNotBePresentInArray && !size)
        return 0;
    
    ArrayElementType* result = &array[offset];

    if (mode == KeyMightNotBePresentInArray && key != extractKey(result))
        return 0;

    if (mode == KeyMustBePresentInArray) {
        ASSERT(size == 1);
        ASSERT(key == extractKey(result));
    }

    return result;
}

// If the element is not found, crash if asserts are enabled, and behave like approximateBinarySearch in release builds.
template<typename ArrayElementType, typename KeyType, typename ArrayType, typename ExtractKey>
inline ArrayElementType* binarySearch(ArrayType& array, size_t size, KeyType key, ExtractKey extractKey = ExtractKey())
{
    return binarySearchImpl<ArrayElementType, KeyType, ArrayType, ExtractKey, KeyMustBePresentInArray>(array, size, key, extractKey);
}

// Return zero if the element is not found.
template<typename ArrayElementType, typename KeyType, typename ArrayType, typename ExtractKey>
inline ArrayElementType* tryBinarySearch(ArrayType& array, size_t size, KeyType key, ExtractKey extractKey = ExtractKey())
{
    return binarySearchImpl<ArrayElementType, KeyType, ArrayType, ExtractKey, KeyMightNotBePresentInArray>(array, size, key, extractKey);
}

// Return the element that is either to the left, or the right, of where the element would have been found.
template<typename ArrayElementType, typename KeyType, typename ArrayType, typename ExtractKey>
inline ArrayElementType* approximateBinarySearch(ArrayType& array, size_t size, KeyType key, ExtractKey extractKey = ExtractKey())
{
    return binarySearchImpl<ArrayElementType, KeyType, ArrayType, ExtractKey, ReturnAdjacentElementIfKeyIsNotPresent>(array, size, key, extractKey);
}

// Variants of the above that use const.
template<typename ArrayElementType, typename KeyType, typename ArrayType, typename ExtractKey>
inline ArrayElementType* binarySearch(const ArrayType& array, size_t size, KeyType key, ExtractKey extractKey = ExtractKey())
{
    return binarySearchImpl<ArrayElementType, KeyType, ArrayType, ExtractKey, KeyMustBePresentInArray>(const_cast<ArrayType&>(array), size, key, extractKey);
}
template<typename ArrayElementType, typename KeyType, typename ArrayType, typename ExtractKey>
inline ArrayElementType* tryBinarySearch(const ArrayType& array, size_t size, KeyType key, ExtractKey extractKey = ExtractKey())
{
    return binarySearchImpl<ArrayElementType, KeyType, ArrayType, ExtractKey, KeyMightNotBePresentInArray>(const_cast<ArrayType&>(array), size, key, extractKey);
}
template<typename ArrayElementType, typename KeyType, typename ArrayType, typename ExtractKey>
inline ArrayElementType* approximateBinarySearch(const ArrayType& array, size_t size, KeyType key, ExtractKey extractKey = ExtractKey())
{
    return binarySearchImpl<ArrayElementType, KeyType, ArrayType, ExtractKey, ReturnAdjacentElementIfKeyIsNotPresent>(const_cast<ArrayType&>(array), size, key, extractKey);
}

template<typename VectorType, typename ElementType>
inline void insertIntoBoundedVector(VectorType& vector, size_t size, const ElementType& element, size_t index)
{
    for (size_t i = size; i-- > index + 1;)
        vector[i] = vector[i - 1];
    vector[index] = element;
}

// This is here instead of CompilationThread.h to prevent that header from being included
// everywhere. The fact that this method, and that header, exist outside of JSC is a bug.
// https://bugs.webkit.org/show_bug.cgi?id=131815
WTF_EXPORT_PRIVATE bool isCompilationThread();

template<typename Func>
bool isStatelessLambda()
{
    return std::is_empty<Func>::value;
}

template<typename ResultType, typename Func, typename... ArgumentTypes>
ResultType callStatelessLambda(ArgumentTypes&&... arguments)
{
    uint64_t data[(sizeof(Func) + sizeof(uint64_t) - 1) / sizeof(uint64_t)];
    memset(data, 0, sizeof(data));
    return (*bitwise_cast<Func*>(data))(std::forward<ArgumentTypes>(arguments)...);
}

template<typename T, typename U>
bool checkAndSet(T& left, U right)
{
    if (left == right)
        return false;
    left = right;
    return true;
}

template<typename T>
bool findBitInWord(T word, size_t& index, size_t endIndex, bool value)
{
    static_assert(std::is_unsigned<T>::value, "Type used in findBitInWord must be unsigned");
    
    word >>= index;
    
    while (index < endIndex) {
        if ((word & 1) == static_cast<T>(value))
            return true;
        index++;
        word >>= 1;
    }
    
    index = endIndex;
    return false;
}

// Visitor adapted from http://stackoverflow.com/questions/25338795/is-there-a-name-for-this-tuple-creation-idiom

template <class A, class... B>
struct Visitor : Visitor<A>, Visitor<B...> {
    Visitor(A a, B... b)
        : Visitor<A>(a)
        , Visitor<B...>(b...)
    {
    }

    using Visitor<A>::operator ();
    using Visitor<B...>::operator ();
};
  
template <class A>
struct Visitor<A> : A {
    Visitor(A a)
        : A(a)
    {
    }

    using A::operator();
};
 
template <class... F>
Visitor<F...> makeVisitor(F... f)
{
    return Visitor<F...>(f...);
}

namespace Detail
{
    template <typename, template <typename...> class>
    struct IsTemplate_ : std::false_type
    {
    };

    template <typename... Ts, template <typename...> class C>
    struct IsTemplate_<C<Ts...>, C> : std::true_type
    {
    };
}

template <typename T, template <typename...> class Template>
struct IsTemplate : public std::integral_constant<bool, Detail::IsTemplate_<T, Template>::value> {};

namespace Detail
{
    template <template <typename...> class Base, typename Derived>
    struct IsBaseOfTemplateImpl
    {
        template <typename... Args>
        static std::true_type test(Base<Args...>*);
        static std::false_type test(void*);

        static constexpr const bool value = decltype(test(std::declval<typename std::remove_cv<Derived>::type*>()))::value;
    };
}

template <template <typename...> class Base, typename Derived>
struct IsBaseOfTemplate : public std::integral_constant<bool, Detail::IsBaseOfTemplateImpl<Base, Derived>::value> {};

template <class T>
struct RemoveCVAndReference  {
    typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
};

} // namespace WTF

// This version of placement new omits a 0 check.
enum NotNullTag { NotNull };
inline void* operator new(size_t, NotNullTag, void* location)
{
    ASSERT(location);
    return location;
}

// This adds various C++14 features for versions of the STL that may not yet have them.
namespace std {
#if COMPILER(CLANG) && __cplusplus < 201400L
template<class T> struct _Unique_if {
    typedef unique_ptr<T> _Single_object;
};

template<class T> struct _Unique_if<T[]> {
    typedef unique_ptr<T[]> _Unknown_bound;
};

template<class T, size_t N> struct _Unique_if<T[N]> {
    typedef void _Known_bound;
};

template<class T, class... Args> inline typename _Unique_if<T>::_Single_object
make_unique(Args&&... args)
{
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T> inline typename _Unique_if<T>::_Unknown_bound
make_unique(size_t n)
{
    typedef typename remove_extent<T>::type U;
    return unique_ptr<T>(new U[n]());
}

template<class T, class... Args> typename _Unique_if<T>::_Known_bound
make_unique(Args&&...) = delete;

// std::exchange
template<class T, class U = T>
T exchange(T& t, U&& newValue)
{
    T oldValue = std::move(t);
    t = std::forward<U>(newValue);

    return oldValue;
}
#endif

template<WTF::CheckMoveParameterTag, typename T>
ALWAYS_INLINE constexpr typename remove_reference<T>::type&& move(T&& value)
{
    static_assert(is_lvalue_reference<T>::value, "T is not an lvalue reference; move() is unnecessary.");

    using NonRefQualifiedType = typename remove_reference<T>::type;
    static_assert(!is_const<NonRefQualifiedType>::value, "T is const qualified.");

    return move(forward<T>(value));
}

} // namespace std

#define WTFMove(value) std::move<WTF::CheckMoveParameter>(value)

using WTF::KB;
using WTF::MB;
using WTF::GB;
using WTF::approximateBinarySearch;
using WTF::binarySearch;
using WTF::bitwise_cast;
using WTF::callStatelessLambda;
using WTF::checkAndSet;
using WTF::findBitInWord;
using WTF::insertIntoBoundedVector;
using WTF::isCompilationThread;
using WTF::isPointerAligned;
using WTF::isStatelessLambda;
using WTF::is8ByteAligned;
using WTF::roundUpToMultipleOf;
using WTF::safeCast;
using WTF::tryBinarySearch;

#if !COMPILER(CLANG) || __cplusplus >= 201400L
// We normally don't want to bring in entire std namespaces, but literals are an exception.
using namespace std::literals::chrono_literals;
#endif

#endif // WTF_StdLibExtras_h
