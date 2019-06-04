/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

// WTF::Expected is based on std::expected, as described here: http://wg21.link/p0323r1
// The specification expects to throw. This implementation doesn't support exceptions, uses RELEASE_ASSERT instead.

#pragma once

#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <wtf/Assertions.h>
#include <wtf/Compiler.h>
#include <wtf/Optional.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

template <class E>
class UnexpectedType {
public:
    UnexpectedType() = delete;
    constexpr explicit UnexpectedType(const E& e) : val(e) { }
    constexpr explicit UnexpectedType(E&& e) : val(std::forward<E>(e)) { }
    constexpr const E& value() const& { return val; }
    RELAXED_CONSTEXPR E& value() & { return val; }
    RELAXED_CONSTEXPR E&& value() && { return WTFMove(val); }

private:
    E val;
};

template <class E> constexpr bool operator==(const UnexpectedType<E>& lhs, const UnexpectedType<E>& rhs) { return lhs.value() == rhs.value(); }
template <class E> constexpr bool operator!=(const UnexpectedType<E>& lhs, const UnexpectedType<E>& rhs) { return lhs.value() != rhs.value(); }
template <class E> constexpr bool operator<(const UnexpectedType<E>& lhs, const UnexpectedType<E>& rhs) { return lhs.value() < rhs.value(); }
template <class E> constexpr bool operator>(const UnexpectedType<E>& lhs, const UnexpectedType<E>& rhs) { return lhs.value() > rhs.value(); }
template <class E> constexpr bool operator<=(const UnexpectedType<E>& lhs, const UnexpectedType<E>& rhs) { return lhs.value() <= rhs.value(); }
template <class E> constexpr bool operator>=(const UnexpectedType<E>& lhs, const UnexpectedType<E>& rhs) { return lhs.value() >= rhs.value(); }

template <class E> constexpr UnexpectedType<std::decay_t<E>> makeUnexpected(E&& v) { return UnexpectedType<typename std::decay<E>::type>(std::forward<E>(v)); }

struct UnexpectTag {
    UnexpectTag() = delete;
};
constexpr UnexpectTag Unexpect { };

