/*
 * Copyright (C) 2012, 2013 Apple Inc. All rights reserved.
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
