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

#include "base/utils/log_utils.h"

namespace Weex{
  bool LogUtil::mDebugMode = false;
  void LogUtil::setDebugMode(bool debug){
    mDebugMode = debug;
  }
  void LogUtil::ConsoleLogPrint(int level, const char* tag, const char* log) {
        // Log = 1, 
        // Warning = 2,
        // Error = 3,
        // Debug = 4,
        // Info = 5,
        
        // if (!mDebugMode) {
        //   LOGE("LogUtil mDebugMode is false");
        //   return;
        // }
        switch(level) {
          case 1:
            LOGV_TAG("jsengine", "%s", log);
            break;
          case 2:
            LOGW_TAG("jsengine", "%s", log);
            break;
          case 3:
            LOGE_TAG("jsengine", "%s", log);
            break;
          case 4:
            LOGD_TAG("jsengine", "%s", log);
            break;
          case 5:
            LOGI_TAG("jsengine", "%s", log);
            break;
          default:
            LOGV_TAG("jsengine", "%s", log);
            break;
        }
      }

}
