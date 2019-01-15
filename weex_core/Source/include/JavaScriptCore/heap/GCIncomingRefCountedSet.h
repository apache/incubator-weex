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

#include "GCIncomingRefCounted.h"

namespace JSC {

// T = some subtype of GCIncomingRefCounted, must support a gcSizeEstimateInBytes()
// method.
template<typename T>
class GCIncomingRefCountedSet {
public:
    GCIncomingRefCountedSet();

    void lastChanceToFinalize();

    // Returns true if the native object is new to this set.
    bool addReference(JSCell*, T*);
    
    void sweep();
    
    size_t size() const { return m_bytes; };
    
private:
    static bool removeAll(JSCell*);
    static bool removeDead(JSCell*);
    
    Vector<T*> m_vector;
    size_t m_bytes;
};

} // namespace JSC
