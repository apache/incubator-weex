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

import android.content.Context;
import android.view.View.MeasureSpec;
import com.taobao.weex.dom.flex.CSSNode;
import com.taobao.weex.dom.flex.MeasureOutput;
import com.taobao.weex.ui.view.WXSwitchView;
import com.taobao.weex.utils.WXLogUtils;

public class WXSwitchDomObject extends WXDomObject {

  private static final MeasureFunction SWITCH_MEASURE_FUNCTION = new MeasureFunction() {

    @Override
    public void measure(CSSNode node, float width, MeasureOutput measureOutput) {
      try {
        Context context=((WXDomObject) node).getDomContext().getUIContext();
        WXSwitchView wxSwitchView = new WXSwitchView(context);
        int widthSpec, heightSpec;
        heightSpec = MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED);
        if (Float.isNaN(width)) {
          widthSpec = MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED);
        } else {
          widthSpec = MeasureSpec.makeMeasureSpec((int) width, MeasureSpec.AT_MOST);
        }
        wxSwitchView.measure(widthSpec, heightSpec);
        measureOutput.width = wxSwitchView.getMeasuredWidth();
        measureOutput.height = wxSwitchView.getMeasuredHeight();
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
