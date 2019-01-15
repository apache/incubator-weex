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
#ifndef RefCountedLeakCounter_h
#define RefCountedLeakCounter_h

#include <atomic>
#include <wtf/Assertions.h>

namespace WTF {
    
struct RefCountedLeakCounter {
    WTF_EXPORT_PRIVATE static void suppressMessages(const char*);
    WTF_EXPORT_PRIVATE static void cancelMessageSuppression(const char*);
    
    WTF_EXPORT_PRIVATE explicit RefCountedLeakCounter(const char* description);
    WTF_EXPORT_PRIVATE ~RefCountedLeakCounter();

    WTF_EXPORT_PRIVATE void increment();
    WTF_EXPORT_PRIVATE void decrement();

#ifndef NDEBUG
private:
    std::atomic<int> m_count;
    const char* m_description;
#endif
};

}  // namespace WTF

#endif
