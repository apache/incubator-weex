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
#ifndef BagToHashMap_h
#define BagToHashMap_h

#include <wtf/Bag.h>
#include <wtf/DataLog.h>
#include <wtf/HashMap.h>

namespace WTF {

template<typename ElementType, typename KeyType, typename HashArg, typename KeyGetterFunctor>
void toHashMap(Bag<ElementType>& bag, KeyGetterFunctor& getKey, HashMap<KeyType, ElementType*, HashArg>& result)
{
    for (typename Bag<ElementType>::iterator iter = bag.begin(); !!iter; ++iter) {
        ElementType* element = *iter;
        KeyType key = getKey(*element);
        result.add(key, element);
    }
}

} // namespace WTF

using WTF::toHashMap;

#endif // BagToHashMap_h

