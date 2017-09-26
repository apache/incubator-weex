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
package com.taobao.weex.dom.action;

import android.widget.ScrollView;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXScroller;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

/**
 * Created by sospartan on 14/02/2017.
 */

class CreateBodyAction extends AbstractAddElementAction {
  private final JSONObject mData;

  CreateBodyAction(JSONObject data) {
    mData = data;
  }

  @Override
  public void executeDom(DOMActionContext context) {
    if (WXTracing.isAvailable()) {
      if (context != null && context.getInstance() != null) {
        WXTracing.TraceEvent execJsEndEvent = WXTracing.newEvent("executeBundleJS", context.getInstanceId(), -1);
        execJsEndEvent.traceId = context.getInstance().mExecJSTraceId;
        execJsEndEvent.ph = "E";
        execJsEndEvent.submit();
      }
    }
    addDomInternal(context, mData);
  }

  @Override
  protected WXComponent createComponent(DOMActionContext context, WXDomObject domObject) {
    return generateComponentTree(context, domObject, null);
  }

  @Override
  protected void appendDomToTree(DOMActionContext context, WXDomObject domObject) {
    long startNanos = System.nanoTime();
    String instanceId = context.getInstanceId();
    WXDomObject.prepareRoot(domObject,
        WXViewUtils.getWebPxByWidth(WXViewUtils.getWeexHeight(instanceId), WXSDKManager.getInstanceViewPortWidth(instanceId)),
        WXViewUtils.getWebPxByWidth(WXViewUtils.getWeexWidth(instanceId), WXSDKManager.getInstanceViewPortWidth(instanceId)));
    domObject.mDomThreadNanos += (System.nanoTime() - startNanos);
  }

  @Override
  protected WXErrorCode getErrorCode() {
    return WXErrorCode.WX_ERR_DOM_CREATEBODY;
  }

  @Override
  protected String getStatementName() {
    return "createBody";
  }


  @Override
  public void executeRender(RenderActionContext context) {
    WXComponent component = context.getComponent(WXDomObject.ROOT);
    WXSDKInstance instance = context.getInstance();
    if (instance == null || instance.getContext() == null) {
      WXLogUtils.e("instance is null or instance is destroy!");
      return;
    }
    try {
      Stopwatch.tick();
      long start = System.currentTimeMillis();
      component.createView();
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.renderPerformanceLog("createView", (System.currentTimeMillis() - start));
        submitPerformance("createView", "X", instance.getInstanceId(), Stopwatch.tackAndTick(), start, true);
      }
      start = System.currentTimeMillis();
      component.applyLayoutAndEvent(component);
      if (WXTracing.isAvailable()) {
        submitPerformance("applyLayoutAndEvent", "X", instance.getInstanceId(), Stopwatch.tackAndTick(), start, true);
      }
      start = System.currentTimeMillis();
      component.bindData(component);

      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.renderPerformanceLog("bind", (System.currentTimeMillis() - start));
        submitPerformance("bindData", "X", instance.getInstanceId(), Stopwatch.tack(), start, true);
      }

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
      instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, WXErrorCode.WX_SUCCESS);
      component.mTraceInfo.uiQueueTime = mUIQueueTime;
      component.onRenderFinish(WXComponent.STATE_ALL_FINISH);
    } catch (Exception e) {
      WXLogUtils.e("create body failed.", e);
    }
  }

}
