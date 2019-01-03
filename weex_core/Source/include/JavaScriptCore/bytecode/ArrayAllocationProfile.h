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

#include "IndexingType.h"
#include "JSArray.h"

namespace JSC {

class ArrayAllocationProfile {
public:
    ArrayAllocationProfile()
        : m_currentIndexingType(ArrayWithUndecided)
        , m_lastArray(0)
    {
    }
    
    IndexingType selectIndexingType()
    {
        JSArray* lastArray = m_lastArray;
        if (lastArray && UNLIKELY(lastArray->indexingType() != m_currentIndexingType))
            updateIndexingType();
        return m_currentIndexingType;
    }
    
    JSArray* updateLastAllocation(JSArray* lastArray)
    {
        m_lastArray = lastArray;
        return lastArray;
    }
    
    JS_EXPORT_PRIVATE void updateIndexingType();
    
    static IndexingType selectIndexingTypeFor(ArrayAllocationProfile* profile)
    {
        if (!profile)
            return ArrayWithUndecided;
        return profile->selectIndexingType();
    }
    
    static JSArray* updateLastAllocationFor(ArrayAllocationProfile* profile, JSArray* lastArray)
    {
        if (profile)
            profile->updateLastAllocation(lastArray);
        return lastArray;
    }

private:
    
    IndexingType m_currentIndexingType;
    JSArray* m_lastArray;
};

} // namespace JSC
