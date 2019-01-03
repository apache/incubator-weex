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

#include <wtf/Vector.h>

namespace JSC {

class HeapSnapshot;
class HeapSnapshotBuilder;
class VM;

class HeapProfiler {
    WTF_MAKE_FAST_ALLOCATED;
public:
    HeapProfiler(VM&);
    ~HeapProfiler();

    VM& vm() const { return m_vm; }

    HeapSnapshot* mostRecentSnapshot();
    void appendSnapshot(std::unique_ptr<HeapSnapshot>);
    void clearSnapshots();

    HeapSnapshotBuilder* activeSnapshotBuilder() const { return m_activeBuilder; }
    void setActiveSnapshotBuilder(HeapSnapshotBuilder*);

private:
    VM& m_vm;
    Vector<std::unique_ptr<HeapSnapshot>> m_snapshots;
    HeapSnapshotBuilder* m_activeBuilder { nullptr };
};

} // namespace JSC
