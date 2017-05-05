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


import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.util.Property;
import android.view.View;

import com.taobao.weex.ui.view.border.BorderDrawable;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

public class BackgroundColorProperty extends Property<View, Integer> {

  private final static String TAG = "BackgroundColorAnimation";

  public BackgroundColorProperty() {
    super(Integer.class, WXAnimationBean.Style.BACKGROUND_COLOR);
  }

  @Override
  public Integer get(View object) {
    int color;
    BorderDrawable borderDrawable;
    if ((borderDrawable = WXViewUtils.getBorderDrawable(object)) != null) {
      color = borderDrawable.getColor();
    } else if (object.getBackground() instanceof ColorDrawable) {
      color = ((ColorDrawable) object.getBackground()).getColor();
    } else {
      color = Color.TRANSPARENT;
      WXLogUtils.e(TAG, "Unsupported background type");
    }
    return color;
  }

  @Override
  public void set(View object, Integer value) {
    BorderDrawable borderDrawable;
    if ((borderDrawable = WXViewUtils.getBorderDrawable(object)) != null) {
      borderDrawable.setColor(value);
    } else if (object.getBackground() instanceof ColorDrawable) {
      ((ColorDrawable) object.getBackground()).setColor(value);
    } else {
      WXLogUtils.e(TAG, "Unsupported background type");
    }
  }
}
