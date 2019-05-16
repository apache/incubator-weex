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

import android.support.annotation.NonNull;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.support.annotation.WorkerThread;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;
import android.util.Log;
import com.taobao.weex.BuildConfig;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.dom.transition.WXTransition;
import com.taobao.weex.performance.WXAnalyzerDataTransfer;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXLogUtils;
import java.util.Arrays;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

public class GraphicActionAddElement extends GraphicActionAbstractAddElement {

  private WXVContainer parent;
  private WXComponent child;
  private GraphicPosition layoutPosition;
  private GraphicSize layoutSize;
  private boolean isLayoutRTL;

  public GraphicActionAddElement(@NonNull WXSDKInstance instance, String ref,
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

    if (instance.getContext() == null) {
      return;
    }
      if (WXAnalyzerDataTransfer.isInteractionLogOpen()){
        Log.d(WXAnalyzerDataTransfer.INTERACTION_TAG, "[client][addelementStart]"+instance.getInstanceId()+","+componentType+","+ref);
      }
    try {
      parent = (WXVContainer) WXSDKManager.getInstance().getWXRenderManager()
          .getWXComponent(getPageId(), mParentRef);
      BasicComponentData basicComponentData = new BasicComponentData(ref, mComponentType,
          mParentRef);
      child = createComponent(instance, parent, basicComponentData);
      child.setTransition(WXTransition.fromMap(child.getStyles(), child));
      if (null != parent && parent.isIgnoreInteraction){
        child.isIgnoreInteraction = true;
      }
      if (!child.isIgnoreInteraction ){
        Object flag = null;
        if (null != child.getAttrs()){
          flag = child.getAttrs().get("ignoreInteraction");
        }
        if ("false".equals(flag) || "0".equals(flag)){
          child.isIgnoreInteraction = false;
        }else if ("1".equals(flag) || "true".equals(flag) || child.isFixed()){
          child.isIgnoreInteraction = true;
        }
      }

    } catch (ClassCastException e) {
      Map<String, String> ext = new ArrayMap<>();
      WXComponent parent = WXSDKManager.getInstance().getWXRenderManager()
          .getWXComponent(getPageId(), mParentRef);

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
          "GraphicActionAddElement",
          String.format(Locale.ENGLISH,"You are trying to add a %s to a %2$s, which is illegal as %2$s is not a container",
              componentType,
              WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), mParentRef).getComponentType()),
          ext);
    }

  }

  @RestrictTo(Scope.LIBRARY)
  @WorkerThread
  public void setRTL(boolean isRTL){
    this.isLayoutRTL = isRTL;
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

  @RestrictTo(Scope.LIBRARY)
  @WorkerThread
  public void setIndex(int index){
    mIndex = index;
  }

  @Override
  public void executeAction() {
    super.executeAction();
    try {
      if (!TextUtils.equals(mComponentType, "video") && !TextUtils.equals(mComponentType, "videoplus"))
        child.mIsAddElementToTree = true;

      parent.addChild(child, mIndex);
      parent.createChildViewAt(mIndex);

      child.setIsLayoutRTL(isLayoutRTL);
      if(layoutPosition !=null && layoutSize != null) {
        child.setDemission(layoutSize, layoutPosition);
      }
      child.applyLayoutAndEvent(child);
      child.bindData(child);
    } catch (Exception e) {
      WXLogUtils.e("add component failed.", e);
    }
  }
}
