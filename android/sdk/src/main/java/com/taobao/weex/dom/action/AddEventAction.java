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
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXExceptionUtils;

import java.util.List;

/**
 * Created by sospartan on 01/03/2017.
 */
class AddEventAction extends TraceableAction implements DOMAction, RenderAction {
  private final String mRef;
  private final Object mEvent;

  private WXDomObject mUpdatedDom;

  AddEventAction(String ref, Object event) {
    mRef = ref;
    mEvent = event;
  }

  @Override
  public void executeDom(DOMActionContext context) {
    if (context.isDestory()) {
      return;
    }

    Stopwatch.tick();
    WXSDKInstance instance = context.getInstance();
    WXDomObject domObject = context.getDomByRef(mRef);
    if (domObject == null) {
      if (instance != null) {
		String event = JSONObject.toJSONString(mEvent);
		StringBuilder sbErr = new StringBuilder()
				.append("|mRef==" + mRef)
				.append("|mEvent=" + event)
				.append("|instance id = " + instance.getInstanceId())
				.append("|context = " + context.toString());

		if(!"_documentElement".equals(mRef)){//Rax framework
		  WXExceptionUtils.commitCriticalExceptionRT(instance.getInstanceId(),
				  WXErrorCode.WX_KEY_EXCEPTION_DOM_ADD_EVENT.getErrorCode(),
				  "addEvent",
				  WXErrorCode.WX_KEY_EXCEPTION_DOM_ADD_EVENT.getErrorMsg() + "| domObject is null |"
						  +sbErr.toString(),null);
		}
      }
      return;
    }

    domObject.getEvents().addEvent(mEvent);
    mUpdatedDom = domObject;
    if (WXTracing.isAvailable() && mBeginEvent != null) {
      submitPerformance("addEventToDom", "X", instance.getInstanceId(), Stopwatch.tack(), Stopwatch.lastTickStamp(), true);
    }

    context.postRenderTask(this);
  }

  @Override
  public void executeRender(RenderActionContext context) {
    WXComponent comp = context.getComponent(mRef);
    if(comp != null){
      //sync dom change to component
      Stopwatch.tick();
      comp.updateDom(mUpdatedDom);
      Stopwatch.split("updateDom");
      comp.addEvent(mEvent);
      Stopwatch.split("addEventToComponent");

      if (WXTracing.isAvailable() && mBeginEvent != null) {
        List<Stopwatch.ProcessEvent> events = Stopwatch.getProcessEvents();
        for (Stopwatch.ProcessEvent event : events) {
          submitPerformance(event.fname, "X", comp.getInstanceId(), event.duration, event.startMillis, true);
        }
      }
    }
  }
}
