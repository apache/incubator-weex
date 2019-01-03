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

#include "SourceProviderCacheItem.h"
#include <wtf/HashMap.h>
#include <wtf/RefCounted.h>

namespace JSC {

class SourceProviderCache : public RefCounted<SourceProviderCache> {
    WTF_MAKE_FAST_ALLOCATED;
public:
    SourceProviderCache() { }
    JS_EXPORT_PRIVATE ~SourceProviderCache();

    JS_EXPORT_PRIVATE void clear();
    void add(int sourcePosition, std::unique_ptr<SourceProviderCacheItem>);
    const SourceProviderCacheItem* get(int sourcePosition) const { return m_map.get(sourcePosition); }

private:
    HashMap<int, std::unique_ptr<SourceProviderCacheItem>, WTF::IntHash<int>, WTF::UnsignedWithZeroKeyHashTraits<int>> m_map;
};

} // namespace JSC
