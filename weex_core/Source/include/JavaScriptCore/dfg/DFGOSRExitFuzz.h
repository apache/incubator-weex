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

#include "Options.h"

namespace JSC { namespace DFG {

extern unsigned g_numberOfStaticOSRExitFuzzChecks;

inline bool doOSRExitFuzzing()
{
    ASSERT(Options::useOSRExitFuzz());

    g_numberOfStaticOSRExitFuzzChecks++;
    if (unsigned atStatic = Options::fireOSRExitFuzzAtStatic())
        return atStatic == g_numberOfStaticOSRExitFuzzChecks;
    
    return true;
}

// DFG- and FTL-generated code will query this on every speculation.
extern unsigned g_numberOfOSRExitFuzzChecks;

} } // namespace JSC::DFG
