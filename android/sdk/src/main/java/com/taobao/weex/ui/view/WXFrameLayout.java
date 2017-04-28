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
package com.taobao.weex.ui.view;

import android.content.Context;
import android.graphics.Canvas;
import android.support.annotation.Nullable;
import android.view.MotionEvent;
import android.widget.FrameLayout;

import com.taobao.weex.ui.component.WXDiv;
import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.utils.WXViewUtils;

import java.lang.ref.WeakReference;

/**
 * FrameLayout wrapper
 *
 */
public class WXFrameLayout extends FrameLayout implements WXGestureObservable,IRenderStatus<WXDiv>,IRenderResult<WXDiv> {

  private WXGesture wxGesture;

  private WeakReference<WXDiv> mWeakReference;

  public WXFrameLayout(Context context) {
    super(context);
  }

  @Override
  public void registerGestureListener(WXGesture wxGesture) {
    this.wxGesture = wxGesture;
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    boolean result = super.onTouchEvent(event);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, event);
    }
    return result;
  }

  @Override
  protected void onDraw(Canvas canvas) {
    WXViewUtils.clipCanvasWithinBorderBox(this, canvas);
    super.onDraw(canvas);
  }

  @Override
  public void holdComponent(WXDiv component) {
    mWeakReference = new WeakReference<WXDiv>(component);
  }

  @Nullable
  @Override
  public WXDiv getComponent() {
    return null != mWeakReference ? mWeakReference.get() : null;
  }
}