namespace ExpectedDetail {

// Invoked where std::Expected would instead throw.
inline NO_RETURN_DUE_TO_CRASH void Throw() { RELEASE_ASSERT_NOT_REACHED(); }

static constexpr enum class ValueTagType { } ValueTag { };
static constexpr enum class ErrorTagType { } ErrorTag { };

template<class T, std::enable_if_t<std::is_trivially_destructible<T>::value>* = nullptr> void destroy(T&) { }
template<class T, std::enable_if_t<!std::is_trivially_destructible<T>::value && (std::is_class<T>::value || std::is_union<T>::value)>* = nullptr> void destroy(T& t) { t.~T(); }

template <class T, class E>
union ConstexprStorage {
    typedef T ValueType;
    typedef E ErrorType;
    char dummy;
    ValueType val;
    ErrorType err;
    constexpr ConstexprStorage() : dummy() { }
    constexpr ConstexprStorage(ValueTagType) : val() { }
    constexpr ConstexprStorage(ErrorTagType) : err() { }
    constexpr ConstexprStorage(ValueTagType, const ValueType& v) : val(v) { }
    constexpr ConstexprStorage(ErrorTagType, const ErrorType& e) : err(e) { }
    ~ConstexprStorage() = default;
};

template <class T, class E>
union Storage {
    typedef T ValueType;
    typedef E ErrorType;
    char dummy;
    ValueType val;
    ErrorType err;
    constexpr Storage() : dummy() { }
    constexpr Storage(ValueTagType) : val() { }
    constexpr Storage(ErrorTagType) : err() { }
    constexpr Storage(ValueTagType, const ValueType& val) : val(val) { }
    constexpr Storage(ValueTagType, ValueType&& val) : val(std::forward<ValueType>(val)) { }
    constexpr Storage(ErrorTagType, const ErrorType& err) : err(err) { }
    constexpr Storage(ErrorTagType, ErrorType&& err) : err(std::forward<ErrorType>(err)) { }
    ~Storage() { }
};

template <class E>
union ConstexprStorage<void, E> {
    typedef void ValueType;
    typedef E ErrorType;
    char dummy;
    ErrorType err;
    constexpr ConstexprStorage() : dummy() { }
    constexpr ConstexprStorage(ValueTagType) : dummy() { }
    constexpr ConstexprStorage(ErrorTagType) : err() { }
    constexpr ConstexprStorage(ErrorTagType, const ErrorType& e) : err(e) { }
    ~ConstexprStorage() = default;
};

template <class E>
union Storage<void, E> {
    typedef void ValueType;
    typedef E ErrorType;
    char dummy;
    ErrorType err;
    constexpr Storage() : dummy() { }
    constexpr Storage(ValueTagType) : dummy() { }
    constexpr Storage(ErrorTagType) : err() { }
    constexpr Storage(ErrorTagType, const ErrorType& err) : err(err) { }
    constexpr Storage(ErrorTagType, ErrorType&& err) : err(std::forward<ErrorType>(err)) { }
    ~Storage() { }
};

template <class T, class E>
struct ConstexprBase {
    typedef T ValueType;
    typedef E ErrorType;
    ConstexprStorage<ValueType, ErrorType> s;
    bool has;
    constexpr ConstexprBase() : s(), has(true) { }
    constexpr ConstexprBase(ValueTagType tag) : s(tag), has(true) { }
    constexpr ConstexprBase(ErrorTagType tag) : s(tag), has(false) { }
    constexpr ConstexprBase(ValueTagType tag, const ValueType& val) : s(tag, val), has(true) { }
    constexpr ConstexprBase(ErrorTagType tag, const ErrorType& err) : s(tag, err), has(false) { }
    ~ConstexprBase() = default;
};

template <class T, class E>
struct Base {
    typedef T ValueType;
    typedef E ErrorType;
    Storage<ValueType, ErrorType> s;
    bool has;
    constexpr Base() : s(), has(true) { }
    constexpr Base(ValueTagType tag) : s(tag), has(true) { }
    constexpr Base(ErrorTagType tag) : s(tag), has(false) { }
    constexpr Base(ValueTagType tag, const ValueType& val) : s(tag, val), has(true) { }
    constexpr Base(ValueTagType tag, ValueType&& val) : s(tag, std::forward<ValueType>(val)), has(true) { }
    constexpr Base(ErrorTagType tag, const ErrorType& err) : s(tag, err), has(false) { }
    constexpr Base(ErrorTagType tag, ErrorType&& err) : s(tag, std::forward<ErrorType>(err)), has(false) { }
    Base(const Base& o)
        : has(o.has)
    {
        if (has)
            ::new (&s.val) ValueType(o.s.val);
        else
            ::new (&s.err) ErrorType(o.s.err);
    }
    Base(Base&& o)
        : has(o.has)
    {
        if (has)
            ::new (&s.val) ValueType(WTFMove(o.s.val));
        else
            ::new (&s.err) ErrorType(WTFMove(o.s.err));
    }
    ~Base()
    {
        if (has)
            destroy(s.val);
        else
            destroy(s.err);
    }
};

template <class E>
struct ConstexprBase<void, E> {
    typedef void ValueType;
    typedef E ErrorType;
    ConstexprStorage<ValueType, ErrorType> s;
    bool has;
    constexpr ConstexprBase() : s(), has(true) { }
    constexpr ConstexprBase(ValueTagType tag) : s(tag), has(true) { }
    constexpr ConstexprBase(ErrorTagType tag) : s(tag), has(false) { }
    constexpr ConstexprBase(ErrorTagType tag, const ErrorType& err) : s(tag, err), has(false) { }
    constexpr ConstexprBase(ErrorTagType tag, ErrorType&& err) : s(tag, std::forward<ErrorType>(err)), has(false) { }
    ~ConstexprBase() = default;
};

template <class E>
struct Base<void, E> {
    typedef void ValueType;
    typedef E ErrorType;
    Storage<ValueType, ErrorType> s;
    bool has;
    constexpr Base() : s(), has(true) { }
    constexpr Base(ValueTagType tag) : s(tag), has(true) { }
    constexpr Base(ErrorTagType tag) : s(tag), has(false) { }
    constexpr Base(ErrorTagType tag, const ErrorType& err) : s(tag, err), has(false) { }
    constexpr Base(ErrorTagType tag, ErrorType&& err) : s(tag, std::forward<ErrorType>(err)), has(false) { }
    Base(const Base& o)
        : has(o.has)
    {
        if (!has)
            ::new (&s.err) ErrorType(o.s.err);
    }
    Base(Base&& o)
        : has(o.has)
    {
        if (!has)
            ::new (&s.err) ErrorType(WTFMove(o.s.err));
    }
    ~Base()
    {
        if (!has)
            destroy(s.err);
    }
};

template <class T, class E>
using BaseSelect = typename std::conditional<
    ((std::is_void<T>::value || std::is_trivially_destructible<T>::value)
        && std::is_trivially_destructible<E>::value),
    ConstexprBase<typename std::remove_const<T>::type, typename std::remove_const<E>::type>,
    Base<typename std::remove_const<T>::type, typename std::remove_const<E>::type>
>::type;

} // namespace ExpectedDetail

