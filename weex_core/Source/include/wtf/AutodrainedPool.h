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
#ifndef AutodrainedPool_h
#define AutodrainedPool_h

#include <wtf/Noncopyable.h>

#if USE(FOUNDATION) && !defined(__OBJC__)
typedef struct objc_object *id;
#endif

namespace WTF {

class AutodrainedPool {
    WTF_MAKE_NONCOPYABLE(AutodrainedPool);
public:
#if USE(FOUNDATION)
    WTF_EXPORT_PRIVATE AutodrainedPool();
    WTF_EXPORT_PRIVATE ~AutodrainedPool();
#else
    explicit AutodrainedPool() { }
    ~AutodrainedPool() { }
#endif
    
private:
#if USE(FOUNDATION)
    id m_pool;
#endif
};

} // namespace WTF

using WTF::AutodrainedPool;

#endif
