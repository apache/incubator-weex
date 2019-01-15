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
#ifndef ThreadIdentifierDataPthreads_h
#define ThreadIdentifierDataPthreads_h

#include <wtf/Threading.h>

namespace WTF {

// Holds ThreadIdentifier in the thread-specific storage and employs pthreads-specific 2-pass destruction to reliably remove
// ThreadIdentifier from threadMap. It assumes regular ThreadSpecific types don't use multiple-pass destruction.
class ThreadIdentifierData {
    WTF_MAKE_NONCOPYABLE(ThreadIdentifierData);
public:
    ~ThreadIdentifierData();

    // One time initialization for this class as a whole.
    // This method must be called before initialize() and it is not thread-safe.
    static void initializeOnce();

    // Creates and puts an instance of ThreadIdentifierData into thread-specific storage.
    static void initialize(ThreadIdentifier identifier);

    // Returns 0 if thread-specific storage was not initialized.
    static ThreadIdentifier identifier();

private:
    ThreadIdentifierData(ThreadIdentifier identifier)
        : m_identifier(identifier)
        , m_isDestroyedOnce(false)
    {
    }

    // This thread-specific destructor is called 2 times when thread terminates:
    // - first, when all the other thread-specific destructors are called, it simply remembers it was 'destroyed once'
    // and re-sets itself into the thread-specific slot to make Pthreads to call it again later.
    // - second, after all thread-specific destructors were invoked, it gets called again - this time, we remove the
    // ThreadIdentifier from the threadMap, completing the cleanup.
    static void destruct(void* data);

    ThreadIdentifier m_identifier;
    bool m_isDestroyedOnce;
    static pthread_key_t m_key;
};

} // namespace WTF

#endif // ThreadIdentifierDataPthreads_h


