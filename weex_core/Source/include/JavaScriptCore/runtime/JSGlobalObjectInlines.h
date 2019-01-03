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
