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

namespace WTF {

// An abstract number of element in a sequence. The sequence has a first element.
// This type should be used instead of integer because 2 contradicting traditions can
// call a first element '0' or '1' which makes integer type ambiguous.
class OrdinalNumber {
public:
    static OrdinalNumber beforeFirst() { return OrdinalNumber(-1); }
    static OrdinalNumber fromZeroBasedInt(int zeroBasedInt) { return OrdinalNumber(zeroBasedInt); }
    static OrdinalNumber fromOneBasedInt(int oneBasedInt) { return OrdinalNumber(oneBasedInt - 1); }

    OrdinalNumber() : m_zeroBasedValue(0) { }

    int zeroBasedInt() const { return m_zeroBasedValue; }
    int oneBasedInt() const { return m_zeroBasedValue + 1; }

    bool operator==(OrdinalNumber other) { return m_zeroBasedValue == other.m_zeroBasedValue; }
    bool operator!=(OrdinalNumber other) { return !((*this) == other); }
    bool operator>(OrdinalNumber other) { return m_zeroBasedValue > other.m_zeroBasedValue; }

private:
    OrdinalNumber(int zeroBasedInt) : m_zeroBasedValue(zeroBasedInt) { }
    int m_zeroBasedValue;
};

}

using WTF::OrdinalNumber;
