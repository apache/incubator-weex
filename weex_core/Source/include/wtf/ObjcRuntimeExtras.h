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
#ifndef WTF_ObjcRuntimeExtras_h
#define WTF_ObjcRuntimeExtras_h

// FIXME: This file's name and function names refer to Objective-C as Objc with a lowercase C,
// but it should be ObjC with an uppercase C.

#include <objc/message.h>

#ifdef __cplusplus

template<typename ReturnType, typename... ArgumentTypes>
inline ReturnType wtfObjcMsgSend(id target, SEL selector, ArgumentTypes... arguments)
{
    return reinterpret_cast<ReturnType (*)(id, SEL, ArgumentTypes...)>(objc_msgSend)(target, selector, arguments...);
}

template<typename ReturnType, typename... ArgumentTypes>
inline ReturnType wtfCallIMP(IMP implementation, id target, SEL selector, ArgumentTypes... arguments)
{
    return reinterpret_cast<ReturnType (*)(id, SEL, ArgumentTypes...)>(implementation)(target, selector, arguments...);
}

#endif // __cplusplus

#endif // WTF_ObjcRuntimeExtras_h
