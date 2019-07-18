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
/**
# Copyright 2018 Taobao (China) Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
**/

#ifndef FLUTTER_UNICORN_RUNTIME_RUNTIME_VM_H_
#define FLUTTER_UNICORN_RUNTIME_RUNTIME_VM_H_

#include <cstdlib>
#include <memory>

namespace unicorn {
class EngineContext;
class RuntimeContext;

class RuntimeVM {
 public:
  //static RuntimeVM* ForProcess();

  RuntimeVM();
  ~RuntimeVM();

  static void* CreateEngineVM();
  static void ReleaseEngineVM(void* vm);

  static EngineContext* GetEngineContext(const void* js_ctx);
  static void NotifyContextCreated(RuntimeContext* context);
  static void NotifyContextDestorying(RuntimeContext* context);

  void* EngineVM();
  bool Shutdown();

 private:
    void* vm_;

  void Initialize();
};

}  // namespace unicorn

#endif  // FLUTTER_UNICORN_RUNTIME_RUNTIME_VM_H_