template <class T, class E>
class Expected : private ExpectedDetail::BaseSelect<T, E> {
    typedef ExpectedDetail::BaseSelect<T, E> base;

public:
    typedef typename base::ValueType ValueType;
    typedef typename base::ErrorType ErrorType;

private:
    typedef Expected<ValueType, ErrorType> type;

public:
    // template <class U> struct rebind { using type = Expected<U, ErrorType>; };

    constexpr Expected() : base(ExpectedDetail::ValueTag) { }
    Expected(const Expected&) = default;
    Expected(Expected&&) = default;
    constexpr Expected(const ValueType& e) : base(ExpectedDetail::ValueTag, e) { }
    constexpr Expected(ValueType&& e) : base(ExpectedDetail::ValueTag, std::forward<ValueType>(e)) { }
    // template <class... Args> constexpr explicit Expected(in_place_t, Args&&...);
    // template <class U, class... Args> constexpr explicit Expected(in_place_t, std::initializer_list<U>, Args&&...);
    constexpr Expected(UnexpectedType<ErrorType> const& u) : base(ExpectedDetail::ErrorTag, u.value()) { }
    constexpr Expected(UnexpectedType<ErrorType>&& u) : base(ExpectedDetail::ErrorTag, std::forward<UnexpectedType<E>>(u).value()) { }
    template <class Err> constexpr Expected(UnexpectedType<Err> const& u) : base(ExpectedDetail::ErrorTag, u.value()) { }
    // template <class... Args> constexpr explicit Expected(UnexpectTag, Args&&...);
    // template <class U, class... Args> constexpr explicit Expected(UnexpectTag, std::initializer_list<U>, Args&&...);

    ~Expected() = default;

    Expected& operator=(const Expected& e) { type(e).swap(*this); return *this; }
    Expected& operator=(Expected&& e) { type(WTFMove(e)).swap(*this); return *this; }
    template <class U> Expected& operator=(U&& u) { type(WTFMove(u)).swap(*this); return *this; }
    Expected& operator=(const UnexpectedType<ErrorType>& u) { type(u).swap(*this); return *this; }
    Expected& operator=(UnexpectedType<ErrorType>&& u) { type(WTFMove(u)).swap(*this); return *this; }
    // template <class... Args> void emplace(Args&&...);
    // template <class U, class... Args> void emplace(std::initializer_list<U>, Args&&...);

    void swap(Expected& o)
    {
        using std::swap;
        if (base::has && o.has)
            swap(base::s.val, o.s.val);
        else if (base::has && !o.has) {
            ErrorType e(WTFMove(o.s.err));
            ExpectedDetail::destroy(o.s.err);
            ::new (&o.s.val) ValueType(WTFMove(base::s.val));
            ExpectedDetail::destroy(base::s.val);
            ::new (&base::s.err) ErrorType(WTFMove(e));
            swap(base::has, o.has);
        } else if (!base::has && o.has) {
            ValueType v(WTFMove(o.s.val));
            ExpectedDetail::destroy(o.s.val);
            ::new (&o.s.err) ErrorType(WTFMove(base::s.err));
            ExpectedDetail::destroy(base::s.err);
            ::new (&base::s.val) ValueType(WTFMove(v));
            swap(base::has, o.has);
        } else
            swap(base::s.err, o.s.err);
    }

