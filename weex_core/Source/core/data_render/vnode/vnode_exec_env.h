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

#ifndef CORE_DATA_RENDER_VNODE_VNODE_EXEC_ENV_H
#define CORE_DATA_RENDER_VNODE_VNODE_EXEC_ENV_H

#include "core/data_render/exec_state.h"

namespace weex {
namespace core {
namespace data_render {
    
class VNodeExecEnv {
 public:
  static void ImportExecEnv(ExecState *state);
  static void ImportExecData(ExecState *state, const std::string &init_data_str);
  static void ParseStyle(ExecState *state);
  static void ParseData(ExecState *state);
  static void ParseScript(ExecState *state);
};
    
Value StringToValue(ExecState *exec_state, const std::string &str);

}  // namespace data_render
}  // namespace core
}  // namespace weex
#endif  // CORE_DATA_RENDER_VNODE_VNODE_EXEC_ENV_H
