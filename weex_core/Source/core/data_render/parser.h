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

#include <string>
#include "core/data_render/statement.h"
#include "core/data_render/ast.h"

namespace weex {
namespace core {
namespace data_render {

class ParseResult {
 public:
  friend class Parser;

  ParseResult() {};
  ParseResult(Handle<Expression> expr) : expr_(expr) {
  };

  inline Handle<ChunkStatement> expr() const { return expr_; }
 private:
  Handle<ChunkStatement> expr_;
};

enum ASTParseError {
  UNKOWN_ERROR,
  BODY_NONE_ERROR,
  FILE_FORMAT_ERROR,
  SYSTEM_MEMORY_ERROR,
};

class Parser final {
 public:
  // Parse. If parse fails, return Json() and assign an error message to err.
  static ParseResult parse(const std::string& in,
                           std::string& err);
 private:
  Parser() {};
};

}  // namespace data_render
}  // namespace core
}  // namespace weex