    constexpr const ValueType* operator->() const { return &base::s.val; }
    ValueType* operator->() { return &base::s.val; }
    constexpr const ValueType& operator*() const & { return base::s.val; }
    ValueType& operator*() & { return base::s.val; }
    constexpr const ValueType&& operator*() const && { return WTFMove(base::s.val); }
    RELAXED_CONSTEXPR ValueType&& operator*() && { return WTFMove(base::s.val); }
    constexpr explicit operator bool() const { return base::has; }
    constexpr bool hasValue() const { return base::has; }
    constexpr const ValueType& value() const & { return base::has ? base::s.val : (ExpectedDetail::Throw(), base::s.val); }
    RELAXED_CONSTEXPR ValueType& value() & { return base::has ? base::s.val : (ExpectedDetail::Throw(), base::s.val); }
    constexpr const ValueType&& value() const && { return base::has ? base::s.val : (ExpectedDetail::Throw(), base::s.val); }
    RELAXED_CONSTEXPR ValueType&& value() && { return WTFMove(base::has ? base::s.val : (ExpectedDetail::Throw(), base::s.val)); }
    constexpr const ErrorType& error() const & { return !base::has ? base::s.err : (ExpectedDetail::Throw(), base::s.err); }
    ErrorType& error() & { return !base::has ? base::s.err : (ExpectedDetail::Throw(), base::s.err); }
    RELAXED_CONSTEXPR ErrorType&& error() && { return !base::has ? base::s.err : (ExpectedDetail::Throw(), base::s.err); }
    constexpr const ErrorType&& error() const && { return !base::has ? base::s.err : (ExpectedDetail::Throw(), base::s.err); }
    constexpr UnexpectedType<ErrorType> getUnexpected() const { return UnexpectedType<ErrorType>(base::s.err); }
    template <class U> constexpr ValueType valueOr(U&& u) const & { return base::has ? **this : static_cast<ValueType>(std::forward<U>(u)); }
    template <class U> ValueType valueOr(U&& u) && { return base::has ? WTFMove(**this) : static_cast<ValueType>(std::forward<U>(u)); }
};

template <class E>
class Expected<void, E> : private ExpectedDetail::BaseSelect<void, E> {
    typedef ExpectedDetail::BaseSelect<void, E> base;

public:
    typedef typename base::ValueType ValueType;
    typedef typename base::ErrorType ErrorType;

private:
    typedef Expected<ValueType, ErrorType> type;

public:
    // template <class U> struct rebind { typedef Expected<U, ErrorType> type; };

    constexpr Expected() : base(ExpectedDetail::ValueTag) { }
    Expected(const Expected&) = default;
    Expected(Expected&&) = default;
    // constexpr explicit Expected(in_place_t);
    constexpr Expected(UnexpectedType<E> const& u) : base(ExpectedDetail::ErrorTag, u.value()) { }
    constexpr Expected(UnexpectedType<E>&& u) : base(ExpectedDetail::ErrorTag, std::forward<UnexpectedType<E>>(u).value()) { }
    template <class Err> constexpr Expected(UnexpectedType<Err> const& u) : base(ExpectedDetail::ErrorTag, u.value()) { }

    ~Expected() = default;

    Expected& operator=(const Expected& e) { type(e).swap(*this); return *this; }
    Expected& operator=(Expected&& e) { type(WTFMove(e)).swap(*this); return *this; }
    Expected& operator=(const UnexpectedType<E>& u) { type(u).swap(*this); return *this; } // Not in the current paper.
    Expected& operator=(UnexpectedType<E>&& u) { type(WTFMove(u)).swap(*this); return *this; } // Not in the current paper.
    // void emplace();

