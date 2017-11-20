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
package com.taobao.weex.adapter;

import android.app.Application;
import android.text.TextUtils;
import android.view.View;

import com.taobao.weex.IWXActivityStateListener;
import com.taobao.weex.WXDebugTool;
import com.taobao.weex.WXPrettyFish;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.scalpel.ScalpelFrameLayout;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by lixinke on 16/6/6.
 */
public class DefautDebugAdapter implements IWXDebugAdapter {

  public static final String SHOW_3D_LAYER="show_3d_layer";
  private Map<String, String> options;

  @Override
  public void initDebug(final Application application) {
    WXSDKManager.getInstance().postOnUiThread(new Runnable() {
      @Override
      public void run() {
        WXPrettyFish.init(application);
        putDebugOptions(SHOW_3D_LAYER,"true");
      }
    }, 0);
  }

  @Override
  public View wrapContainer(WXSDKInstance instance, View wxView) {
    if (wxView != null && wxView.getContext() != null && wxView.getParent() == null && TextUtils.equals(getDebugOptions(SHOW_3D_LAYER),"true")) {
      ScalpelFrameLayout scalpelFrameLayout = new ScalpelFrameLayout(wxView.getContext());
      WXDebugTool.updateScapleView(scalpelFrameLayout);
      scalpelFrameLayout.addView(wxView);
      instance.registerActivityStateListener(new DebugActivityState(wxView));
      return scalpelFrameLayout;
    }
    return wxView;
  }

  @Override
  public void putDebugOptions(String key, String value) {
    if (options == null) {
      options = new HashMap<>();
    }
    options.put(key, value);
  }

  @Override
  public String getDebugOptions(String key) {
    if (options != null) {
      return options.get(key);
    }
    return null;
  }

  static class DebugActivityState implements IWXActivityStateListener {

    private View mWXView;

    public DebugActivityState(View wxView) {
      mWXView = wxView;
    }

    @Override
    public void onActivityCreate() {

    }

    @Override
    public void onActivityStart() {

    }

    @Override
    public void onActivityPause() {
      WXDebugTool.updateScapleView(null);
    }

    @Override
    public void onActivityResume() {
      if (mWXView != null && mWXView.getParent() instanceof ScalpelFrameLayout) {
        WXDebugTool.updateScapleView(mWXView.getParent());
      }
    }

    @Override
    public void onActivityStop() {

    }

    @Override
    public void onActivityDestroy() {

    }

    @Override
    public boolean onActivityBack() {
      return false;
    }
  }
}
