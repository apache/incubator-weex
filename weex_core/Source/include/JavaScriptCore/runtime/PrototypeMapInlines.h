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

#include "PrototypeMap.h"
#include "WeakGCMapInlines.h"

namespace JSC {

inline TriState PrototypeMap::isPrototype(JSObject* object) const
{
    if (!m_prototypes.contains(object))
        return FalseTriState;

    // We know that 'object' was used as a prototype at one time, so be
    // conservative and say that it might still be so. (It would be expensive
    // to find out for sure, and we don't know of any cases where being precise
    // would improve performance.)
    return MixedTriState;
}

} // namespace JSC

