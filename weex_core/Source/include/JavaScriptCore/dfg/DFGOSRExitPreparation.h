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

#if ENABLE(DFG_JIT)

#include "CallFrame.h"
#include "CodeOrigin.h"

namespace JSC { namespace DFG {

// Make sure all code on our inline stack is JIT compiled. This is necessary since
// we may opt to inline a code block even before it had ever been compiled by the
// JIT, but our OSR exit infrastructure currently only works if the target of the
// OSR exit is JIT code. This could be changed since there is nothing particularly
// hard about doing an OSR exit into the interpreter, but for now this seems to make
// sense in that if we're OSR exiting from inlined code of a DFG code block, then
// probably it's a good sign that the thing we're exiting into is hot. Even more
// interestingly, since the code was inlined, it may never otherwise get JIT
// compiled since the act of inlining it may ensure that it otherwise never runs.
void prepareCodeOriginForOSRExit(ExecState*, CodeOrigin);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
