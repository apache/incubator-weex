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

package com.taobao.weex.ui.animation;

import android.animation.IntEvaluator;
import android.animation.TimeInterpolator;
import android.animation.ValueAnimator;
import android.support.annotation.NonNull;
import android.support.v4.util.Pair;
import android.view.View;
import android.view.ViewGroup;

import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.view.IRenderResult;

public class DimensionUpdateListener implements ValueAnimator.AnimatorUpdateListener {

  private View view;
  private Pair<Integer, Integer> width;
  private Pair<Integer, Integer> height;
  private IntEvaluator intEvaluator;

  public DimensionUpdateListener(@NonNull View view) {
    this.view = view;
    intEvaluator = new IntEvaluator();
  }

  public void setWidth(int from, int to) {
    width = new Pair<>(from, to);
  }

  public void setHeight(int from, int to) {
    height = new Pair<>(from, to);
  }

  @Override
  public void onAnimationUpdate(ValueAnimator animation) {
    if (view.getLayoutParams() != null) {
      ViewGroup.LayoutParams layoutParams = view.getLayoutParams();
      TimeInterpolator interpolator = animation.getInterpolator();
      float fraction = animation.getAnimatedFraction();
      int preWidth = layoutParams.width;
      int preHeight = layoutParams.height;
      if (width != null) {
        layoutParams.width = intEvaluator.evaluate(interpolator.getInterpolation(fraction),
                                                   width.first,
                                                   width.second);
      }
      if (height != null) {
        layoutParams.height = intEvaluator.evaluate(interpolator.getInterpolation(fraction),
                                                    height.first,
                                                    height.second);
      }
      if (preHeight != layoutParams.height || preWidth != layoutParams.width) {
        view.requestLayout();
      }

      if (view instanceof IRenderResult) {
        WXComponent component = ((IRenderResult) view).getComponent();
        if (component != null) {
          if (preWidth != layoutParams.width || preHeight != layoutParams.height) {
            //Notify the animated component it native size has changed
            //The component will decides whether to update domobject
            component.notifyNativeSizeChanged(layoutParams.width, layoutParams.height);
          }
        }
      }
    }
  }

}
