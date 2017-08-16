/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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
#include "CallVariant.h"

#include "JSCInlines.h"
#include <wtf/ListDump.h>

namespace JSC {

void CallVariant::dump(PrintStream& out) const
{
    if (!*this) {
        out.print("null");
        return;
    }
    
    if (InternalFunction* internalFunction = this->internalFunction()) {
        out.print("InternalFunction: ", JSValue(internalFunction));
        return;
    }
    
    if (JSFunction* function = this->function()) {
        out.print("(Function: ", JSValue(function), "; Executable: ", *executable(), ")");
        return;
    }
    
    out.print("Executable: ", *executable());
}

CallVariantList variantListWithVariant(const CallVariantList& list, CallVariant variantToAdd)
{
    ASSERT(variantToAdd);
    CallVariantList result;
    for (CallVariant variant : list) {
        ASSERT(variant);
        if (!!variantToAdd) {
            if (variant == variantToAdd)
                variantToAdd = CallVariant();
            else if (variant.despecifiedClosure() == variantToAdd.despecifiedClosure()) {
                variant = variant.despecifiedClosure();
                variantToAdd = CallVariant();
            }
        }
        result.append(variant);
    }
    if (!!variantToAdd)
        result.append(variantToAdd);
    
    if (!ASSERT_DISABLED) {
        for (unsigned i = 0; i < result.size(); ++i) {
            for (unsigned j = i + 1; j < result.size(); ++j) {
                if (result[i] != result[j])
                    continue;
                
                dataLog("variantListWithVariant(", listDump(list), ", ", variantToAdd, ") failed: got duplicates in result: ", listDump(result), "\n");
                RELEASE_ASSERT_NOT_REACHED();
            }
        }
    }
    
    return result;
}

CallVariantList despecifiedVariantList(const CallVariantList& list)
{
    CallVariantList result;
    for (CallVariant variant : list)
        result = variantListWithVariant(result, variant.despecifiedClosure());
    return result;
}

} // namespace JSC

