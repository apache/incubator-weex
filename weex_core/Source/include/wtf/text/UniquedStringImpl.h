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
