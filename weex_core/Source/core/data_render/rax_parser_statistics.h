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

#ifndef DATA_RENDER_RAX_PARSER_STATISTICS_
#define DATA_RENDER_RAX_PARSER_STATISTICS_

#include <vector>
#include <memory>

namespace weex {
namespace core {
namespace data_render {

#define COUNTER_TYPE(F) \
F(Token) \
F(Expression) \
F(InputCharacter) \
F(Allocations) \
F(Line)
    
class Statistics {
    enum CountType {
#define COUNT_TYPE(t) k##t,
        COUNTER_TYPE(COUNT_TYPE)
#undef COUNT_TYPE
        kSize,
    };
public:
    Statistics() : counters_{0} { }
#define COUNTER_ACCESSOR(t) std::size_t &t() { return counters_[CountType::k##t]; }
        COUNTER_TYPE(COUNTER_ACCESSOR)
#undef COUNTER_ACCESSOR
    void dump();
private:
    std::size_t counters_[CountType::kSize];    
};
    
}
}
}

#endif
