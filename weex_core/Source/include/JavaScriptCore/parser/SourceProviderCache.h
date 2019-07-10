/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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
