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
import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.MotionEvent;
import android.widget.FrameLayout;
import com.taobao.weex.ui.component.WXDiv;
import com.taobao.weex.ui.flat.widget.Widget;
import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;
import java.lang.ref.WeakReference;
import java.util.List;

/**
 * FrameLayout wrapper
 *
 */
public class WXFrameLayout extends FrameLayout implements WXGestureObservable,IRenderStatus<WXDiv>,IRenderResult<WXDiv> {

  private WXGesture wxGesture;

  private WeakReference<WXDiv> mWeakReference;

  private List<Widget> mWidgets;

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
  public void holdComponent(WXDiv component) {
    mWeakReference = new WeakReference<WXDiv>(component);
  }

  @Nullable
  @Override
  public WXDiv getComponent() {
    return null != mWeakReference ? mWeakReference.get() : null;
  }

  public void mountFlatGUI(List<Widget> widgets){
    this.mWidgets = widgets;
    if (mWidgets != null) {
      setWillNotDraw(true);
    }
    invalidate();
  }

  public void unmountFlatGUI(){
    mWidgets = null;
    setWillNotDraw(false);
    invalidate();
  }

  @Override
  protected boolean verifyDrawable(@NonNull Drawable who) {
    return mWidgets != null || super.verifyDrawable(who);
  }

  @Override
  protected void dispatchDraw(Canvas canvas) {
    try {
      if (mWidgets != null) {
        canvas.save();
        canvas.translate(getPaddingLeft(), getPaddingTop());
        for (Widget widget : mWidgets) {
          widget.draw(canvas);
        }
        canvas.restore();
      } else {
        WXViewUtils.clipCanvasWithinBorderBox(this, canvas);
        super.dispatchDraw(canvas);
      }
    }catch (Throwable e){
      WXLogUtils.e("FlatGUI Crashed when dispatchDraw", WXLogUtils.getStackTrace(e));
    }
  }
}
