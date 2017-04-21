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
package com.alibaba.weex.extend.component;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.support.annotation.NonNull;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.widget.PopupWindow;

import com.alibaba.weex.extend.view.WXMaskView;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXVContainer;

/**
 * Created by lixinke on 2016/12/26.
 */

public class WXMask extends WXVContainer {

  private PopupWindow mPopupWindow;
  private WXMaskView mContainerView;

  public WXMask(WXSDKInstance instance, WXDomObject dom, WXVContainer parent) {
    super(instance, dom, parent);
  }

  @Override
  protected View initComponentHostView(@NonNull Context context) {

    mContainerView = new WXMaskView(context);
    mPopupWindow = new PopupWindow(context);
    mPopupWindow.setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
    mPopupWindow.setClippingEnabled(false);
    mPopupWindow.setContentView(mContainerView);
    mPopupWindow.setWidth(WindowManager.LayoutParams.WRAP_CONTENT);
    mPopupWindow.setHeight(WindowManager.LayoutParams.WRAP_CONTENT);
    mPopupWindow.showAtLocation(((Activity) context).getWindow().getDecorView(), Gravity.TOP, (int) getDomObject().getLayoutX(), (int) getDomObject().getLayoutY());

    return mContainerView;
  }

  @Override
  public boolean isVirtualComponent() {
    return true;
  }

  @Override
  public void removeVirtualComponent() {
    if (mPopupWindow.isShowing()) {
      mPopupWindow.dismiss();
    }
  }
}
