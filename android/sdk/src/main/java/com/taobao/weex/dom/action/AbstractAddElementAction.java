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


import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXLogUtils;

import java.util.List;

/**
 * Created by sospartan on 22/02/2017.
 */

public abstract class AbstractAddElementAction extends TraceableAction implements DOMAction, RenderAction {

  protected WXComponent generateComponentTree(DOMActionContext context, WXDomObject dom, WXVContainer parent) {
    if (dom == null) {
      return null;
    }
    long startNanos = System.nanoTime();
    WXComponent component = WXComponentFactory.newInstance(context.getInstance(), dom, parent);
    if (component != null) {
      component.mTraceInfo.domThreadStart = dom.mDomThreadTimestamp;
      component.mTraceInfo.rootEventId = mTracingEventId;
      component.mTraceInfo.domQueueTime = mDomQueueTime;
    }

    context.registerComponent(dom.getRef(), component);
    if (component instanceof WXVContainer) {
      WXVContainer parentC = (WXVContainer) component;
      int count = dom.childCount();
      WXDomObject child = null;
      for (int i = 0; i < count; ++i) {
        child = dom.getChild(i);
        if (child != null) {
          parentC.addChild(generateComponentTree(context, child, parentC));
        }
      }
    }
    if (component != null) {
      component.mTraceInfo.domThreadNanos = System.nanoTime() - startNanos;
    }
    return component;
  }

  /**
   * Add DOM node.
   */
  protected void addDomInternal(DOMActionContext context, JSONObject dom) {
    if (context.isDestory()) {
      return;
    }

    WXSDKInstance instance = context.getInstance();
    if (instance == null) {
      return;
    }
    String errCode = getErrorCode().getErrorCode();
	String errMsg  = getErrorMsg();

	if (dom == null) {
//      instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, errCode);
	  WXExceptionUtils.commitCriticalExceptionRT(instance.getInstanceId(), errCode, "addDomInternal", errMsg, null);
    }

    //only non-root has parent.
    Stopwatch.tick();
    WXDomObject domObject = WXDomObject.parse(dom, instance, null);
    Stopwatch.split("parseDomObject");

    if (domObject == null || context.getDomByRef(domObject.getRef()) != null) {
      WXLogUtils.e("[DOMActionContextImpl] " + getStatementName() + " error,DOM object is null or already registered!!");
//      instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, errCode);
	  WXExceptionUtils.commitCriticalExceptionRT(instance.getInstanceId(), errCode, "addDomInternal", errMsg, null);
	  return;
    }
    appendDomToTree(context, domObject);
    Stopwatch.split("appendDomToTree");

    domObject.traverseTree(
        context.getAddDOMConsumer(),
        context.getApplyStyleConsumer()
    );
    Stopwatch.split("traverseTree");


    //Create component in dom thread
    WXComponent component = createComponent(context, domObject);
    if (component == null) {
//      instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, errCode);
      //stop redner, some fatal happened.
//	  errMsg = "component == null";
//	  WXExceptionUtils.commitCriticalExceptionRT(instance.getInstanceId(), errCode, "addDomInternal", errMsg, null);
	  return;
    }
    Stopwatch.split("createComponent");

    context.addDomInfo(domObject.getRef(), component);
    context.postRenderTask(this);
    addAnimationForDomTree(context, domObject);

//    instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, WXErrorCode.WX_SUCCESS);
    if (WXTracing.isAvailable()) {
      List<Stopwatch.ProcessEvent> events = Stopwatch.getProcessEvents();
      for (Stopwatch.ProcessEvent event : events) {
        submitPerformance(event.fname, "X", context.getInstanceId(), event.duration, event.startMillis, true);
      }
    }
  }

  public void addAnimationForDomTree(DOMActionContext context, WXDomObject domObject) {
    context.addAnimationForElement(domObject.getRef(), domObject.getStyles());
    for (int i = 0; i < domObject.childCount(); i++) {
      addAnimationForDomTree(context, domObject.getChild(i));
    }
  }

  protected abstract WXComponent createComponent(DOMActionContext context, WXDomObject domObject);

  protected abstract void appendDomToTree(DOMActionContext context, WXDomObject domObject);

  protected abstract String getStatementName();

  protected abstract WXErrorCode getErrorCode();

  protected abstract String getErrorMsg();

}
