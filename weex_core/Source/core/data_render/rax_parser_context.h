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
//
// Created by pentao.pt on 2018/7/25.
//

#ifndef DATA_RENDER_RAX_PARSER_CONTEXT_
#define DATA_RENDER_RAX_PARSER_CONTEXT_

#include "core/data_render/rax_parser_statistics.h"

namespace weex {
namespace core {
namespace data_render {
  
class Scope;

class ParserContext {
public:
    ParserContext();
    ~ParserContext() { };
    Scope *globalScope() { return global_scope_.get(); }
    Statistics &Counters();
private:
    std::unique_ptr<Scope> global_scope_;
    Statistics statistics_;
};
    
}
}
}

#endif
