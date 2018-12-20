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
#include <string.h>
#ifdef OS_ANDROID
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include "base/third_party/icu/icu_utf.h"

namespace weex {
namespace base {

constexpr int32_t kErrorCodePoint = 0xFFFD;

template <typename T>
std::string to_string(T value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

static bool IsValidCodepoint(uint32_t code_point) {
  // Excludes the surrogate code points ([0xD800, 0xDFFF]) and
  // codepoints larger than 0x10FFFF (the highest codepoint allowed).
  // Non-characters and unassigned codepoints are allowed.
  return code_point < 0xD800u ||
         (code_point >= 0xE000u && code_point <= 0x10FFFFu);
}

static uint32_t convert_single_char(char16_t in) {
  if (!CBU16_IS_SINGLE(in) || !IsValidCodepoint(in)) {
    return kErrorCodePoint;
  }
  return in;
}

inline static std::string to_utf8(uint16_t* utf16, size_t length) {
  char16_t *WC = reinterpret_cast<char16_t *>(utf16);
  char* dest = static_cast<char*>(malloc(length * sizeof(char16_t) + sizeof(char16_t)));
  memset(dest, 0, length * sizeof(char16_t) + sizeof(char16_t));
  int32_t i = 0;
  int32_t d_len = 0;
  int32_t* dest_len = &d_len;
  // Always have another symbol in order to avoid checking boundaries in the
  // middle of the surrogate pair.
  while (i < length - 1) {
    int32_t code_point;

    if (CBU16_IS_LEAD(WC[i]) && CBU16_IS_TRAIL(WC[i + 1])) {
      code_point = CBU16_GET_SUPPLEMENTARY(WC[i], WC[i + 1]);
      i += 2;
    } else {
      code_point = convert_single_char(WC[i]);
      ++i;
    }

    CBU8_APPEND_UNSAFE(dest, *dest_len, code_point);
  }

  if (i < length)
    CBU8_APPEND_UNSAFE(dest, *dest_len, convert_single_char(WC[i]));

  std::string output(dest);
  free(dest);
  return output;
}

#ifdef OS_ANDROID
static std::u16string to_utf16(char* utf8, size_t length) {
  std::u16string dest_str;
  dest_str.resize(length);
  auto* dest = &dest_str[0];
  int32_t dest_len = 0;

  bool success = true;

  for (int32_t i = 0; i < length;) {
    int32_t code_point;
    CBU8_NEXT(utf8, i, static_cast<int32_t>(length), code_point);

    if (!IsValidCodepoint(code_point)) {
      success = false;
      code_point = kErrorCodePoint;
    }

    CBU16_APPEND_UNSAFE(dest, dest_len, code_point);
  }
  if (!success) {
    return std::u16string();
  }
  dest_str.resize(dest_len);
  dest_str.shrink_to_fit();
  return dest_str;
}
#endif

}  // namespace base
}  // namespace weex
#endif  // CORE_BASE_STRING_UTIL_H
