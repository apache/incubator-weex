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
#ifndef SimpleStats_h
#define SimpleStats_h

#include <wtf/MathExtras.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

// Simple and cheap way of tracking statistics if you're not worried about chopping on
// the sum of squares (i.e. the sum of squares is unlikely to exceed 2^52).
class SimpleStats {
public:
    SimpleStats()
        : m_count(0)
        , m_sum(0)
        , m_sumOfSquares(0)
    {
    }
    
    void add(double value)
    {
        m_count++;
        m_sum += value;
        m_sumOfSquares += value * value;
    }
    
    explicit operator bool() const
    {
        return !!m_count;
    }
    
    double count() const
    {
        return m_count;
    }
    
    double sum() const
    {
        return m_sum;
    }
    
    double sumOfSquares() const
    {
        return m_sumOfSquares;
    }
    
    double mean() const
    {
        return m_sum / m_count;
    }
    
    // NB. This gives a biased variance as it divides by the number of samples rather
    // than the degrees of freedom. This is fine once the count grows large, which in
    // our case will happen rather quickly.
    double variance() const
    {
        if (m_count < 2)
            return 0;
        
        // Compute <x^2> - <x>^2
        double secondMoment = m_sumOfSquares / m_count;
        double firstMoment = m_sum / m_count;
        
        double result = secondMoment - firstMoment * firstMoment;
        
        // It's possible to get -epsilon. Protect against this and turn it into
        // +0.
        if (result <= 0)
            return 0;
        
        return result;
    }
    
    // NB. This gives a biased standard deviation. See above.
    double standardDeviation() const
    {
        return sqrt(variance());
    }
    
private:
    double m_count;
    double m_sum;
    double m_sumOfSquares;
};

} // namespace WTF

using WTF::SimpleStats;

#endif // SimpleStats_h

