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

// This is guarded by ENABLE_DFG_JIT only because it uses some value profiles
// that are currently only used if the DFG is enabled (i.e. they are not
// available in the profile-only configuration). Hopefully someday all of
// these #if's will disappear...
#if ENABLE(DFG_JIT)

#include "GPRInfo.h"
#include "JSCJSValue.h"

namespace JSC {

class CCallHelpers;
class CodeBlock;
class LazyOperandValueProfileKey;
struct ArithProfile;
struct ValueProfile;

class MethodOfGettingAValueProfile {
public:
    MethodOfGettingAValueProfile()
        : m_kind(None)
    {
    }
    
    MethodOfGettingAValueProfile(ValueProfile* profile)
    {
        if (profile) {
            m_kind = Ready;
            u.profile = profile;
        } else
            m_kind = None;
    }
    
    MethodOfGettingAValueProfile(ArithProfile* profile)
    {
        if (profile) {
            m_kind = ArithProfileReady;
            u.arithProfile = profile;
        } else
            m_kind = None;
    }
    
    static MethodOfGettingAValueProfile fromLazyOperand(
        CodeBlock*, const LazyOperandValueProfileKey&);
    
    explicit operator bool() const { return m_kind != None; }
    
    void emitReportValue(CCallHelpers&, JSValueRegs) const;
    
private:
    enum Kind {
        None,
        Ready,
        ArithProfileReady,
        LazyOperand
    };
    
    Kind m_kind;
    union {
        ValueProfile* profile;
        ArithProfile* arithProfile;
        struct {
            CodeBlock* codeBlock;
            unsigned bytecodeOffset;
            int operand;
        } lazyOperand;
    } u;
};

} // namespace JSC

#endif // ENABLE(DFG_JIT)
