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
package com.taobao.weex.dom;

import android.view.View;

import com.taobao.weex.dom.flex.CSSNode;
import com.taobao.weex.dom.flex.MeasureOutput;
import com.taobao.weex.ui.view.WXSwitchView;
import com.taobao.weex.utils.WXLogUtils;

public class WXSwitchDomObject extends WXDomObject {

  private static final MeasureFunction SWITCH_MEASURE_FUNCTION = new MeasureFunction() {

    private boolean measured;
    private int mWidth;
    private int mHeight;

    @Override
    public void measure(CSSNode node, float width, MeasureOutput measureOutput) {
      try {
        if (!measured) {
          WXSwitchView wxSwitchView = new WXSwitchView(((WXDomObject) node).getDomContext().getUIContext());
          int heightSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
          int widthSpec = View.MeasureSpec.makeMeasureSpec((int) width, View.MeasureSpec.AT_MOST);
          wxSwitchView.measure(widthSpec, heightSpec);
          mWidth = wxSwitchView.getMeasuredWidth();
          mHeight = wxSwitchView.getMeasuredHeight();
          measured = true;
        }
        measureOutput.width = mWidth;
        measureOutput.height = mHeight;
      } catch (RuntimeException e) {
        WXLogUtils.e(TAG, WXLogUtils.getStackTrace(e));
      }
    }
  };

  public WXSwitchDomObject() {
    super();
    setMeasureFunction(SWITCH_MEASURE_FUNCTION);
  }

}
