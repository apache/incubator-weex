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
#ifndef CORE_DATA_RENDER_TOKENIZER_
#define CORE_DATA_RENDER_TOKENIZER_

#include "core/data_render/scanner.h"
#include "core/data_render/token.h"

namespace weex {
namespace core {
namespace data_render {

class TokenizerState;

/*
 * implementation of complete Tokenizer to be independant of flex
 */
class Tokenizer {
 public:
  Tokenizer(CharacterStream* stream);
  ~Tokenizer();
  Token::Type Peek();

  void Advance(bool divide_expected = false);

  void Reset(CharacterStream* stream);

  Token& CurrentToken();

 private:
  Token AdvanceInternal(bool not_regex);
  Token ParseString(char delim);
  Token ParseNumber(char start);
  Token ParseRegex(bool* ok);

  TokenizerState* state_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex
#endif  // CORE_DATA_RENDER_TOKENIZER_