/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

#ifndef ScopedLambda_h
#define ScopedLambda_h

namespace WTF {

// You can use ScopedLambda to efficiently pass lambdas without allocating memory or requiring
// template specialization of the callee. The callee should be declared as:
//
// void foo(const ScopedLambda<MyThings* (int, Stuff&)>&);
//
// The caller just does:
//
// void foo(scopedLambda<MyThings* (int, Stuff&)>([&] (int x, Stuff& y) -> MyThings* { blah }));
//
// Note that this relies on foo() not escaping the lambda. The lambda is only valid while foo() is
// on the stack - hence the name ScopedLambda.

template<typename FunctionType> class ScopedLambda;
template<typename ResultType, typename... ArgumentTypes>
class ScopedLambda<ResultType (ArgumentTypes...)> {
public:
    ScopedLambda(ResultType (*impl)(void* arg, ArgumentTypes...) = nullptr, void* arg = nullptr)
        : m_impl(impl)
        , m_arg(arg)
    {
    }

    template<typename... PassedArgumentTypes>
    ResultType operator()(PassedArgumentTypes&&... arguments) const
    {
        return m_impl(m_arg, std::forward<PassedArgumentTypes>(arguments)...);
    }

private:
    ResultType (*m_impl)(void* arg, ArgumentTypes...);
    void *m_arg;
};

template<typename FunctionType, typename Functor> class ScopedLambdaFunctor;
template<typename ResultType, typename... ArgumentTypes, typename Functor>
class ScopedLambdaFunctor<ResultType (ArgumentTypes...), Functor> : public ScopedLambda<ResultType (ArgumentTypes...)> {
public:
    template<typename PassedFunctor>
    ScopedLambdaFunctor(PassedFunctor&& functor)
        : ScopedLambda<ResultType (ArgumentTypes...)>(implFunction, this)
        , m_functor(std::forward<PassedFunctor>(functor))
    {
    }
    
    // We need to make sure that copying and moving ScopedLambdaFunctor results in a ScopedLambdaFunctor
    // whose ScopedLambda supertype still points to this rather than other.
    ScopedLambdaFunctor(const ScopedLambdaFunctor& other)
        : ScopedLambda<ResultType (ArgumentTypes...)>(implFunction, this)
        , m_functor(other.m_functor)
    {
    }

    ScopedLambdaFunctor(ScopedLambdaFunctor&& other)
        : ScopedLambda<ResultType (ArgumentTypes...)>(implFunction, this)
        , m_functor(WTFMove(other.m_functor))
    {
    }
    
    ScopedLambdaFunctor& operator=(const ScopedLambdaFunctor& other)
    {
        m_functor = other.m_functor;
        return *this;
    }
    
    ScopedLambdaFunctor& operator=(ScopedLambdaFunctor&& other)
    {
        m_functor = WTFMove(other.m_functor);
        return *this;
    }

private:
    static ResultType implFunction(void* argument, ArgumentTypes... arguments)
    {
        return static_cast<ScopedLambdaFunctor*>(argument)->m_functor(arguments...);
    }

    Functor m_functor;
};

// Can't simply rely on perfect forwarding because then the ScopedLambdaFunctor would point to the functor
// by const reference. This would be surprising in situations like:
//
// auto scopedLambda = scopedLambda<Foo(Bar)>([&] (Bar) -> Foo { ... });
//
// We expected scopedLambda to be valid for its entire lifetime, but if it computed the lambda by reference
// then it would be immediately invalid.
template<typename FunctionType, typename Functor>
ScopedLambdaFunctor<FunctionType, Functor> scopedLambda(const Functor& functor)
{
    return ScopedLambdaFunctor<FunctionType, Functor>(functor);
}

template<typename FunctionType, typename Functor>
ScopedLambdaFunctor<FunctionType, Functor> scopedLambda(Functor&& functor)
{
    return ScopedLambdaFunctor<FunctionType, Functor>(WTFMove(functor));
}

template<typename FunctionType, typename Functor> class ScopedLambdaRefFunctor;
template<typename ResultType, typename... ArgumentTypes, typename Functor>
class ScopedLambdaRefFunctor<ResultType (ArgumentTypes...), Functor> : public ScopedLambda<ResultType (ArgumentTypes...)> {
public:
    ScopedLambdaRefFunctor(const Functor& functor)
        : ScopedLambda<ResultType (ArgumentTypes...)>(implFunction, this)
        , m_functor(&functor)
    {
    }
    
    // We need to make sure that copying and moving ScopedLambdaRefFunctor results in a
    // ScopedLambdaRefFunctor whose ScopedLambda supertype still points to this rather than
    // other.
    ScopedLambdaRefFunctor(const ScopedLambdaRefFunctor& other)
        : ScopedLambda<ResultType (ArgumentTypes...)>(implFunction, this)
        , m_functor(other.m_functor)
    {
    }

    ScopedLambdaRefFunctor(ScopedLambdaRefFunctor&& other)
        : ScopedLambda<ResultType (ArgumentTypes...)>(implFunction, this)
        , m_functor(other.m_functor)
    {
    }
    
    ScopedLambdaRefFunctor& operator=(const ScopedLambdaRefFunctor& other)
    {
        m_functor = other.m_functor;
        return *this;
    }
    
    ScopedLambdaRefFunctor& operator=(ScopedLambdaRefFunctor&& other)
    {
        m_functor = other.m_functor;
        return *this;
    }

private:
    static ResultType implFunction(void* argument, ArgumentTypes... arguments)
    {
        return (*static_cast<ScopedLambdaRefFunctor*>(argument)->m_functor)(arguments...);
    }

    const Functor* m_functor;
};

// This is for when you already refer to a functor by reference, and you know its lifetime is
// good. This just creates a ScopedLambda that points to your functor.
template<typename FunctionType, typename Functor>
ScopedLambdaRefFunctor<FunctionType, Functor> scopedLambdaRef(const Functor& functor)
{
    return ScopedLambdaRefFunctor<FunctionType, Functor>(functor);
}

} // namespace WTF

using WTF::ScopedLambda;
using WTF::scopedLambda;
using WTF::scopedLambdaRef;

#endif // ScopedLambda_h

