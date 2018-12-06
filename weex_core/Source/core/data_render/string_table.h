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

#ifndef CORE_DATA_RENDER_STRING_TABLE_H
#define CORE_DATA_RENDER_STRING_TABLE_H

#include <memory>
#include <string>
#include <map>
#include <vector>

namespace weex {
namespace core {
namespace data_render {
class String {
 public:
  explicit String(const char *str, std::size_t len);
  explicit String(const std::string &str);
  ~String();

  char *c_str() { return str_.get(); }

  std::size_t length() { return length_; }

 private:
  std::unique_ptr<char[]> str_;
  size_t length_;
};

class StringTable {
 public:
  StringTable() : store_() {}
  ~StringTable();

  String *StringFromUTF8(const std::string &str);
  inline const std::vector<std::pair<std::string, std::unique_ptr<String>>>& store() {return store_;}

 private:
  std::vector<std::pair<std::string, std::unique_ptr<String>>> store_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_STRING_TABLE_H
