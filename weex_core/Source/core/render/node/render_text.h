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
#ifndef CORE_RENDER_NODE_RENDER_TEXT_H_
#define CORE_RENDER_NODE_RENDER_TEXT_H_

#include <string>

#include "core/render/node/render_object.h"

namespace WeexCore {
class RenderText : public RenderObject {
 private:
  void UpdateAttr(std::string key, std::string value);

  StyleType UpdateStyle(std::string key, std::string value);
};
}  // namespace WeexCore

#endif  // CORE_RENDER_NODE_RENDER_TEXT_H_
