/*
 * Copyright (C) 2013-2017 Apple Inc. All Rights Reserved.
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

#pragma once

#include "Register.h"
#include <wtf/ForbidHeapAllocation.h>

namespace JSC {

struct JS_EXPORT_PRIVATE ProtoCallFrame {
    WTF_FORBID_HEAP_ALLOCATION;
public:
    Register codeBlockValue;
    Register calleeValue;
    Register argCountAndCodeOriginValue;
    Register thisArg;
    uint32_t paddedArgCount;
    bool arityMissMatch;
    JSValue *args;

    void init(CodeBlock*, JSObject*, JSValue, int, JSValue* otherArgs = 0);

    CodeBlock* codeBlock() const { return codeBlockValue.Register::codeBlock(); }
    void setCodeBlock(CodeBlock* codeBlock) { codeBlockValue = codeBlock; }

    JSObject* callee() const { return calleeValue.Register::object(); }
    void setCallee(JSObject* callee) { calleeValue = callee; }

    int argumentCountIncludingThis() const { return argCountAndCodeOriginValue.payload(); }
    int argumentCount() const { return argumentCountIncludingThis() - 1; }
    void setArgumentCountIncludingThis(int count) { argCountAndCodeOriginValue.payload() = count; }
    void setPaddedArgCount(uint32_t argCount) { paddedArgCount = argCount; }

    void clearCurrentVPC() { argCountAndCodeOriginValue.tag() = 0; }
    
    JSValue thisValue() const { return thisArg.Register::jsValue(); }
    void setThisValue(JSValue value) { thisArg = value; }

    bool needArityCheck() { return arityMissMatch; }

    JSValue argument(size_t argumentIndex)
    {
        ASSERT(static_cast<int>(argumentIndex) < argumentCount());
        return args[argumentIndex];
    }
    void setArgument(size_t argumentIndex, JSValue value)
    {
        ASSERT(static_cast<int>(argumentIndex) < argumentCount());
        args[argumentIndex] = value;
    }
};

} // namespace JSC
