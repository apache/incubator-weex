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
#ifndef CheckedArithmetic_h
#define CheckedArithmetic_h

#include <wtf/Assertions.h>

#include <limits>
#include <stdint.h>
#include <type_traits>

/* Checked<T>
 *
 * This class provides a mechanism to perform overflow-safe integer arithmetic
 * without having to manually ensure that you have all the required bounds checks
 * directly in your code.
 *
 * There are two modes of operation:
 *  - The default is Checked<T, CrashOnOverflow>, and crashes at the point
 *    and overflow has occurred.
 *  - The alternative is Checked<T, RecordOverflow>, which uses an additional
 *    byte of storage to track whether an overflow has occurred, subsequent
 *    unchecked operations will crash if an overflow has occured
 *
 * It is possible to provide a custom overflow handler, in which case you need
 * to support these functions:
 *  - void overflowed();
 *    This function is called when an operation has produced an overflow.
 *  - bool hasOverflowed();
 *    This function must return true if overflowed() has been called on an
 *    instance and false if it has not.
 *  - void clearOverflow();
 *    Used to reset overflow tracking when a value is being overwritten with
 *    a new value.
 *
 * Checked<T> works for all integer types, with the following caveats:
 *  - Mixing signedness of operands is only supported for types narrower than
 *    64bits.
 *  - It does have a performance impact, so tight loops may want to be careful
 *    when using it.
 *
 */

namespace WTF {

enum class CheckedState {
    DidOverflow,
    DidNotOverflow
};

class CrashOnOverflow {
public:
    static NO_RETURN_DUE_TO_CRASH void overflowed()
    {
        crash();
    }

    void clearOverflow() { }

    static NO_RETURN_DUE_TO_CRASH void crash()
    {
        CRASH();
    }

public:
    bool hasOverflowed() const { return false; }
};

class RecordOverflow {
protected:
    RecordOverflow()
        : m_overflowed(false)
    {
    }

    void overflowed()
    {
        m_overflowed = true;
    }

    void clearOverflow()
    {
        m_overflowed = false;
    }

