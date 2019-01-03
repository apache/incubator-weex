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
#ifndef WTF_IteratorRange_h
#define WTF_IteratorRange_h

namespace WTF {

template<typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
        : m_begin(WTFMove(begin))
        , m_end(WTFMove(end))
    {
    }

    Iterator begin() const { return m_begin; }
    Iterator end() const { return m_end; }

private:
    Iterator m_begin;
    Iterator m_end;
};

template<typename Iterator>
IteratorRange<Iterator> makeIteratorRange(Iterator&& begin, Iterator&& end)
{
    return IteratorRange<Iterator>(std::forward<Iterator>(begin), std::forward<Iterator>(end));
}

} // namespace WTF

#endif // WTF_IteratorRange_h
