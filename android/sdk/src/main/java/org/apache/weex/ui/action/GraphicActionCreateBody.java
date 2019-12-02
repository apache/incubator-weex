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
package org.apache.weex.ui.action;

import android.support.annotation.NonNull;
import android.widget.ScrollView;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.common.WXRenderStrategy;
import org.apache.weex.dom.transition.WXTransition;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXScroller;
import org.apache.weex.utils.WXLogUtils;

import java.util.Map;
import java.util.Set;

public class GraphicActionCreateBody extends GraphicActionAbstractAddElement {

  private WXComponent component;

  public GraphicActionCreateBody(@NonNull WXSDKInstance instance, String ref,
                                 String componentType,
                                 Map<String, String> style,
                                 Map<String, String> attributes,
                                 Set<String> events,
                                 float[] margins,
                                 float[] paddings,
                                 float[] borders) {
    super(instance, ref);
    this.mComponentType = componentType;
    this.mStyle = style;
    this.mAttributes = attributes;
    this.mEvents = events;
    this.mMargins = margins;
    this.mPaddings = paddings;
    this.mBorders = borders;

    if (instance.getContext() == null) {
      return;
    }

    BasicComponentData basicComponentData = new BasicComponentData(getRef(), mComponentType, null);
    component = createComponent(instance, null, basicComponentData);
    if (component == null) {
      return;
    }
    component.setTransition(WXTransition.fromMap(component.getStyles(), component));
  }

  @Override
  public void executeAction() {
    super.executeAction();
    try {
      component.createView();
      component.applyLayoutAndEvent(component);
      component.bindData(component);
      WXSDKInstance instance = getWXSDKIntance();

      if (component instanceof WXScroller) {
        WXScroller scroller = (WXScroller) component;
        if (scroller.getInnerView() instanceof ScrollView) {
          instance.setRootScrollView((ScrollView) scroller.getInnerView());
        }
      }

      instance.onRootCreated(component);

      if (instance.getRenderStrategy() != WXRenderStrategy.APPEND_ONCE) {
        instance.onCreateFinish();
      }
    } catch (Exception e) {
      WXLogUtils.e("create body failed.", e);
    }
  }
}
