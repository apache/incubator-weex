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

#include "js_processer.h"
namespace android {

std::vector<base::android::JSEnginePtrContainer **> JSContainerProcesser::m_saved_container;

base::android::JSEnginePtrContainer **JSContainerProcesser::CreateJSPtrContainer() {
  base::android::JSEnginePtrContainer **result = nullptr;
  result = new base::android::JSEnginePtrContainer *[1];
  auto ptr_container = new base::android::JSEnginePtrContainer();
  *result = ptr_container;
  LOGD_TAG("JSEngine",
           "Create %ld",
           (long) result);

  m_saved_container.push_back(result);
  return result;
}
void JSContainerProcesser::DestroyJSPtrContainer(long ctxContainer) {
  if (ctxContainer == 0) {
    return;
  }

  auto **realPtr = (base::android::JSEnginePtrContainer **) ctxContainer;
  removeContainer(realPtr);

  if (*realPtr == nullptr) {
    return;
  }

  long contextPtr = (*realPtr)->m_ptr_weexcore_js_context;
  if (contextPtr == 0) {
    return;
  }

  LOGD_TAG("JSEngine",
           "Destroy %ld",
           (long) realPtr);

  auto pContext = (WeexCore::JSContext *) contextPtr;
  delete pContext;
  pContext = nullptr;

  delete (*realPtr);
  *realPtr = nullptr;

  delete realPtr;
  realPtr = nullptr;
}
WeexCore::JSContext *JSContainerProcesser::ExtraJsContext(long ctxContainer) {
  if (ctxContainer == 0) {
    return nullptr;
  }

  auto **realPtr = (base::android::JSEnginePtrContainer **) ctxContainer;
  if (*realPtr == nullptr) {
    return nullptr;
  }

  if (!hasContainer(realPtr)) {
    LOGD_TAG("JSEngine", "Do not Has Container");
    return nullptr;
  }

  long contextPtr = (*realPtr)->m_ptr_weexcore_js_context;
  if (contextPtr == 0) {
    return nullptr;
  }
  auto jsContext = (WeexCore::JSContext *) contextPtr;
  return jsContext;
}

bool JSContainerProcesser::hasContainer(base::android::JSEnginePtrContainer **ptr) {
  auto it = findContainer(ptr);
  return it != m_saved_container.end();
}

std::vector<base::android::JSEnginePtrContainer **>::iterator JSContainerProcesser::findContainer(
    base::android::JSEnginePtrContainer **ptr) {
  auto it = m_saved_container.begin();
  while (it != m_saved_container.end()) {
    if (*it == ptr) {
      break;
    }
    it++;
  }
  return it;
}
void JSContainerProcesser::removeContainer(base::android::JSEnginePtrContainer **ptr) {
  auto it = findContainer(ptr);
  if (it != m_saved_container.end()) {
    m_saved_container.erase(it);
  }
}
}  // namespace android