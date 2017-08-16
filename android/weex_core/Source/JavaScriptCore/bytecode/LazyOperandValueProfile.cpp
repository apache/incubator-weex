/*
 * Copyright (C) 2012, 2013 Apple Inc. All rights reserved.
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

#include "config.h"
#include "LazyOperandValueProfile.h"

#include "JSCInlines.h"

namespace JSC {

CompressedLazyOperandValueProfileHolder::CompressedLazyOperandValueProfileHolder() { }
CompressedLazyOperandValueProfileHolder::~CompressedLazyOperandValueProfileHolder() { }

void CompressedLazyOperandValueProfileHolder::computeUpdatedPredictions(const ConcurrentJSLocker& locker)
{
    if (!m_data)
        return;
    
    for (unsigned i = 0; i < m_data->size(); ++i)
        m_data->at(i).computeUpdatedPrediction(locker);
}

LazyOperandValueProfile* CompressedLazyOperandValueProfileHolder::add(
    const ConcurrentJSLocker&, const LazyOperandValueProfileKey& key)
{
    if (!m_data)
        m_data = std::make_unique<LazyOperandValueProfile::List>();
    else {
        for (unsigned i = 0; i < m_data->size(); ++i) {
            if (m_data->at(i).key() == key)
                return &m_data->at(i);
        }
    }
    
    m_data->append(LazyOperandValueProfile(key));
    return &m_data->last();
}

LazyOperandValueProfileParser::LazyOperandValueProfileParser() { }
LazyOperandValueProfileParser::~LazyOperandValueProfileParser() { }

void LazyOperandValueProfileParser::initialize(
    const ConcurrentJSLocker&, CompressedLazyOperandValueProfileHolder& holder)
{
    ASSERT(m_map.isEmpty());
    
    if (!holder.m_data)
        return;
    
    LazyOperandValueProfile::List& data = *holder.m_data;
    for (unsigned i = 0; i < data.size(); ++i)
        m_map.add(data[i].key(), &data[i]);
}

LazyOperandValueProfile* LazyOperandValueProfileParser::getIfPresent(
    const LazyOperandValueProfileKey& key) const
{
    HashMap<LazyOperandValueProfileKey, LazyOperandValueProfile*>::const_iterator iter =
        m_map.find(key);
    
    if (iter == m_map.end())
        return 0;
    
    return iter->value;
}

SpeculatedType LazyOperandValueProfileParser::prediction(
    const ConcurrentJSLocker& locker, const LazyOperandValueProfileKey& key) const
{
    LazyOperandValueProfile* profile = getIfPresent(key);
    if (!profile)
        return SpecNone;
    
    return profile->computeUpdatedPrediction(locker);
}

} // namespace JSC

