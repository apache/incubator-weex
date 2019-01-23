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
#ifndef AtomicStringHash_h
#define AtomicStringHash_h

#include <wtf/text/AtomicString.h>
#include <wtf/HashTraits.h>

namespace WTF {

    struct AtomicStringHash {
        static unsigned hash(const AtomicString& key)
        {
            return key.impl()->existingHash();
        }

        static bool equal(const AtomicString& a, const AtomicString& b)
        {
            return a == b;
        }

        static const bool safeToCompareToEmptyOrDeleted = false;
    };

    template<> struct HashTraits<WTF::AtomicString> : SimpleClassHashTraits<WTF::AtomicString> {
        static const bool hasIsEmptyValueFunction = true;
        static bool isEmptyValue(const AtomicString& value)
        {
            return value.isNull();
        }

        static void customDeleteBucket(AtomicString& value)
        {
            // See unique_ptr's customDeleteBucket() for an explanation.
            ASSERT(!isDeletedValue(value));
            AtomicString valueToBeDestroyed = WTFMove(value);
            constructDeletedValue(value);
        }
    };

}

using WTF::AtomicStringHash;

#endif
