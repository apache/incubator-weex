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
#ifndef _JSENGINE_PTR_CONTAINER_H_
#define _JSENGINE_PTR_CONTAINER_H_

namespace base {
namespace android {
class JSEnginePtrContainer {
 public:
  JSEnginePtrContainer(){
    this->m_ptr_jss_js_action = 0;
    this->m_ptr_weexcore_js_context = 0;
  };

  ~JSEnginePtrContainer() {
    this->m_ptr_jss_js_action = 0;
    this->m_ptr_weexcore_js_context = 0;
  }

  long m_ptr_jss_js_action;
  long m_ptr_weexcore_js_context;
};
}  // namespace android
}  // namespace base

#endif  //_JSENGINE_PTR_CONTAINER_H_