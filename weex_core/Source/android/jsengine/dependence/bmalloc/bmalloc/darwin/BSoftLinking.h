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
