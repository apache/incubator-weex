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

#ifndef CORE_BASE_STRING_UTIL_H
#define CORE_BASE_STRING_UTIL_H

#include <sstream>
#include <string>
#include <codecvt>

namespace weex {
namespace base {

template <typename T>
std::string to_string(T value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

std::string to_utf8(uint16_t* utf16, size_t length) {
  char16_t *WC = reinterpret_cast<char16_t *>(utf16);
  std::u16string str(WC, length);
  /* 转换宽字符字符串 */
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(str);
}

}  // namespace base
}  // namespace weex
#endif  // CORE_BASE_STRING_UTIL_H
