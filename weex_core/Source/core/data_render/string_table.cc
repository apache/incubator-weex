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

#include <string.h>
#include "core/data_render/string_table.h"

namespace weex {
namespace core {
namespace data_render {
    
String::String(const char *str, std::size_t len) {
  length_ = len;
  str_ = std::unique_ptr<char[]>(new char[len + 1]);
  memcpy(str_.get(), str, len);
  str_[len] = 0;
}

String::String(const std::string &str) : String(str.c_str(), str.length()) {}

String::~String() {}

String *StringTable::StringFromUTF8(const std::string &str) {
    for (auto &it : store_) {
        if (it.first == str) {
            return it.second.get();
        }
    }
    std::string key = str;
    auto result = new String(key);
    store_.push_back(std::make_pair(std::move(key), std::unique_ptr<String>(result)));
    return result;
}

StringTable::~StringTable() {}

}  // namespace data_render

}  // namespace core
}  // namespace weex
