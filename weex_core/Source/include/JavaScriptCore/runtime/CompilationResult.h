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

#include <wtf/PrintStream.h>

namespace JSC {

enum CompilationResult {
    // We tried to compile the code, but we couldn't compile it. This could be
    // because we ran out of memory, or because the compiler encountered an
    // internal error and decided to bail out gracefully. Either way, this implies
    // that we shouldn't try to compile this code block again.
    CompilationFailed,
    
    // The profiling assumptions that were fed into the compiler were invalidated
    // even before we finished compiling. This means we should try again: in such
    // cases the profiling will now be updated and the next compilation won't
    // encounter the same problem. But it does mean that we should exercise
    // exponential back-off, to get even more profiling so that new profiling
    // pathologies aren't encountered.
    CompilationInvalidated,
    
    // The compilation succeeded and the code block now has JITCode for the newly
    // compiled code. However, compilation success doesn't mean that the CodeBlock
    // will execute yet; you typically have to install it first, unless you plan
    // on invoking it manually (something that *could* be done for some kinds of
    // OSR entry).
    CompilationSuccessful,
    
    // We decided to do the compilation asynchronously. This means that we haven't
    // yet compiled the code. This only happens when you pass a
    // DeferredCompilationCallback. That callback will get called with some
    // interesting result, once compilation completes.
    CompilationDeferred
};

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, JSC::CompilationResult);

} // namespace WTF
