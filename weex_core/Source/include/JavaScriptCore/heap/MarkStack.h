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

#include "GCSegmentedArray.h"

namespace JSC {

class JSCell;

class MarkStackArray : public GCSegmentedArray<const JSCell*> {
public:
    MarkStackArray();

    void transferTo(MarkStackArray&);
    size_t transferTo(MarkStackArray&, size_t limit); // Optimized for when `limit` is small.
    void donateSomeCellsTo(MarkStackArray&);
    void stealSomeCellsFrom(MarkStackArray&, size_t idleThreadCount);
};

} // namespace JSC
