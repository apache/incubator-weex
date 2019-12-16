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

#include "object/weex_env.h"
#include "js_action_task.h"
JSActionTask::JSActionTask(long ctxContainer, const int32_t jsActionType, const std::string arg)
    : WeexTask("") {
  this->m_ctxContainer = ctxContainer;
  this->m_js_action_type = jsActionType;
  this->m_arg = arg;
}
void JSActionTask::run(WeexRuntime *runtime) {
  JSAction *pAction = WeexEnv::getEnv()->getJSAction(this->m_ctxContainer);
  if(pAction == nullptr) {
    return;
  }
  if (pAction) {
    if (this->m_js_action_type == static_cast<int32_t>(JSACTION::BIND)) {
      pAction->BindFunction(m_arg.c_str());
    } else if (this->m_js_action_type == static_cast<int32_t>(JSACTION::UNBIND)) {
      pAction->UnBindFunction(m_arg.c_str());
    } else if (this->m_js_action_type == static_cast<int32_t>(JSACTION::EXEJS)) {
      pAction->EvaluateScript(m_arg.c_str());
    }
  }

}
