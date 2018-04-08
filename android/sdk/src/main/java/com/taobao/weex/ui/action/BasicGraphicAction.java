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
package com.taobao.weex.ui.action;

import android.text.TextUtils;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.utils.WXLogUtils;

public abstract class BasicGraphicAction implements IExecutable, Runnable {

  private final String mPageId;
  private final String mRef;

  public BasicGraphicAction(String pageId, String ref) {
    this.mPageId = pageId;
    this.mRef = ref;
  }

  public final String getPageId() {
    return mPageId;
  }

  public final String getRef() {
    return mRef;
  }

  public void executeActionOnRender() {
    if (TextUtils.isEmpty(mPageId)) {
        WXLogUtils.e("[BasicGraphicAction] pageId can not be null");
        if (WXEnvironment.isApkDebugable()) {
            throw new RuntimeException("["+getClass().getName()+"] pageId can not be null");
        }
        return;
    }
    WXSDKManager.getInstance().getWXRenderManager().postGraphicAction(mPageId, this);
  }

  @Override
  public void run() {
    try {
      executeAction();
    } catch (Throwable e) {
      //catch everything may throw from exection.
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e("BasicGraphicAction",
            "SafeRunnable run throw expection:" + e.getMessage());
        throw e;
      }
      WXLogUtils.w("BasicGraphicAction", e);
    }
  }
}
