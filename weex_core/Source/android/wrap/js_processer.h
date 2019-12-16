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

#ifndef WEEX_PROJECT_JS_PROCESSER_H
#define WEEX_PROJECT_JS_PROCESSER_H

#include <vector>
#include "android/jsengine_ptr_container.h"
#include "android/wrap/js_context.h"

namespace android {
class JSContainerProcesser {
 public:
  static base::android::JSEnginePtrContainer **CreateJSPtrContainer();
  static void DestroyJSPtrContainer(long ctxContainer);
  static WeexCore::JSContext *ExtraJsContext(long ctxContainer);

 private:

  static std::vector<base::android::JSEnginePtrContainer **>::iterator findContainer(base::android::JSEnginePtrContainer **ptr);
  static bool hasContainer(base::android::JSEnginePtrContainer **ptr);
  static void removeContainer(base::android::JSEnginePtrContainer **ptr);

  static std::vector<base::android::JSEnginePtrContainer **> m_saved_container;

};

}  // namespace android
#endif //WEEX_PROJECT_JS_PROCESSER_H
