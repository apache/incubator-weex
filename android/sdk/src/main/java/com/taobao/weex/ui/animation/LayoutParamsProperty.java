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
package com.taobao.weex.ui.animation;


import android.util.Property;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.view.IRenderResult;

/**
 * android.util.IntProperty<T> cannot be applied here, as it is only added at API 24.
 */
abstract class LayoutParamsProperty extends Property<View, Integer> {

  LayoutParamsProperty() {
    super(Integer.class, "layoutParams");
  }

  @Override
  public Integer get(View object) {
    LayoutParams layoutParams;
    if (object != null && (layoutParams = object.getLayoutParams()) != null) {
      return getProperty(layoutParams);
    }
    return 0;
  }

  @Override
  public void set(View object, Integer value) {
    LayoutParams layoutParams;
    if (object != null && (layoutParams = object.getLayoutParams()) != null) {
      setProperty(layoutParams, value);
      if (object instanceof IRenderResult) {
        WXComponent component = ((IRenderResult) object).getComponent();
        if (component != null) {
            component.notifyNativeSizeChanged(layoutParams.width, layoutParams.height);
        }
      }
      object.requestLayout();
    }
  }

  protected abstract Integer getProperty(LayoutParams layoutParams);

  protected abstract void setProperty(LayoutParams layoutParams, Integer expected);
}
