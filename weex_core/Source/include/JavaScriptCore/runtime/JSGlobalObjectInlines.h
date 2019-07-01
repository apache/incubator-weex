/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "JSGlobalObject.h"

#include "ArrayPrototype.h"
#include "ObjectPrototype.h"

namespace JSC {

ALWAYS_INLINE bool JSGlobalObject::objectPrototypeIsSane()
{
    return !hasIndexedProperties(m_objectPrototype->indexingType())
        && m_objectPrototype->getPrototypeDirect().isNull();
}

ALWAYS_INLINE bool JSGlobalObject::arrayPrototypeChainIsSane()
{
    return !hasIndexedProperties(m_arrayPrototype->indexingType())
        && m_arrayPrototype->getPrototypeDirect() == m_objectPrototype.get()
        && objectPrototypeIsSane();
}

ALWAYS_INLINE bool JSGlobalObject::stringPrototypeChainIsSane()
{
    return !hasIndexedProperties(m_stringPrototype->indexingType())
        && m_stringPrototype->getPrototypeDirect() == m_objectPrototype.get()
        && objectPrototypeIsSane();
}


ALWAYS_INLINE bool JSGlobalObject::isArrayIteratorProtocolFastAndNonObservable()
{
    // We're fast if we don't have any indexed properties on the prototype.
    // We're non-observable if the iteration protocol hasn't changed.
    //
    // Note: it only makes sense to call this from the main thread. If you're
    // trying to prove this behavior on the compiler thread, you'll want to
    // carefully set up watchpoints to have correct ordering while JS code is
    // executing concurrently.

    return arrayIteratorProtocolWatchpoint().isStillValid() && !isHavingABadTime() && arrayPrototypeChainIsSane();
}

} // namespace JSC
