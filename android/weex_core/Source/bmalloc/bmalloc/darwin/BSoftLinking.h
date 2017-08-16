/*
 * Copyright (C) 2007, 2008, 2011-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BSoftLinking_h
#define BSoftLinking_h

#include "BAssert.h"
#include <dlfcn.h>
#include <mutex>

#define BSOFT_LINK_PRIVATE_FRAMEWORK(framework) \
    static void* framework##Library() \
    { \
        static void* frameworkLibrary; \
        static std::once_flag once; \
        std::call_once(once, [] { \
            frameworkLibrary = dlopen("/System/Library/PrivateFrameworks/" #framework ".framework/" #framework, RTLD_NOW); \
            RELEASE_BASSERT_WITH_MESSAGE(frameworkLibrary, "%s", dlerror()); \
        }); \
        return frameworkLibrary; \
    }

#define BSOFT_LINK_FUNCTION(framework, functionName, resultType, parameterDeclarations, parameterNames) \
    extern "C" { \
    resultType functionName parameterDeclarations; \
    } \
    static resultType init##functionName parameterDeclarations; \
    static resultType (*softLink##functionName) parameterDeclarations = init##functionName; \
    \
    static resultType init##functionName parameterDeclarations \
    { \
        static std::once_flag once; \
        std::call_once(once, [] { \
            softLink##functionName = (resultType (*) parameterDeclarations) dlsym(framework##Library(), #functionName); \
            RELEASE_BASSERT_WITH_MESSAGE(softLink##functionName, "%s", dlerror()); \
        }); \
        return softLink##functionName parameterNames; \
    } \
    \
    inline resultType functionName parameterDeclarations \
    { \
        return softLink##functionName parameterNames; \
    }

#endif // BSoftLinking_h
