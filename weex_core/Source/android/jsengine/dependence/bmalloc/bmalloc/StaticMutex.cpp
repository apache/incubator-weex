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
#include "ScopeExit.h"
#include "StaticMutex.h"
#include <thread>

namespace bmalloc {

void StaticMutex::lockSlowCase()
{
    // The longest critical section in bmalloc is much shorter than the
    // time it takes to make a system call to yield to the OS scheduler.
    // So, we try again a lot before we yield.
    static const size_t aLot = 256;
    
    if (!m_isSpinning.test_and_set()) {
        auto clear = makeScopeExit([&] { m_isSpinning.clear(); });

        for (size_t i = 0; i < aLot; ++i) {
            if (try_lock())
                return;
        }
    }

    // Avoid spinning pathologically.
    while (!try_lock())
        std::this_thread::yield();
}

} // namespace bmalloc
