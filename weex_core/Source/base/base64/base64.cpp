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

#include "./base64.h"
#include <string.h>

bool Base64Encode(const std::string& input, std::string* output) {
  std::string temp;
  temp.resize(modp_b64_encode_len(input.size()));  // makes room for null byte

  // null terminates result since result is base64 text!
  int input_size = static_cast<int>(input.size());
  int output_size= modp_b64_encode(&(temp[0]), input.data(), input_size);
  if (output_size < 0)
    return false;

  temp.resize(output_size);  // strips off null byte
  output->swap(temp);
  return true;
}

bool Base64Decode(const std::string& input, std::string* output) {
  std::string temp;
  temp.resize(modp_b64_decode_len(input.size()));

  // does not null terminate result since result is binary data!
  int input_size = static_cast<int>(input.size());
  int output_size = modp_b64_decode(&(temp[0]), input.data(), input_size);
  if (output_size < 0)
    return false;

  temp.resize(output_size);
  output->swap(temp);
  return true;
}