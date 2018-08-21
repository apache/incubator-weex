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
// Created by pengtao.pt on 2018/7/25.
//

#include <string>
#include <cstdlib>
#include <memory>
#include "core/data_render/tokenizer.h"
#include "core/data_render/rax_parser_builder.h"
#include "core/data_render/rax_parser_context.h"

namespace weex {
namespace core {
namespace data_render {
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

RAXParserBuilder::RAXParserBuilder(const std::string &content) :
    content_(content),
    context_{ make_unique<ParserContext>() },
    factory_{ ASTFactory::GetFactoryInstance() },
    manager_{ make_unique<ScopeManager>(context_.get()) },
    stream_(new StandardCharacterStream(content_))
{
    lex_ = make_unique<Tokenizer>(stream_.get(), context_.get());
    locator_ = make_unique<SourceLocator>(lex_.get());
    builder_ = make_unique<ASTBuilder>(context_.get(), factory_, locator_.get(), manager_.get());
    parser_ = make_unique<RAXParser>(context_.get(), builder_.get(), locator_.get(), lex_.get(), manager_.get());
}
    
}
}
}
