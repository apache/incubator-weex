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

#include "JSObject.h"
#include "StackFrame.h"
#include <wtf/Vector.h>

namespace JSC {
    
class Exception : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = StructureIsImmortal | Base::StructureFlags;

    enum StackCaptureAction {
        CaptureStack,
        DoNotCaptureStack
    };
    JS_EXPORT_PRIVATE static Exception* create(VM&, JSValue thrownValue, StackCaptureAction = CaptureStack);

    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);

    static void visitChildren(JSCell*, SlotVisitor&);

    DECLARE_EXPORT_INFO;

    static ptrdiff_t valueOffset()
    {
        return OBJECT_OFFSETOF(Exception, m_value);
    }

    JSValue value() const { return m_value.get(); }
    const Vector<StackFrame>& stack() const { return m_stack; }

    bool didNotifyInspectorOfThrow() const { return m_didNotifyInspectorOfThrow; }
    void setDidNotifyInspectorOfThrow() { m_didNotifyInspectorOfThrow = true; }

    ~Exception();

private:
    Exception(VM&);
    void finishCreation(VM&, JSValue thrownValue, StackCaptureAction);

    WriteBarrier<Unknown> m_value;
    Vector<StackFrame> m_stack;
    bool m_didNotifyInspectorOfThrow { false };

    friend class LLIntOffsetsExtractor;
};

} // namespace JSC
