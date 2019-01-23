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
package com.taobao.weex.ui.prerenderaction;

import android.support.annotation.NonNull;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.support.annotation.WorkerThread;
import android.support.v4.util.ArrayMap;
import android.util.Log;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.performance.WXAnalyzerDataTransfer;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.node.WXComponentNode;
import com.taobao.weex.utils.WXExceptionUtils;

import java.util.Arrays;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

public class PrerenderGraphicActionAddElement extends PrerenderGraphicActionAbstractAddElement {

  private GraphicPosition layoutPosition;
  private GraphicSize layoutSize;
  private boolean isLayoutRTL;
  private WXComponentNode childNode;

  PrerenderGraphicActionAddElement(@NonNull WXSDKInstance instance, String ref,
                                          String componentType, String parentRef,
                                          int index,
                                          Map<String, String> style,
                                          Map<String, String> attributes,
                                          Set<String> events,
                                          float[] margins,
                                          float[] paddings,
                                          float[] borders) {
    super(instance, ref);
    this.mComponentType = componentType;
    this.mParentRef = parentRef;
    this.mIndex = index;
    this.mStyle = style;
    this.mAttributes = attributes;
    this.mEvents = events;
    this.mPaddings = paddings;
    this.mMargins = margins;
    this.mBorders = borders;

    if (WXAnalyzerDataTransfer.isInteractionLogOpen()){
      Log.d(WXAnalyzerDataTransfer.INTERACTION_TAG, "[client][addelementStart]"+instance.getInstanceId()+","+componentType+","+ref);
    }
    try {
      BasicComponentData basicComponentData = new BasicComponentData(ref, mComponentType,
          mParentRef);
      childNode = createNode(instance, basicComponentData)
              .setIndex(mIndex)
              .setLayoutPosition(layoutPosition)
              .setLayoutSize(layoutSize).build();
      childNode.createComponent();

    } catch (ClassCastException e) {
      Map<String, String> ext = new ArrayMap<>();
      WXComponent parent = WXSDKManager.getInstance().getWXRenderManager()
          .getWXComponent(getPageId(), mParentRef);

      if (parent != null) {
        if (mStyle != null && !mStyle.isEmpty()) {
          ext.put("child.style", mStyle.toString());
        }
        if (parent != null && parent.getStyles() != null && !parent.getStyles().isEmpty()) {
          ext.put("parent.style", parent.getStyles().toString());
        }

        if (mAttributes != null && !mAttributes.isEmpty()) {
          ext.put("child.attr", mAttributes.toString());
        }
        if (parent != null && parent.getAttrs() != null && !parent.getAttrs().isEmpty()) {
          ext.put("parent.attr", parent.getAttrs().toString());
        }

        if (mEvents != null && !mEvents.isEmpty()) {
          ext.put("child.event", mEvents.toString());
        }
        if (parent != null && parent.getEvents() != null && !parent.getEvents().isEmpty()) {
          ext.put("parent.event", parent.getEvents().toString());
        }

        if (mMargins != null && mMargins.length > 0) {
          ext.put("child.margin", Arrays.toString(mMargins));
        }
        if (parent != null && parent.getMargin() != null) {
          ext.put("parent.margin", parent.getMargin().toString());
        }

        if (mPaddings != null && mPaddings.length > 0) {
          ext.put("child.padding", Arrays.toString(mPaddings));
        }
        if (parent != null && parent.getPadding() != null) {
          ext.put("parent.padding", parent.getPadding().toString());
        }

        if (mBorders != null && mBorders.length > 0) {
          ext.put("child.border", Arrays.toString(mBorders));
        }
        if (parent != null && parent.getBorder() != null) {
          ext.put("parent.border", parent.getBorder().toString());
        }

        WXExceptionUtils.commitCriticalExceptionRT(instance.getInstanceId(),
                WXErrorCode.WX_RENDER_ERR_CONTAINER_TYPE,
                "PrerenderGraphicActionAddElement",
                String.format(Locale.ENGLISH, "You are trying to add a %s to a %2$s, which is illegal as %2$s is not a container",
                        componentType,
                        WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), mParentRef).getComponentType()),
                ext);
      }
    }

  }

  @RestrictTo(Scope.LIBRARY)
  @WorkerThread
  @Override
  public void setRTL(boolean isRTL){
    isLayoutRTL = isRTL;
    childNode.setIsLayoutRTL(isRTL);
  }

  @RestrictTo(Scope.LIBRARY)
  @WorkerThread
  @Override
  public void setSize(GraphicSize graphicSize){
    this.layoutSize = graphicSize;
    childNode.setLayoutSize(graphicSize);
  }

  @RestrictTo(Scope.LIBRARY)
  @WorkerThread
  @Override
  public void setPosition(GraphicPosition position){
    this.layoutPosition = position;
    childNode.setLayoutPosition(position);
  }

  @RestrictTo(Scope.LIBRARY)
  @WorkerThread
  @Override
  public void setIndex(int index){
    childNode.setIndex(index);
  }

  @Override
  public void executeAction() {
    super.executeAction();
    childNode.setIsLayoutRTL(isLayoutRTL);
    childNode.addElement();
  }
}
