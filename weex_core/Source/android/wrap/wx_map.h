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

#ifndef WEEX_PROJECT_WX_MAP_H
#define WEEX_PROJECT_WX_MAP_H

#include <jni.h>
#include <map>
#include <string>
#include <vector>
#include "android/base/jni/scoped_java_ref.h"
#include "android/wrap/jni_object_wrap.h"
#include "base/common.h"

namespace WeexCore {
class WXMap : public JNIObjectWrap {
 public:
  static bool RegisterJNIUtils(JNIEnv* env);
  WXMap();
  virtual ~WXMap();

  void Put(JNIEnv* env,
           const std::vector<std::pair<std::string, std::string>>& vector);
  void Put(JNIEnv* env, const std::map<std::string, std::string>& map);

 private:
  DISALLOW_COPY_AND_ASSIGN(WXMap);
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_WX_MAP_H
