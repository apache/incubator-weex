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
//
// Created by yxp on 2018/6/29.
//

#include "android/jsengine/bridge/script/script_bridge_in_simple.h"

#include "android/jsengine/object/weex_env.h"
#include "android/jsengine/bridge/script/core_side_in_simple.h"
#include "android/jsengine/bridge/script/script_side_in_simple.h"
#include "android/jsengine/bridge/script/script_side_in_queue.h"
#include "android/jsengine/weex_runtime.h"

namespace weex {
namespace bridge {
namespace js {
ScriptBridgeInSimple* ScriptBridgeInSimple::g_instance = NULL;

ScriptBridgeInSimple::ScriptBridgeInSimple() {
  WeexEnv::getEnv()->setScriptBridge(this);
  set_script_side(new ScriptSideInSimple(new WeexRuntime(nullptr, this, false)));
  set_core_side(new CoreSideInSimple());
}

ScriptBridgeInSimple::~ScriptBridgeInSimple() {}
}  // namespace js
}  // namespace bridge
}  // namespace weex
