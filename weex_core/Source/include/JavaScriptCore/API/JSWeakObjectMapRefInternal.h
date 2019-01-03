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
#ifndef JSWeakObjectMapRefInternal_h
#define JSWeakObjectMapRefInternal_h

#include "WeakGCMap.h"
#include <wtf/RefCounted.h>

namespace JSC {

class JSObject;

}

typedef void (*JSWeakMapDestroyedCallback)(struct OpaqueJSWeakObjectMap*, void*);

typedef JSC::WeakGCMap<void*, JSC::JSObject> WeakMapType;

struct OpaqueJSWeakObjectMap : public RefCounted<OpaqueJSWeakObjectMap> {
public:
    static Ref<OpaqueJSWeakObjectMap> create(JSC::VM& vm, void* data, JSWeakMapDestroyedCallback callback)
    {
        return adoptRef(*new OpaqueJSWeakObjectMap(vm, data, callback));
    }

    WeakMapType& map() { return m_map; }
    
    ~OpaqueJSWeakObjectMap()
    {
        m_callback(this, m_data);
    }

private:
    OpaqueJSWeakObjectMap(JSC::VM& vm, void* data, JSWeakMapDestroyedCallback callback)
        : m_map(vm)
        , m_data(data)
        , m_callback(callback)
    {
    }
    WeakMapType m_map;
    void* m_data;
    JSWeakMapDestroyedCallback m_callback;
};


#endif // JSWeakObjectMapInternal_h
