/*
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
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

#ifndef UniquedStringImpl_h
#define UniquedStringImpl_h

#include <wtf/text/StringImpl.h>

namespace WTF {

// It represents that the string impl is uniqued in some ways.
// When the given 2 string impls are both uniqued string impls, we can compare it just using pointer comparison.
class UniquedStringImpl : public StringImpl {
private:
    UniquedStringImpl() = delete;
protected:
    UniquedStringImpl(CreateSymbolTag, const LChar* characters, unsigned length) : StringImpl(CreateSymbol, characters, length) { }
    UniquedStringImpl(CreateSymbolTag, const UChar* characters, unsigned length) : StringImpl(CreateSymbol, characters, length) { }
    UniquedStringImpl(CreateSymbolTag) : StringImpl(CreateSymbol) { }
};

#if !ASSERT_DISABLED
// UniquedStringImpls created from StaticStringImpl will ASSERT
// in the generic ValueCheck<T>::checkConsistency
// as they are not allocated by fastMalloc.
// We don't currently have any way to detect that case
// so we ignore the consistency check for all UniquedStringImpls*.
template<> struct
ValueCheck<UniquedStringImpl*> {
    static void checkConsistency(const UniquedStringImpl*) { }
};

template<> struct
ValueCheck<const UniquedStringImpl*> {
    static void checkConsistency(const UniquedStringImpl*) { }
};
#endif

} // namespace WTF

using WTF::UniquedStringImpl;

#endif // UniquedStringImpl_h
