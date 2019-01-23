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
