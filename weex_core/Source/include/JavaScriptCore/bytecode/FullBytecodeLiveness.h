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

#include <wtf/FastBitVector.h>

namespace JSC {

class BytecodeLivenessAnalysis;

typedef HashMap<unsigned, FastBitVector, WTF::IntHash<unsigned>, WTF::UnsignedWithZeroKeyHashTraits<unsigned>> BytecodeToBitmapMap;

class FullBytecodeLiveness {
    WTF_MAKE_FAST_ALLOCATED;
public:
    const FastBitVector& getLiveness(unsigned bytecodeIndex) const
    {
        return m_map[bytecodeIndex];
    }
    
    bool operandIsLive(int operand, unsigned bytecodeIndex) const
    {
        return operandIsAlwaysLive(operand) || operandThatIsNotAlwaysLiveIsLive(getLiveness(bytecodeIndex), operand);
    }
    
private:
    friend class BytecodeLivenessAnalysis;
    
    Vector<FastBitVector, 0, UnsafeVectorOverflow> m_map;
};

} // namespace JSC
