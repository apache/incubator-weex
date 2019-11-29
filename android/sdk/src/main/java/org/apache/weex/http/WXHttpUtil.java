/*
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
package org.apache.weex.http;

import android.content.Context;
import android.text.TextUtils;

import org.apache.weex.common.WXConfig;
import org.apache.weex.utils.WXViewUtils;

import java.util.Map;

/**
 * Created by lixinke on 16/4/6.
 */
public class WXHttpUtil {

  private static String sDefaultUA = null;

  public static final String KEY_USER_AGENT = "user-agent";

  public static String assembleUserAgent(Context ctx,Map<String, String> config) {
    if (TextUtils.isEmpty(sDefaultUA)) {
      StringBuilder builder = new StringBuilder();
      builder.append(config.get(WXConfig.sysModel))
          .append("(Android/")
          .append(config.get(WXConfig.sysVersion))
          .append(")")
          .append(" ")

          .append(TextUtils.isEmpty(config.get(WXConfig.appGroup)) ? "" : config.get(WXConfig.appGroup))
          .append("(")
          .append(TextUtils.isEmpty(config.get(WXConfig.appName)) ? "" : config.get(WXConfig.appName))
          .append("/")
          .append(config.get(WXConfig.appVersion))
          .append(")")
          .append(" ")

          .append("Weex/")
          .append(config.get(WXConfig.weexVersion))
          .append(" ")

          .append(TextUtils.isEmpty(config.get(WXConfig.externalUserAgent)) ? "" : config.get(WXConfig.externalUserAgent))
          .append(TextUtils.isEmpty(config.get(WXConfig.externalUserAgent)) ? "" : " ")

          .append(WXViewUtils.getScreenWidth(ctx) + "x" + WXViewUtils.getScreenHeight(ctx));
      sDefaultUA = builder.toString();
    }
    return sDefaultUA;
  }
}
