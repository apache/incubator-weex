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
package org.apache.weex.ui.module;

import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;

import org.apache.weex.WXEnvironment;
import org.apache.weex.annotation.JSMethod;
import org.apache.weex.bridge.JSCallback;
import org.apache.weex.bridge.WXBridgeManager;
import org.apache.weex.common.WXModule;
import org.apache.weex.utils.LogLevel;
import java.util.Map;


public class ConsoleLogModule extends WXModule {

  @JSMethod(uiThread = false)
  public void switchLogLevel(@Nullable String logLevel, @Nullable JSCallback callback) {
    LogLevel logLevelEnum = getLogLevel(logLevel);
    Map<String, String> ret = new ArrayMap<>();
    if (logLevelEnum != null) {
      WXEnvironment.sLogLevel = logLevelEnum;
      WXBridgeManager.getInstance().setLogLevel(WXEnvironment.sLogLevel.getValue(),WXEnvironment.isPerf());
      ret.put("status", "success");
    } else {
      ret.put("status", "failure");
    }

    if (callback != null) {
      callback.invoke(ret);
    }

  }

  @JSMethod(uiThread = false)
  public void setPerfMode(@Nullable String on) {
    WXEnvironment.isPerf = "true".equals(on);
    WXBridgeManager.getInstance().setLogLevel(WXEnvironment.sLogLevel.getValue(),WXEnvironment.isPerf());
  }

  private @Nullable LogLevel getLogLevel(@Nullable String logLevel) {
    LogLevel logLevelEnum = null;
    if(!TextUtils.isEmpty(logLevel)){
      switch (logLevel){
        case "off":
          logLevelEnum = LogLevel.OFF;
          break;
        case "error":
          logLevelEnum = LogLevel.ERROR;
          break;
        case "warning":
          logLevelEnum = LogLevel.WARN;
          break;
        case "info":
          logLevelEnum = LogLevel.INFO;
          break;
        case "debug":
          logLevelEnum = LogLevel.DEBUG;
          break;
      }
    }
    return logLevelEnum;
  }


}
