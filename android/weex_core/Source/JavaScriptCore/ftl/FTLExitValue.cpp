/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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

#include "config.h"
#include "FTLExitValue.h"

#if ENABLE(FTL_JIT)

#include "FTLExitTimeObjectMaterialization.h"
#include "JSCInlines.h"
#include "TrackedReferences.h"

namespace JSC { namespace FTL {

ExitValue ExitValue::materializeNewObject(ExitTimeObjectMaterialization* data)
{
    ExitValue result;
    result.m_kind = ExitValueMaterializeNewObject;
    result.u.newObjectMaterializationData = data;
    return result;
}

ExitValue ExitValue::withLocalsOffset(int offset) const
{
    if (!isInJSStackSomehow())
        return *this;
    if (!virtualRegister().isLocal())
        return *this;
    return withVirtualRegister(virtualRegister() + offset);
}

DataFormat ExitValue::dataFormat() const
{
    switch (kind()) {
    case InvalidExitValue:
        RELEASE_ASSERT_NOT_REACHED();
        return DataFormatNone;
            
    case ExitValueDead:
    case ExitValueConstant:
    case ExitValueInJSStack:
    case ExitValueMaterializeNewObject:
        return DataFormatJS;
            
    case ExitValueArgument:
        return exitArgument().format();
            
    case ExitValueInJSStackAsInt32:
        return DataFormatInt32;
            
    case ExitValueInJSStackAsInt52:
        return DataFormatInt52;
            
    case ExitValueInJSStackAsDouble:
        return DataFormatDouble;
            
    case ExitValueRecovery:
        return recoveryFormat();
    }
        
    RELEASE_ASSERT_NOT_REACHED();
}

void ExitValue::dumpInContext(PrintStream& out, DumpContext* context) const
{
    switch (kind()) {
    case InvalidExitValue:
        out.print("Invalid");
        return;
    case ExitValueDead:
        out.print("Dead");
        return;
    case ExitValueArgument:
        out.print("Argument(", exitArgument(), ")");
        return;
    case ExitValueConstant:
        out.print("Constant(", inContext(constant(), context), ")");
        return;
    case ExitValueInJSStack:
        out.print("InJSStack:", virtualRegister());
        return;
    case ExitValueInJSStackAsInt32:
        out.print("InJSStackAsInt32:", virtualRegister());
        return;
    case ExitValueInJSStackAsInt52:
        out.print("InJSStackAsInt52:", virtualRegister());
        return;
    case ExitValueInJSStackAsDouble:
        out.print("InJSStackAsDouble:", virtualRegister());
        return;
    case ExitValueRecovery:
        out.print("Recovery(", recoveryOpcode(), ", arg", leftRecoveryArgument(), ", arg", rightRecoveryArgument(), ", ", recoveryFormat(), ")");
        return;
    case ExitValueMaterializeNewObject:
        out.print("Materialize(", WTF::RawPointer(objectMaterialization()), ")");
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

void ExitValue::dump(PrintStream& out) const
{
    dumpInContext(out, 0);
}

void ExitValue::validateReferences(const TrackedReferences& trackedReferences) const
{
    if (isConstant())
        trackedReferences.check(constant());
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

