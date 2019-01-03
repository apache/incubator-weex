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

#include "JSCell.h"
#include "Watchpoint.h"
#include "WriteBarrier.h"

namespace JSC {

// Allocate one of these if you'd like to infer a constant value. Writes to the value should use
// notifyWrite(). So long as exactly one value had ever been written and invalidate() has never been
// called, and you register a watchpoint, you can rely on the inferredValue() being the one true
// value.
//
// Commonly used for inferring singletons - in that case each allocation does notifyWrite(). But you
// can use it for other things as well.

class InferredValue : public JSCell {
public:
    typedef JSCell Base;
    
    static InferredValue* create(VM&);
    
    static const bool needsDestruction = true;
    static void destroy(JSCell*);
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);
    
    static void visitChildren(JSCell*, SlotVisitor&);
    
    DECLARE_INFO;
    
    // For the purpose of deciding whether or not to watch this variable, you only need
    // to inspect inferredValue(). If this returns something other than the empty
    // value, then it means that at all future safepoints, this watchpoint set will be
    // in one of these states:
    //
    //    IsWatched: in this case, the variable's value must still be the
    //        inferredValue.
    //
    //    IsInvalidated: in this case the variable's value may be anything but you'll
    //        either notice that it's invalidated and not install the watchpoint, or
    //        you will have been notified that the watchpoint was fired.
    JSValue inferredValue() { return m_value.get(); }

    // Forwards some WatchpointSet methods.
    WatchpointState state() const { return m_set.state(); }
    bool isStillValid() const { return m_set.isStillValid(); }
    bool hasBeenInvalidated() const { return m_set.hasBeenInvalidated(); }
    void add(Watchpoint* watchpoint) { m_set.add(watchpoint); }
    
    void notifyWrite(VM& vm, JSValue value, const FireDetail& detail)
    {
        if (LIKELY(m_set.stateOnJSThread() == IsInvalidated))
            return;
        notifyWriteSlow(vm, value, detail);
    }
    
    void notifyWrite(VM& vm, JSValue value, const char* reason)
    {
        if (LIKELY(m_set.stateOnJSThread() == IsInvalidated))
            return;
        notifyWriteSlow(vm, value, reason);
    }
    
    void invalidate(VM& vm, const FireDetail& detail)
    {
        m_value.clear();
        m_set.invalidate(vm, detail);
    }
    
    static const unsigned StructureFlags = StructureIsImmortal | Base::StructureFlags;
    
    // We could have used Weak<>. But we want arbitrary JSValues, not just cells. It's also somewhat
    // convenient to have eager notification of death.
    //
    // Also note that this should be a private class, but it isn't because Windows.
    class ValueCleanup : public UnconditionalFinalizer {
        WTF_MAKE_FAST_ALLOCATED;
        
    public:
        ValueCleanup(InferredValue*);
        virtual ~ValueCleanup();
        
    protected:
        void finalizeUnconditionally() override;
        
    private:
        InferredValue* m_owner;
    };
    
private:
    InferredValue(VM&);
    ~InferredValue();
    
    JS_EXPORT_PRIVATE void notifyWriteSlow(VM&, JSValue, const FireDetail&);
    JS_EXPORT_PRIVATE void notifyWriteSlow(VM&, JSValue, const char* reason);
    
    friend class ValueCleanup;
    
    InlineWatchpointSet m_set;
    WriteBarrier<Unknown> m_value;
    std::unique_ptr<ValueCleanup> m_cleanup;
};

// FIXME: We could have an InlineInferredValue, which only allocates the InferredValue object when
// a notifyWrite() transitions us towards watching, and then clears the reference (allowing the object
// to die) when we get invalidated.

} // namespace JSC
