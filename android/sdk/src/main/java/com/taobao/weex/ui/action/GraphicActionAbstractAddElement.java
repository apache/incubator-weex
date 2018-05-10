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
package com.taobao.weex.ui.action;

import android.support.v4.util.ArrayMap;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXVContainer;
import java.util.Map;
import java.util.Set;

public abstract class GraphicActionAbstractAddElement extends BasicGraphicAction {

  protected String mComponentType;
  protected String mParentRef;
  protected int mIndex = -1;
  protected Map<String, String> mStyle;
  protected Map<String, String> mAttributes;
  protected Set<String> mEvents;
  protected float[] mMargins;
  protected float[] mPaddings;
  protected float[] mBorders;
  private long startTime;

  public GraphicActionAbstractAddElement(String pageId, String ref) {
    super(pageId, ref);
    startTime = System.currentTimeMillis();
  }

  protected WXComponent createComponent(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    long createComponentStart = System.currentTimeMillis();
    if (basicComponentData != null) {
      basicComponentData.addStyle(mStyle);
      basicComponentData.addAttr(mAttributes);
      basicComponentData.addEvent(mEvents);
      basicComponentData.addShorthand(mMargins, CSSShorthand.TYPE.MARGIN);
      basicComponentData.addShorthand(mPaddings, CSSShorthand.TYPE.PADDING);
      basicComponentData.addShorthand(mBorders, CSSShorthand.TYPE.BORDER);
    }

    WXComponent component = WXComponentFactory.newInstance(instance, parent, basicComponentData);
    WXSDKManager.getInstance().getWXRenderManager().registerComponent(getPageId(), getRef(), component);
    if(mStyle.containsKey(Constants.Name.TRANSFORM) && component.getTransition() == null) {
      Map<String, Object> animationMap = new ArrayMap<>(2);
      animationMap.put(Constants.Name.TRANSFORM, mStyle.get(Constants.Name.TRANSFORM));
      animationMap
          .put(Constants.Name.TRANSFORM_ORIGIN, mStyle.get(Constants.Name.TRANSFORM_ORIGIN));
      component.addAnimationForElement(animationMap);
    }
    instance.onComponentCreate(component,System.currentTimeMillis() -createComponentStart);
    return component;
  }

  @Override
  public void executeAction() {
    WXSDKManager.getInstance().getSDKInstance(getPageId()).callActionAddElementTime(System.currentTimeMillis() - startTime);
  }

  public String getComponentType() {
    return mComponentType;
  }

  public String getParentRef() {
    return mParentRef;
  }

  public int getIndex() {
    return mIndex;
  }

  public Map<String, String> getStyle() {
    return mStyle;
  }

  public Map<String, String> getAttributes() {
    return mAttributes;
  }

  public Set<String> getEvents() {
    return mEvents;
  }
}
