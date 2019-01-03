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
#ifndef FunctionDispatcher_h
#define FunctionDispatcher_h

#include <functional>
#include <wtf/Function.h>
#include <wtf/ThreadSafeRefCounted.h>

namespace WTF {

// FunctionDispatcher is an abstract representation of something that functions can be
// dispatched to. This can for example be a run loop or a work queue.

class FunctionDispatcher : public ThreadSafeRefCounted<FunctionDispatcher> {
public:
    WTF_EXPORT_PRIVATE virtual ~FunctionDispatcher();

    virtual void dispatch(Function<void ()>&&) = 0;

protected:
    WTF_EXPORT_PRIVATE FunctionDispatcher();
};

} // namespace WTF

using WTF::FunctionDispatcher;

#endif // FunctionDispatcher_h
