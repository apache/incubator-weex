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
package com.taobao.weex.ui.view.refresh.core;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import com.taobao.weex.common.WXThread;
import com.taobao.weex.ui.view.refresh.circlebar.CircleProgressBar;

public class WXRefreshView extends FrameLayout {

  private CircleProgressBar circleProgressBar;
  private LinearLayout linearLayout;

  public WXRefreshView(Context context) {
    super(context);
    setupViews();
  }

  public WXRefreshView(Context context, AttributeSet attrs) {
    super(context, attrs);
    setupViews();
  }

  public WXRefreshView(Context context, AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    setupViews();
  }

  private void setupViews() {
    linearLayout = new LinearLayout(getContext());
    FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(FrameLayout.LayoutParams
            .MATCH_PARENT,FrameLayout
            .LayoutParams.MATCH_PARENT);
    linearLayout.setOrientation(LinearLayout.VERTICAL);
    linearLayout.setGravity(Gravity.CENTER);
    addView(linearLayout,lp);
  }

  public void setContentGravity(int gravity) {
    if (linearLayout != null) {
      linearLayout.setGravity(gravity);
    }
  }

  /**
   * Setting refresh view or loading view
   *
   * @param view refresh or loading
   */
  public void setRefreshView(final View view) {
    if (view == null)
      return;
    post(WXThread.secure(new Runnable() {
      @Override
      public void run() {
        View child = null;
        View temp = view;
        if (view.getParent() != null) {
          ((ViewGroup) view.getParent()).removeView(view);
        }
        for (int i = 0;i<((ViewGroup)temp).getChildCount(); i++) {
          child = ((ViewGroup) temp).getChildAt(i);
          if (child instanceof CircleProgressBar)
            circleProgressBar = (CircleProgressBar) child;
        }
        linearLayout.addView(temp);
      }
    }));
  }

  /**
   * Set loading_indicator bgColor
   *
   * @param color
   */
  public void setProgressBgColor(int color) {
    if (circleProgressBar != null) {
      circleProgressBar.setBackgroundColor(color);
    }
  }

  /**
   * Set loading_indicator color
   *
   * @param color
   */
  public void setProgressColor(int color) {
    if (circleProgressBar != null) {
      circleProgressBar.setColorSchemeColors(color);
    }
  }

  protected void startAnimation() {
    if (circleProgressBar != null) {
      circleProgressBar.start();
    }
  }

  /**
   * Set the start and end trim for the progress spinner arc.
   *
   * @param startAngle start angle
   * @param endAngle end angle
   */
  public void setStartEndTrim(float startAngle, float endAngle) {
    if (circleProgressBar != null) {
      circleProgressBar.setStartEndTrim(startAngle, endAngle);
    }
  }

  protected void stopAnimation() {
    if (circleProgressBar != null) {
      circleProgressBar.stop();
    }
  }

  /**
   * Set the amount of rotation to apply to the progress spinner.
   *
   * @param rotation Rotation is from [0..1]
   */
  public void setProgressRotation(float rotation) {
    if (circleProgressBar != null)
      circleProgressBar.setProgressRotation(rotation);
  }
}