    static NO_RETURN_DUE_TO_CRASH void crash()
    {
        CRASH();
    }

public:
    bool hasOverflowed() const { return m_overflowed; }

private:
    unsigned char m_overflowed;
};

template <typename T, class OverflowHandler = CrashOnOverflow> class Checked;
template <typename T> struct RemoveChecked;
template <typename T> struct RemoveChecked<Checked<T>>;

template <typename Target, typename Source, bool isTargetBigger = sizeof(Target) >= sizeof(Source), bool targetSigned = std::numeric_limits<Target>::is_signed, bool sourceSigned = std::numeric_limits<Source>::is_signed> struct BoundsChecker;
template <typename Target, typename Source> struct BoundsChecker<Target, Source, false, false, false> {
    static bool inBounds(Source value)
    {
        // Same signedness so implicit type conversion will always increase precision to widest type.
        return value <= std::numeric_limits<Target>::max();
    }
};
template <typename Target, typename Source> struct BoundsChecker<Target, Source, false, true, true> {
    static bool inBounds(Source value)
    {
        // Same signedness so implicit type conversion will always increase precision to widest type.
        return std::numeric_limits<Target>::min() <= value && value <= std::numeric_limits<Target>::max();
    }
};

template <typename Target, typename Source> struct BoundsChecker<Target, Source, false, false, true> {
    static bool inBounds(Source value)
    {
        // When converting value to unsigned Source, value will become a big value if value is negative.
        // Casted value will become bigger than Target::max as Source is bigger than Target.
        return static_cast<typename std::make_unsigned<Source>::type>(value) <= std::numeric_limits<Target>::max();
    }
};

template <typename Target, typename Source> struct BoundsChecker<Target, Source, false, true, false> {
    static bool inBounds(Source value)
    {
        // The unsigned Source type has greater precision than the target so max(Target) -> Source will widen.
        return value <= static_cast<Source>(std::numeric_limits<Target>::max());
    }
};

template <typename Target, typename Source> struct BoundsChecker<Target, Source, true, false, false> {
    static bool inBounds(Source)
    {
        // Same sign, greater or same precision.
        return true;
    }
};

template <typename Target, typename Source> struct BoundsChecker<Target, Source, true, true, true> {
    static bool inBounds(Source)
    {
        // Same sign, greater or same precision.
        return true;
    }
};

template <typename Target, typename Source> struct BoundsChecker<Target, Source, true, true, false> {
    static bool inBounds(Source value)
    {
        // Target is signed with greater or same precision. If strictly greater, it is always safe.
        if (sizeof(Target) > sizeof(Source))
            return true;
        return value <= static_cast<Source>(std::numeric_limits<Target>::max());
    }
};

template <typename Target, typename Source> struct BoundsChecker<Target, Source, true, false, true> {
    static bool inBounds(Source value)
    {
        // Target is unsigned with greater precision.
        return value >= 0;
    }
};

template <typename Target, typename Source> static inline bool isInBounds(Source value)
{
    return BoundsChecker<Target, Source>::inBounds(value);
}

template <typename Target, typename Source> static inline bool convertSafely(Source input, Target& output)
{
    if (!isInBounds<Target>(input))
        return false;
    output = static_cast<Target>(input);
    return true;
}

template <typename T> struct RemoveChecked {
    typedef T CleanType;
    static const CleanType DefaultValue = 0;    
};

template <typename T> struct RemoveChecked<Checked<T, CrashOnOverflow>> {
    typedef typename RemoveChecked<T>::CleanType CleanType;
    static const CleanType DefaultValue = 0;
};

template <typename T> struct RemoveChecked<Checked<T, RecordOverflow>> {
    typedef typename RemoveChecked<T>::CleanType CleanType;
    static const CleanType DefaultValue = 0;
};

// The ResultBase and SignednessSelector are used to workaround typeof not being
// available in MSVC
template <typename U, typename V, bool uIsBigger = (sizeof(U) > sizeof(V)), bool sameSize = (sizeof(U) == sizeof(V))> struct ResultBase;
template <typename U, typename V> struct ResultBase<U, V, true, false> {
    typedef U ResultType;
};

template <typename U, typename V> struct ResultBase<U, V, false, false> {
    typedef V ResultType;
};

template <typename U> struct ResultBase<U, U, false, true> {
    typedef U ResultType;
};

template <typename U, typename V, bool uIsSigned = std::numeric_limits<U>::is_signed, bool vIsSigned = std::numeric_limits<V>::is_signed> struct SignednessSelector;
template <typename U, typename V> struct SignednessSelector<U, V, true, true> {
    typedef U ResultType;
};

template <typename U, typename V> struct SignednessSelector<U, V, false, false> {
    typedef U ResultType;
};

template <typename U, typename V> struct SignednessSelector<U, V, true, false> {
    typedef V ResultType;
};

template <typename U, typename V> struct SignednessSelector<U, V, false, true> {
    typedef U ResultType;
};

template <typename U, typename V> struct ResultBase<U, V, false, true> {
    typedef typename SignednessSelector<U, V>::ResultType ResultType;
};

template <typename U, typename V> struct Result : ResultBase<typename RemoveChecked<U>::CleanType, typename RemoveChecked<V>::CleanType> {
};

template <typename LHS, typename RHS, typename ResultType = typename Result<LHS, RHS>::ResultType, 
    bool lhsSigned = std::numeric_limits<LHS>::is_signed, bool rhsSigned = std::numeric_limits<RHS>::is_signed> struct ArithmeticOperations;

template <typename LHS, typename RHS, typename ResultType> struct ArithmeticOperations<LHS, RHS, ResultType, true, true> {
    // LHS and RHS are signed types

    // Helper function
    static inline bool signsMatch(LHS lhs, RHS rhs)
    {
        return (lhs ^ rhs) >= 0;
    }

    static inline bool add(LHS lhs, RHS rhs, ResultType& result) WARN_UNUSED_RETURN
    {
        if (signsMatch(lhs, rhs)) {
            if (lhs >= 0) {
                if ((std::numeric_limits<ResultType>::max() - rhs) < lhs)
                    return false;
            } else {
                ResultType temp = lhs - std::numeric_limits<ResultType>::min();
                if (rhs < -temp)
                    return false;
            }
        } // if the signs do not match this operation can't overflow
        result = lhs + rhs;
        return true;
    }

    static inline bool sub(LHS lhs, RHS rhs, ResultType& result) WARN_UNUSED_RETURN
    {
        if (!signsMatch(lhs, rhs)) {
            if (lhs >= 0) {
                if (lhs > std::numeric_limits<ResultType>::max() + rhs)
                    return false;
            } else {
                if (rhs > std::numeric_limits<ResultType>::max() + lhs)
                    return false;
            }
        } // if the signs match this operation can't overflow
        result = lhs - rhs;
        return true;
    }

    static inline bool multiply(LHS lhs, RHS rhs, ResultType& result) WARN_UNUSED_RETURN
    {
        if (signsMatch(lhs, rhs)) {
            if (lhs >= 0) {
                if (lhs && (std::numeric_limits<ResultType>::max() / lhs) < rhs)
                    return false;
            } else {
                if (static_cast<ResultType>(lhs) == std::numeric_limits<ResultType>::min() || static_cast<ResultType>(rhs) == std::numeric_limits<ResultType>::min())
                    return false;
                if ((std::numeric_limits<ResultType>::max() / -lhs) < -rhs)
                    return false;
            }
        } else {
            if (lhs < 0) {
                if (rhs && lhs < (std::numeric_limits<ResultType>::min() / rhs))
                    return false;
            } else {
                if (lhs && rhs < (std::numeric_limits<ResultType>::min() / lhs))
                    return false;
            }
        }
        result = lhs * rhs;
        return true;
    }

    static inline bool equals(LHS lhs, RHS rhs) { return lhs == rhs; }

};

template <typename LHS, typename RHS, typename ResultType> struct ArithmeticOperations<LHS, RHS, ResultType, false, false> {
    // LHS and RHS are unsigned types so bounds checks are nice and easy
    static inline bool add(LHS lhs, RHS rhs, ResultType& result) WARN_UNUSED_RETURN
    {
        ResultType temp = lhs + rhs;
        if (temp < lhs)
            return false;
        result = temp;
        return true;
    }

    static inline bool sub(LHS lhs, RHS rhs, ResultType& result) WARN_UNUSED_RETURN
    {
        ResultType temp = lhs - rhs;
        if (temp > lhs)
            return false;
        result = temp;
        return true;
    }

    static inline bool multiply(LHS lhs, RHS rhs, ResultType& result) WARN_UNUSED_RETURN
    {
        if (!lhs || !rhs) {
            result = 0;
            return true;
        }
        if (std::numeric_limits<ResultType>::max() / lhs < rhs)
            return false;
        result = lhs * rhs;
        return true;
    }

    static inline bool equals(LHS lhs, RHS rhs) { return lhs == rhs; }

};

template <typename ResultType> struct ArithmeticOperations<int, unsigned, ResultType, true, false> {
    static inline bool add(int64_t lhs, int64_t rhs, ResultType& result)
    {
        int64_t temp = lhs + rhs;
        if (temp < std::numeric_limits<ResultType>::min())
            return false;
        if (temp > std::numeric_limits<ResultType>::max())
            return false;
        result = static_cast<ResultType>(temp);
        return true;
    }
    
    static inline bool sub(int64_t lhs, int64_t rhs, ResultType& result)
    {
        int64_t temp = lhs - rhs;
        if (temp < std::numeric_limits<ResultType>::min())
            return false;
        if (temp > std::numeric_limits<ResultType>::max())
            return false;
        result = static_cast<ResultType>(temp);
        return true;
    }

    static inline bool multiply(int64_t lhs, int64_t rhs, ResultType& result)
    {
        int64_t temp = lhs * rhs;
        if (temp < std::numeric_limits<ResultType>::min())
            return false;
        if (temp > std::numeric_limits<ResultType>::max())
            return false;
        result = static_cast<ResultType>(temp);
        return true;
    }

    static inline bool equals(int lhs, unsigned rhs)
    {
        return static_cast<int64_t>(lhs) == static_cast<int64_t>(rhs);
    }
};

template <typename ResultType> struct ArithmeticOperations<unsigned, int, ResultType, false, true> {
    static inline bool add(int64_t lhs, int64_t rhs, ResultType& result)
    {
        return ArithmeticOperations<int, unsigned, ResultType>::add(rhs, lhs, result);
    }
    
    static inline bool sub(int64_t lhs, int64_t rhs, ResultType& result)
    {
        return ArithmeticOperations<int, unsigned, ResultType>::sub(lhs, rhs, result);
    }

    static inline bool multiply(int64_t lhs, int64_t rhs, ResultType& result)
    {
        return ArithmeticOperations<int, unsigned, ResultType>::multiply(rhs, lhs, result);
    }

    static inline bool equals(unsigned lhs, int rhs)
    {
        return ArithmeticOperations<int, unsigned, ResultType>::equals(rhs, lhs);
    }
};

template <typename U, typename V, typename R> static inline bool safeAdd(U lhs, V rhs, R& result)
{
    return ArithmeticOperations<U, V, R>::add(lhs, rhs, result);
}

template <typename U, typename V, typename R> static inline bool safeSub(U lhs, V rhs, R& result)
{
    return ArithmeticOperations<U, V, R>::sub(lhs, rhs, result);
}

template <typename U, typename V, typename R> static inline bool safeMultiply(U lhs, V rhs, R& result)
{
    return ArithmeticOperations<U, V, R>::multiply(lhs, rhs, result);
}

template <typename U, typename V> static inline bool safeEquals(U lhs, V rhs)
{
    return ArithmeticOperations<U, V>::equals(lhs, rhs);
}

enum ResultOverflowedTag { ResultOverflowed };
    
template <typename T, class OverflowHandler> class Checked : public OverflowHandler {
public:
    template <typename _T, class _OverflowHandler> friend class Checked;
    Checked()
        : m_value(0)
    {
    }

    Checked(ResultOverflowedTag)
        : m_value(0)
    {
        this->overflowed();
    }

    template <typename U> Checked(U value)
    {
        if (!isInBounds<T>(value))
            this->overflowed();
        m_value = static_cast<T>(value);
    }
    
    template <typename V> Checked(const Checked<T, V>& rhs)
        : m_value(rhs.m_value)
    {
        if (rhs.hasOverflowed())
            this->overflowed();
    }
    
    template <typename U> Checked(const Checked<U, OverflowHandler>& rhs)
        : OverflowHandler(rhs)
    {
        if (!isInBounds<T>(rhs.m_value))
            this->overflowed();
        m_value = static_cast<T>(rhs.m_value);
    }
    
    template <typename U, typename V> Checked(const Checked<U, V>& rhs)
    {
        if (rhs.hasOverflowed())
            this->overflowed();
        if (!isInBounds<T>(rhs.m_value))
            this->overflowed();
        m_value = static_cast<T>(rhs.m_value);
    }
    
    const Checked& operator=(Checked rhs)
    {
        this->clearOverflow();
        if (rhs.hasOverflowed())
            this->overflowed();
        m_value = static_cast<T>(rhs.m_value);
        return *this;
    }
    
    template <typename U> const Checked& operator=(U value)
    {
        return *this = Checked(value);
    }
    
    template <typename U, typename V> const Checked& operator=(const Checked<U, V>& rhs)
    {
        return *this = Checked(rhs);
    }
    
    // prefix
    const Checked& operator++()
    {
        if (m_value == std::numeric_limits<T>::max())
            this->overflowed();
        m_value++;
        return *this;
    }
    
    const Checked& operator--()
    {
        if (m_value == std::numeric_limits<T>::min())
            this->overflowed();
        m_value--;
        return *this;
    }
    
    // postfix operators
    const Checked operator++(int)
    {
        if (m_value == std::numeric_limits<T>::max())
            this->overflowed();
        return Checked(m_value++);
    }
    
    const Checked operator--(int)
    {
        if (m_value == std::numeric_limits<T>::min())
            this->overflowed();
        return Checked(m_value--);
    }
    
    // Boolean operators
    bool operator!() const
    {
        if (this->hasOverflowed())
            this->crash();
        return !m_value;
    }

    explicit operator bool() const
    {
        if (this->hasOverflowed())
            this->crash();
        return m_value;
    }

    // Value accessors. unsafeGet() will crash if there's been an overflow.
    T unsafeGet() const
    {
        if (this->hasOverflowed())
            this->crash();
        return m_value;
    }
    
    inline CheckedState safeGet(T& value) const WARN_UNUSED_RETURN
    {
        value = m_value;
        if (this->hasOverflowed())
            return CheckedState::DidOverflow;
        return CheckedState::DidNotOverflow;
    }

    // Mutating assignment
    template <typename U> const Checked operator+=(U rhs)
    {
        if (!safeAdd(m_value, rhs, m_value))
            this->overflowed();
        return *this;
    }

    template <typename U> const Checked operator-=(U rhs)
    {
        if (!safeSub(m_value, rhs, m_value))
            this->overflowed();
        return *this;
    }

    template <typename U> const Checked operator*=(U rhs)
    {
        if (!safeMultiply(m_value, rhs, m_value))
            this->overflowed();
        return *this;
    }

    const Checked operator*=(double rhs)
    {
        double result = rhs * m_value;
        // Handle +/- infinity and NaN
        if (!(std::numeric_limits<T>::min() <= result && std::numeric_limits<T>::max() >= result))
            this->overflowed();
        m_value = (T)result;
        return *this;
    }

    const Checked operator*=(float rhs)
    {
        return *this *= (double)rhs;
    }
    
    template <typename U, typename V> const Checked operator+=(Checked<U, V> rhs)
    {
        if (rhs.hasOverflowed())
            this->overflowed();
        return *this += rhs.m_value;
    }

    template <typename U, typename V> const Checked operator-=(Checked<U, V> rhs)
    {
        if (rhs.hasOverflowed())
            this->overflowed();
        return *this -= rhs.m_value;
    }

    template <typename U, typename V> const Checked operator*=(Checked<U, V> rhs)
    {
        if (rhs.hasOverflowed())
            this->overflowed();
        return *this *= rhs.m_value;
    }

    // Equality comparisons
    template <typename V> bool operator==(Checked<T, V> rhs)
    {
        return unsafeGet() == rhs.unsafeGet();
    }

    template <typename U> bool operator==(U rhs)
    {
        if (this->hasOverflowed())
            this->crash();
        return safeEquals(m_value, rhs);
    }
    
    template <typename U, typename V> const Checked operator==(Checked<U, V> rhs)
    {
        return unsafeGet() == Checked(rhs.unsafeGet());
    }

    template <typename U> bool operator!=(U rhs)
    {
        return !(*this == rhs);
    }

    // Other comparisons
    template <typename V> bool operator<(Checked<T, V> rhs) const
    {
        return unsafeGet() < rhs.unsafeGet();
    }

    bool operator<(T rhs) const
    {
        return unsafeGet() < rhs;
    }

    template <typename V> bool operator<=(Checked<T, V> rhs) const
    {
        return unsafeGet() <= rhs.unsafeGet();
    }

    bool operator<=(T rhs) const
    {
        return unsafeGet() <= rhs;
    }

    template <typename V> bool operator>(Checked<T, V> rhs) const
    {
        return unsafeGet() > rhs.unsafeGet();
    }

    bool operator>(T rhs) const
    {
        return unsafeGet() > rhs;
    }

    template <typename V> bool operator>=(Checked<T, V> rhs) const
    {
        return unsafeGet() >= rhs.unsafeGet();
    }

    bool operator>=(T rhs) const
    {
        return unsafeGet() >= rhs;
    }

private:
    // Disallow implicit conversion of floating point to integer types
    Checked(float);
    Checked(double);
    void operator=(float);
    void operator=(double);
    void operator+=(float);
    void operator+=(double);
    void operator-=(float);
    void operator-=(double);
    T m_value;
};

template <typename U, typename V, typename OverflowHandler> static inline Checked<typename Result<U, V>::ResultType, OverflowHandler> operator+(Checked<U, OverflowHandler> lhs, Checked<V, OverflowHandler> rhs)
{
    U x = 0;
    V y = 0;
    bool overflowed = lhs.safeGet(x) == CheckedState::DidOverflow || rhs.safeGet(y) == CheckedState::DidOverflow;
    typename Result<U, V>::ResultType result = 0;
    overflowed |= !safeAdd(x, y, result);
    if (overflowed)
        return ResultOverflowed;
    return result;
}

template <typename U, typename V, typename OverflowHandler> static inline Checked<typename Result<U, V>::ResultType, OverflowHandler> operator-(Checked<U, OverflowHandler> lhs, Checked<V, OverflowHandler> rhs)
{
    U x = 0;
    V y = 0;
    bool overflowed = lhs.safeGet(x) == CheckedState::DidOverflow || rhs.safeGet(y) == CheckedState::DidOverflow;
    typename Result<U, V>::ResultType result = 0;
    overflowed |= !safeSub(x, y, result);
    if (overflowed)
        return ResultOverflowed;
    return result;
}

template <typename U, typename V, typename OverflowHandler> static inline Checked<typename Result<U, V>::ResultType, OverflowHandler> operator*(Checked<U, OverflowHandler> lhs, Checked<V, OverflowHandler> rhs)
{
    U x = 0;
    V y = 0;
    bool overflowed = lhs.safeGet(x) == CheckedState::DidOverflow || rhs.safeGet(y) == CheckedState::DidOverflow;
    typename Result<U, V>::ResultType result = 0;
    overflowed |= !safeMultiply(x, y, result);
    if (overflowed)
        return ResultOverflowed;
    return result;
}

template <typename U, typename V, typename OverflowHandler> static inline Checked<typename Result<U, V>::ResultType, OverflowHandler> operator+(Checked<U, OverflowHandler> lhs, V rhs)
{
    return lhs + Checked<V, OverflowHandler>(rhs);
}

template <typename U, typename V, typename OverflowHandler> static inline Checked<typename Result<U, V>::ResultType, OverflowHandler> operator-(Checked<U, OverflowHandler> lhs, V rhs)
{
    return lhs - Checked<V, OverflowHandler>(rhs);
}

template <typename U, typename V, typename OverflowHandler> static inline Checked<typename Result<U, V>::ResultType, OverflowHandler> operator*(Checked<U, OverflowHandler> lhs, V rhs)
{
    return lhs * Checked<V, OverflowHandler>(rhs);
}

template <typename U, typename V, typename OverflowHandler> static inline Checked<typename Result<U, V>::ResultType, OverflowHandler> operator+(U lhs, Checked<V, OverflowHandler> rhs)
{
    return Checked<U, OverflowHandler>(lhs) + rhs;
}

template <typename U, typename V, typename OverflowHandler> static inline Checked<typename Result<U, V>::ResultType, OverflowHandler> operator-(U lhs, Checked<V, OverflowHandler> rhs)
{
    return Checked<U, OverflowHandler>(lhs) - rhs;
}

template <typename U, typename V, typename OverflowHandler> static inline Checked<typename Result<U, V>::ResultType, OverflowHandler> operator*(U lhs, Checked<V, OverflowHandler> rhs)
{
    return Checked<U, OverflowHandler>(lhs) * rhs;
}

// Convenience typedefs.
typedef Checked<int8_t, RecordOverflow> CheckedInt8;
typedef Checked<uint8_t, RecordOverflow> CheckedUint8;
typedef Checked<int16_t, RecordOverflow> CheckedInt16;
typedef Checked<uint16_t, RecordOverflow> CheckedUint16;
typedef Checked<int32_t, RecordOverflow> CheckedInt32;
typedef Checked<uint32_t, RecordOverflow> CheckedUint32;
typedef Checked<int64_t, RecordOverflow> CheckedInt64;
typedef Checked<uint64_t, RecordOverflow> CheckedUint64;
typedef Checked<size_t, RecordOverflow> CheckedSize;

template<typename T, typename U>
Checked<T, RecordOverflow> checkedSum(U value)
{
    return Checked<T, RecordOverflow>(value);
}
template<typename T, typename U, typename... Args>
Checked<T, RecordOverflow> checkedSum(U value, Args... args)
{
    return Checked<T, RecordOverflow>(value) + checkedSum<T>(args...);
}

// Sometimes, you just want to check if some math would overflow - the code to do the math is
// already in place, and you want to guard it.

template<typename T, typename... Args> bool sumOverflows(Args... args)
{
    return checkedSum<T>(args...).hasOverflowed();
}

template<typename T, typename U> bool differenceOverflows(U left, U right)
{
    return (Checked<T, RecordOverflow>(left) - Checked<T, RecordOverflow>(right)).hasOverflowed();
}

template<typename T, typename U>
Checked<T, RecordOverflow> checkedProduct(U value)
{
    return Checked<T, RecordOverflow>(value);
}
template<typename T, typename U, typename... Args>
Checked<T, RecordOverflow> checkedProduct(U value, Args... args)
{
    return Checked<T, RecordOverflow>(value) * checkedProduct<T>(args...);
}

// Sometimes, you just want to check if some math would overflow - the code to do the math is
// already in place, and you want to guard it.

template<typename T, typename... Args> bool productOverflows(Args... args)
{
    return checkedProduct<T>(args...).hasOverflowed();
}

}

using WTF::Checked;
using WTF::CheckedState;
using WTF::RecordOverflow;
using WTF::CheckedInt8;
using WTF::CheckedUint8;
using WTF::CheckedInt16;
using WTF::CheckedUint16;
using WTF::CheckedInt32;
using WTF::CheckedUint32;
using WTF::CheckedInt64;
using WTF::CheckedUint64;
using WTF::CheckedSize;
using WTF::checkedSum;
using WTF::differenceOverflows;
using WTF::productOverflows;
using WTF::sumOverflows;

#endif
