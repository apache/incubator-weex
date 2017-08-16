/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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

#include "CellProfile.h"
#include <wtf/HashMap.h>
#include <wtf/SegmentedVector.h>

namespace JSC {

class CellList {
    WTF_MAKE_FAST_ALLOCATED;
public:
    CellList(const char* name)
        : m_name(name)
    {
    }
    
    const char* name() const { return m_name; }
    size_t size() const { return m_cells.size(); }

    typedef SegmentedVector<CellProfile, 64> CellProfileVector;
    CellProfileVector& cells() { return m_cells; }

    void add(CellProfile&& profile)
    {
        m_cells.append(WTFMove(profile));
        m_mapIsUpToDate = false;
    }

    void reset();

    CellProfile* find(HeapCell*);

private:
    const char* m_name;
    CellProfileVector m_cells;

    bool m_mapIsUpToDate { false };
    HashMap<HeapCell*, CellProfile*> m_map;
};
    
} // namespace JSC