    void swap(Expected& o)
    {
        using std::swap;
        if (base::has && o.has) {
            // Do nothing.
        } else if (base::has && !o.has) {
            ErrorType e(WTFMove(o.s.err));
            ::new (&base::s.err) ErrorType(e);
            swap(base::has, o.has);
        } else if (!base::has && o.has) {
            ::new (&o.s.err) ErrorType(WTFMove(base::s.err));
            swap(base::has, o.has);
        } else
            swap(base::s.err, o.s.err);
    }

    constexpr explicit operator bool() const { return base::has; }
    constexpr bool hasValue() const { return base::has; }
    void value() const { !base::has ? ExpectedDetail::Throw() : void(); }
    constexpr const E& error() const & { return !base::has ? base::s.err : (ExpectedDetail::Throw(), base::s.err); }
    E& error() & { return !base::has ? base::s.err : (ExpectedDetail::Throw(), base::s.err); } // Not in the current paper.
    RELAXED_CONSTEXPR E&& error() && { return !base::has ? base::s.err : (ExpectedDetail::Throw(), base::s.err); }
    constexpr const E&& error() const && { return !base::has ? base::s.err : (ExpectedDetail::Throw(), base::s.err); } // Not in the current paper.
    // constexpr E& error() &;
    constexpr UnexpectedType<E> getUnexpected() const { return UnexpectedType<E>(base::s.err); }
};

template <class T, class E> constexpr bool operator==(const Expected<T, E>& x, const Expected<T, E>& y) { return bool(x) == bool(y) && (x ? x.value() == y.value() : x.error() == y.error()); }
template <class T, class E> constexpr bool operator!=(const Expected<T, E>& x, const Expected<T, E>& y) { return !(x == y); }
template <class T, class E> constexpr bool operator<(const Expected<T, E>& x, const Expected<T, E>& y) { return (!bool(x) && bool(y)) ? false : ((bool(x) && !bool(y)) ? true : ((bool(x) && bool(y)) ? x.value() < y.value() : x.error() < y.error())); }
template <class T, class E> constexpr bool operator>(const Expected<T, E>& x, const Expected<T, E>& y) { return !(x == y) && !(x < y); }
template <class T, class E> constexpr bool operator<=(const Expected<T, E>& x, const Expected<T, E>& y) { return (x == y) || (x < y); }
template <class T, class E> constexpr bool operator>=(const Expected<T, E>& x, const Expected<T, E>& y) { return (x == y) || (x > y); }

template <class E> constexpr bool operator==(const Expected<void, E>& x, const Expected<void, E>& y) { return bool(x) == bool(y) && (x ? true : x.error() == y.error()); } // Not in the current paper.
template <class E> constexpr bool operator<(const Expected<void, E>& x, const Expected<void, E>& y) { return (!bool(x) && bool(y)) ? false : ((bool(x) && !bool(y)) ? true : ((bool(x) && bool(y)) ? false : x.error() < y.error())); } // Not in the current paper.

template <class T, class E> constexpr bool operator==(const Expected<T, E>& x, const T& y) { return x == Expected<T, E>(y); }
template <class T, class E> constexpr bool operator==(const T& x, const Expected<T, E>& y) { return Expected<T, E>(x) == y; }
template <class T, class E> constexpr bool operator!=(const Expected<T, E>& x, const T& y) { return x != Expected<T, E>(y); }
template <class T, class E> constexpr bool operator!=(const T& x, const Expected<T, E>& y) { return Expected<T, E>(x) != y; }
template <class T, class E> constexpr bool operator<(const Expected<T, E>& x, const T& y) { return x < Expected<T, E>(y); }
template <class T, class E> constexpr bool operator<(const T& x, const Expected<T, E>& y) { return Expected<T, E>(x) < y; }
template <class T, class E> constexpr bool operator<=(const Expected<T, E>& x, const T& y) { return x <= Expected<T, E>(y); }
template <class T, class E> constexpr bool operator<=(const T& x, const Expected<T, E>& y) { return Expected<T, E>(x) <= y; }
template <class T, class E> constexpr bool operator>(const Expected<T, E>& x, const T& y) { return x > Expected<T, E>(y); }
template <class T, class E> constexpr bool operator>(const T& x, const Expected<T, E>& y) { return Expected<T, E>(x) > y; }
template <class T, class E> constexpr bool operator>=(const Expected<T, E>& x, const T& y) { return x >= Expected<T, E>(y); }
template <class T, class E> constexpr bool operator>=(const T& x, const Expected<T, E>& y) { return Expected<T, E>(x) >= y; }

