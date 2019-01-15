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

#include "DFGBasicBlock.h"
#include "DFGBlockMap.h"
#include "DFGBlockSet.h"
#include "DFGCFG.h"
#include "DFGCommon.h"
#include "DFGGraph.h"
#include <wtf/Dominators.h>
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>

namespace JSC { namespace DFG {

class Dominators : public WTF::Dominators<CFG> {
    WTF_MAKE_NONCOPYABLE(Dominators);
    WTF_MAKE_FAST_ALLOCATED;
public:
    Dominators(Graph& graph)
        : WTF::Dominators<CFG>(*graph.m_cfg)
    {
    }
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
