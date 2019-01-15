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

#include "GenericOffset.h"
#include <wtf/PrintStream.h>

namespace JSC {

// This is an offset into the special arguments object, which captures the arguments to a
// function. It only comes into play if the arguments aren't also lifted into the activation.
// If they were then accesses to the arguments would resolve to a ScopeOffset and not a
// DirectArgumentsOffset.
class DirectArgumentsOffset : public GenericOffset<DirectArgumentsOffset> {
public:
    DirectArgumentsOffset() { }
    
    explicit DirectArgumentsOffset(unsigned offset)
        : GenericOffset(offset)
    {
    }
    
    void dump(PrintStream&) const;
};

} // namespace JSC
