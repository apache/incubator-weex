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
package com.taobao.weex.ui.component.richtext;


import android.content.Context;
import android.support.annotation.NonNull;
import android.text.Spannable;
import android.text.Spanned;
import android.text.SpannedString;
import android.text.TextUtils;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.layout.measurefunc.TextContentBoxMeasurement;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXText;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.component.richtext.node.RichTextNode;

import java.lang.reflect.InvocationTargetException;

public class WXRichText extends WXText {

  static class RichTextContentBoxMeasurement extends TextContentBoxMeasurement {

    public RichTextContentBoxMeasurement(WXComponent component) {
      super(component);
    }

    @NonNull
    @Override
    protected Spanned createSpanned(String text) {
      if (mComponent.getInstance() != null & mComponent.getInstance().getUIContext() != null &&
              !TextUtils.isEmpty(mComponent.getInstanceId())) {
        Spannable spannable = RichTextNode.parse(
                mComponent.getInstance().getUIContext(),
                mComponent.getInstanceId(),
                mComponent.getRef(),
                text);
        updateSpannable(spannable, RichTextNode.createSpanFlag(0));
        return spannable;
      } else {
        return new SpannedString("");
      }
    }
  }

  public static class Creator implements ComponentCreator {

    public WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      return new WXRichText(instance, parent, basicComponentData);
    }
  }

  public WXRichText(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
    setContentBoxMeasurement(new RichTextContentBoxMeasurement(this));
  }

  @Override
  protected WXRichTextView initComponentHostView(@NonNull Context context) {
    return new WXRichTextView(context);
  }
}