template <class T, class E> constexpr bool operator==(const Expected<T, E>& x, const UnexpectedType<E>& y) { return x == Expected<T, E>(y); }
template <class T, class E> constexpr bool operator==(const UnexpectedType<E>& x, const Expected<T, E>& y) { return Expected<T, E>(x) == y; }
template <class T, class E> constexpr bool operator!=(const Expected<T, E>& x, const UnexpectedType<E>& y) { return x != Expected<T, E>(y); }
template <class T, class E> constexpr bool operator!=(const UnexpectedType<E>& x, const Expected<T, E>& y) { return Expected<T, E>(x) != y; }
template <class T, class E> constexpr bool operator<(const Expected<T, E>& x, const UnexpectedType<E>& y) { return x < Expected<T, E>(y); }
template <class T, class E> constexpr bool operator<(const UnexpectedType<E>& x, const Expected<T, E>& y) {  return Expected<T, E>(x) < y; }
template <class T, class E> constexpr bool operator<=(const Expected<T, E>& x, const UnexpectedType<E>& y) { return x <= Expected<T, E>(y); }
template <class T, class E> constexpr bool operator<=(const UnexpectedType<E>& x, const Expected<T, E>& y) { return Expected<T, E>(x) <= y; }
template <class T, class E> constexpr bool operator>(const Expected<T, E>& x, const UnexpectedType<E>& y) { return x > Expected<T, E>(y); }
template <class T, class E> constexpr bool operator>(const UnexpectedType<E>& x, const Expected<T, E>& y) { return Expected<T, E>(x) > y; }
template <class T, class E> constexpr bool operator>=(const Expected<T, E>& x, const UnexpectedType<E>& y) { return x >= Expected<T, E>(y); }
template <class T, class E> constexpr bool operator>=(const UnexpectedType<E>& x, const Expected<T, E>& y) { return Expected<T, E>(x) >= y; }

template <typename T, typename E> void swap(Expected<T, E>& x, Expected<T, E>& y) { x.swap(y); }

template <class T, class E = std::nullopt_t> constexpr Expected<std::decay_t<T>, E> makeExpected(T&& v)
{
    return Expected<typename std::decay<T>::type, E>(std::forward<T>(v));
}
template <class T, class E> constexpr Expected<T, std::decay_t<E>> makeExpectedFromError(E&& e) { return Expected<T, std::decay_t<E>>(makeUnexpected(e)); }
template <class T, class E, class U> constexpr Expected<T, E> makeExpectedFromError(U&& u) { return Expected<T, E>(makeUnexpected(E { std::forward<U>(u) } )); }
// template <class F, class E = std::nullopt_t> constexpr Expected<typename std::result_of<F>::type, E> makeExpected_from_call(F f);

inline Expected<void, std::nullopt_t> makeExpected() { return Expected<void, std::nullopt_t>(); }

} // namespace WTF

namespace std {

template <class T, class E> struct hash<WTF::Expected<T, E>> {
    typedef WTF::Expected<T, E> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& e) const { return e ? hash<typename argument_type::ValueType> { } (e.value()) : hash<typename argument_type::ErrorType> { } (e.error()); }
};

template <class E> struct hash<WTF::Expected<void, E>> {
    typedef WTF::Expected<void, E> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& e) const { return e ? 0 : hash<typename argument_type::ErrorType> { } (e.error()); }
};

}

using WTF::UnexpectedType;
using WTF::makeUnexpected;
using WTF::Unexpect;
using WTF::Expected;
using WTF::makeExpected;
using WTF::makeExpectedFromError;
