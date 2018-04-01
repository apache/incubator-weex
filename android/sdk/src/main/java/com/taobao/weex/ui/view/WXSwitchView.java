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
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.support.v7.widget.SwitchCompat;
import android.view.Gravity;
import android.view.MotionEvent;

import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.utils.WXResourceUtils;

public class WXSwitchView extends SwitchCompat implements WXGestureObservable {

  static final String TRACK_TINT_COLOR_NORMAL = "#b9b8b8";
  static final String TRACK_TINT_COLOR_ACTIVATED = "#fb97bd";
  static final String TRACK_TINT_COLOR_DISABLED = "#42b9b8b8";
  static final String THUMB_TINT_COLOR_NORMAL = "#f1f1f1";
  static final String THUMB_TINT_COLOR_ACTIVATED = "#fc4482";
  static final String THUMB_TINT_COLOR_DISABLED = "#42f1f1f1";

  private WXGesture wxGesture;

  public WXSwitchView(Context context) {
    super(context);
    setShowText(false);
    setGravity(Gravity.CENTER_VERTICAL);
    setBackgroundColor(Color.TRANSPARENT);
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

  public void setThumbColor(String thumbTintColorNormal, String thumbTintColorActivated) {
    setThumbTintList(new ColorStateList(new int[][]{
        new int[]{-android.R.attr.state_enabled},
        new int[]{-android.R.attr.state_checked},
        new int[]{android.R.attr.state_checked}
    }, new int[]{
        WXResourceUtils.getColor(THUMB_TINT_COLOR_DISABLED, Color.parseColor(THUMB_TINT_COLOR_DISABLED)),
        WXResourceUtils.getColor(thumbTintColorNormal, Color.parseColor(THUMB_TINT_COLOR_NORMAL)),
        WXResourceUtils.getColor(thumbTintColorActivated, Color.parseColor(THUMB_TINT_COLOR_ACTIVATED))
    }));
  }

  public void setTrackColor(String trackTintColorNormal, String trackTintColorActivated) {
    setTrackTintList(new ColorStateList(new int[][]{
        new int[]{-android.R.attr.state_enabled},
        new int[]{-android.R.attr.state_checked},
        new int[]{android.R.attr.state_checked}
    }, new int[]{
        WXResourceUtils.getColor(TRACK_TINT_COLOR_DISABLED, Color.parseColor(TRACK_TINT_COLOR_DISABLED)),
        WXResourceUtils.getColor(trackTintColorNormal, Color.parseColor(TRACK_TINT_COLOR_NORMAL)),
        WXResourceUtils.getColor(trackTintColorActivated, Color.parseColor(TRACK_TINT_COLOR_ACTIVATED))
    }));
  }
}
