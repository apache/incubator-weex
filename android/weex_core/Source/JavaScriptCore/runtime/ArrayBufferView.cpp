/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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
#include "ArrayBufferView.h"

namespace JSC {

ArrayBufferView::ArrayBufferView(
    RefPtr<ArrayBuffer>&& buffer,
    unsigned byteOffset)
        : m_byteOffset(byteOffset)
        , m_isNeuterable(true)
        , m_buffer(WTFMove(buffer))
{
    m_baseAddress = m_buffer ? (static_cast<char*>(m_buffer->data()) + m_byteOffset) : 0;
}

ArrayBufferView::~ArrayBufferView()
{
    if (!m_isNeuterable)
        m_buffer->unpin();
}

void ArrayBufferView::setNeuterable(bool flag)
{
    if (flag == m_isNeuterable)
        return;
    
    m_isNeuterable = flag;
    
    if (!m_buffer)
        return;
    
    if (flag)
        m_buffer->unpin();
    else
        m_buffer->pin();
}

} // namespace JSC
