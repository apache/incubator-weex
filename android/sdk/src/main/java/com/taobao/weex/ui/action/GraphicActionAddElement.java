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

import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.support.annotation.WorkerThread;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.dom.WXTransition;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

public class GraphicActionAddElement extends GraphicActionAbstractAddElement {

  private WXVContainer parent;
  private WXComponent child;
  private GraphicPosition layoutPosition;
  private GraphicSize layoutSize;

  public GraphicActionAddElement(String pageId, String ref,
                                 String componentType, String parentRef,
                                 int index,
                                 Map<String, String> style,
                                 Map<String, String> attributes,
                                 Set<String> events,
                                 HashMap<String, String> paddings,
                                 HashMap<String, String> margins,
                                 HashMap<String, String> borders) {
    super(pageId, ref);
    this.mComponentType = componentType;
    this.mParentRef = parentRef;
    this.mIndex = index;
    this.mStyle = style;
    this.mAttributes = attributes;
    this.mEvents = events;
    this.mPaddings = paddings;
    this.mMargins = margins;
    this.mBorders = borders;

    WXSDKInstance instance = WXSDKManager.getInstance().getWXRenderManager().getWXSDKInstance(getPageId());
    if (instance == null || instance.getContext() == null) {
      return;
    }

    parent = (WXVContainer) WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), mParentRef);
    BasicComponentData basicComponentData = new BasicComponentData(getRef(), getComponentType(), getParentRef());
    child = createComponent(instance, parent, basicComponentData);
    child.setTransition(WXTransition.fromMap(child.getStyles(), child));

    if (child == null || parent == null) {
      return;
    }

    WXSDKManager.getInstance().getSDKInstance(getPageId()).nativeBindComponentToWXCore(getPageId(), child, getRef());
  }

  @RestrictTo(Scope.LIBRARY)
  @WorkerThread
  public void setSize(GraphicSize graphicSize){
    this.layoutSize = graphicSize;
  }

  @RestrictTo(Scope.LIBRARY)
  @WorkerThread
  public void setPosition(GraphicPosition position){
    this.layoutPosition = position;
  }

  @Override
  public void executeAction() {
    try {
      parent.addChild(child, mIndex);
      parent.createChildViewAt(mIndex);

      long start = System.currentTimeMillis();
      child.updateDemission(layoutSize, layoutPosition);
      child.applyLayoutAndEvent(child);
      WXSDKManager.getInstance().getSDKInstance(getPageId()).callLayoutaAplyLayoutAndEventTime(System.currentTimeMillis() - start);

      start = System.currentTimeMillis();
      child.bindData(child);
      WXSDKManager.getInstance().getSDKInstance(getPageId()).callLayoutBindDataCoreTime(System.currentTimeMillis() - start);
    } catch (Exception e) {
      WXLogUtils.e("add component failed.", e);
    }
  }
}
