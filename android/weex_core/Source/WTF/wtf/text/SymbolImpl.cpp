/*
 * Copyright (C) 2016 Yusuke Suzuki <utatane.tea@gmail.com>.
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

#include "config.h"
#include "SymbolImpl.h"

namespace WTF {

// In addition to the normal hash value, store specialized hash value for
// symbolized StringImpl*. And don't use the normal hash value for symbolized
// StringImpl* when they are treated as Identifiers. Unique nature of these
// symbolized StringImpl* keys means that we don't need them to match any other
// string (in fact, that's exactly the oposite of what we want!), and the
// normal hash would lead to lots of conflicts.
unsigned SymbolImpl::nextHashForSymbol()
{
    static unsigned s_nextHashForSymbol = 0;
    s_nextHashForSymbol += 1 << s_flagCount;
    s_nextHashForSymbol |= 1 << 31;
    return s_nextHashForSymbol;
}

Ref<SymbolImpl> SymbolImpl::create(StringImpl& rep)
{
    auto* ownerRep = (rep.bufferOwnership() == BufferSubstring) ? rep.substringBuffer() : &rep;
    ASSERT(ownerRep->bufferOwnership() != BufferSubstring);
    if (rep.is8Bit())
        return adoptRef(*new SymbolImpl(rep.m_data8, rep.length(), *ownerRep));
    return adoptRef(*new SymbolImpl(rep.m_data16, rep.length(), *ownerRep));
}

Ref<SymbolImpl> SymbolImpl::createNullSymbol()
{
    return adoptRef(*new SymbolImpl);
}

} // namespace WTF
