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

namespace JSC {

inline bool isARMv7IDIVSupported()
{
#if HAVE(ARM_IDIV_INSTRUCTIONS)
    return true;
#else
    return false;
#endif
}

inline bool isARM64()
{
#if CPU(ARM64)
    return true;
#else
    return false;
#endif
}

inline bool isX86()
{
#if CPU(X86_64) || CPU(X86)
    return true;
#else
    return false;
#endif
}

inline bool isX86_64()
{
#if CPU(X86_64)
    return true;
#else
    return false;
#endif
}

inline bool optimizeForARMv7IDIVSupported()
{
    return isARMv7IDIVSupported() && Options::useArchitectureSpecificOptimizations();
}

inline bool optimizeForARM64()
{
    return isARM64() && Options::useArchitectureSpecificOptimizations();
}

inline bool optimizeForX86()
{
    return isX86() && Options::useArchitectureSpecificOptimizations();
}

inline bool optimizeForX86_64()
{
    return isX86_64() && Options::useArchitectureSpecificOptimizations();
}

inline bool hasSensibleDoubleToInt()
{
    return optimizeForX86();
}

} // namespace JSC

