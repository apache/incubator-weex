/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#if ENABLE(FTL_JIT)

#include "FTLLocation.h"
#include "FTLSlowPathCallKey.h"
#include "MacroAssemblerCodeRef.h"
#include <wtf/HashMap.h>

namespace JSC {

class VM;

namespace FTL {

MacroAssemblerCodeRef osrExitGenerationThunkGenerator(VM*);
MacroAssemblerCodeRef lazySlowPathGenerationThunkGenerator(VM*);
MacroAssemblerCodeRef slowPathCallThunkGenerator(VM&, const SlowPathCallKey&);

template<typename KeyTypeArgument>
struct ThunkMap {
    typedef KeyTypeArgument KeyType;
    typedef HashMap<KeyType, MacroAssemblerCodeRef> ToThunkMap;
    typedef HashMap<MacroAssemblerCodePtr, KeyType> FromThunkMap;
    
    ToThunkMap m_toThunk;
    FromThunkMap m_fromThunk;
};

template<typename MapType, typename GeneratorType>
MacroAssemblerCodeRef generateIfNecessary(
    VM& vm, MapType& map, const typename MapType::KeyType& key, GeneratorType generator)
{
    typename MapType::ToThunkMap::iterator iter = map.m_toThunk.find(key);
    if (iter != map.m_toThunk.end())
        return iter->value;
    
    MacroAssemblerCodeRef result = generator(vm, key);
    map.m_toThunk.add(key, result);
    map.m_fromThunk.add(result.code(), key);
    return result;
}

template<typename MapType>
typename MapType::KeyType keyForThunk(MapType& map, MacroAssemblerCodePtr ptr)
{
    typename MapType::FromThunkMap::iterator iter = map.m_fromThunk.find(ptr);
    RELEASE_ASSERT(iter != map.m_fromThunk.end());
    return iter->value;
}

class Thunks {
public:
    MacroAssemblerCodeRef getSlowPathCallThunk(VM& vm, const SlowPathCallKey& key)
    {
        return generateIfNecessary(
            vm, m_slowPathCallThunks, key, slowPathCallThunkGenerator);
    }
    
    SlowPathCallKey keyForSlowPathCallThunk(MacroAssemblerCodePtr ptr)
    {
        return keyForThunk(m_slowPathCallThunks, ptr);
    }
    
private:
    ThunkMap<SlowPathCallKey> m_slowPathCallThunks;
};

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)
