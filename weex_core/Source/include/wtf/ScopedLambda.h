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

