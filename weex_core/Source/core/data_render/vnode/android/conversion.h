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
#ifdef OS_ANDROID

#ifndef CORE_DATA_RENDER_VNODE_ANDROID_CONVERSION_H_
#define CORE_DATA_RENDER_VNODE_ANDROID_CONVERSION_H_

#include <string>
#include <unordered_map>
#include <vector>
#include "include/WeexApiHeader.h"
#include "third_party/json11/json11.hpp"

namespace weex {
namespace core {
namespace data_render {
class Value;
class Conversion {
 public:
  static json11::Json GenJSON(const Value* v);
  static VALUE_WITH_TYPE* GenValueWithType(const char* event_str);
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_VNODE_ANDROID_CONVERSION_H_
#endif
