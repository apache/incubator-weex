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
package com.taobao.weex.ui.module;

import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;
import android.os.LocaleList;
import android.text.TextUtils;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.common.WXModule;

import java.util.Arrays;
import java.util.List;
import java.util.Locale;

/**
 * Created by moxun on 2017/9/26.
 * <p>
 * Ref: https://tools.ietf.org/html/bcp47
 */

public class WXLocaleModule extends WXModule {

  @JSMethod(uiThread = false)
  public String getLanguage() {
    return getLanguageTags();
  }

  @JSMethod(uiThread = false)
  public void getLanguage(JSCallback callback) {
    callback.invoke(getLanguageTags());
  }

  @JSMethod(uiThread = false)
  public List<String> getLanguages() {
    String[] tags = getLanguageTags().split(",");
    return Arrays.asList(tags);
  }

  @JSMethod(uiThread = false)
  public void getLanguages(JSCallback callback) {
    callback.invoke(getLanguageTags().split(","));
  }

  private String getLanguageTags() {
    Context application = WXEnvironment.getApplication();
    if (application != null) {
      Resources res = application.getResources();
      if (res != null) {
        Configuration configuration = res.getConfiguration();
        if (configuration != null) {
          if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            LocaleList localeList = configuration.getLocales();
            return localeList.toLanguageTags();
          } else {
            Locale local = configuration.locale;
            if (local != null) {
              return toLanguageTag(local);
            }
          }
        }
      }
    }
    return "";
  }

  private String toLanguageTag(Locale locale) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      return locale.toLanguageTag();
    } else {
      StringBuilder sb = new StringBuilder();
      String language = locale.getLanguage();
      String script = locale.getScript();
      String region = locale.getCountry();
      sb.append(language);
      if (!TextUtils.isEmpty(script)) {
        sb.append("-").append(script);
      }
      if (!TextUtils.isEmpty(region)) {
        sb.append("-").append(region);
      }
      return sb.toString();
    }
  }
}
