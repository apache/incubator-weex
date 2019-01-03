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

#if ENABLE(JIT)

#include "DOMJITEffect.h"
#include "DOMJITPatchpoint.h"
#include "RegisterSet.h"

namespace JSC { namespace DOMJIT {

class CallDOMGetterPatchpoint : public Patchpoint {
public:
    static Ref<CallDOMGetterPatchpoint> create()
    {
        return adoptRef(*new CallDOMGetterPatchpoint());
    }

    // To look up DOMWrapper cache, GlobalObject is required.
    // FIXME: Later, we will extend this patchpoint to represent the result type by DOMJIT::Signature.
    // And after that, we will automatically pass a global object when the result type includes a DOM wrapper thing.
    // https://bugs.webkit.org/show_bug.cgi?id=162980
    bool requireGlobalObject { true };

    Effect effect { };

private:
    CallDOMGetterPatchpoint() = default;
};

} }

#endif
