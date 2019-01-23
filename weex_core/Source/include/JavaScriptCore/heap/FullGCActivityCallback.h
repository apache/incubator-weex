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

#include "GCActivityCallback.h"

namespace JSC {

class JS_EXPORT_PRIVATE FullGCActivityCallback : public GCActivityCallback {
public:
    FullGCActivityCallback(Heap*);

    void doCollection() override;

    bool didSyncGCRecently() const { return m_didSyncGCRecently; }
    void setDidSyncGCRecently() { m_didSyncGCRecently = true; }

protected:
    double lastGCLength() override;
    double gcTimeSlice(size_t bytes) override;
    double deathRate() override;

    bool m_didSyncGCRecently { false };
};

inline RefPtr<FullGCActivityCallback> GCActivityCallback::createFullTimer(Heap* heap)
{
    return s_shouldCreateGCTimer ? adoptRef(new FullGCActivityCallback(heap)) : nullptr;
}

} // namespace JSC
