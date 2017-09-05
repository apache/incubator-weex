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

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.ui.component.WXComponent;

import java.util.List;

/**
 * Created by sospartan on 01/03/2017.
 */
class AddEventAction extends TraceableAction implements DOMAction, RenderAction {
  private final String mRef;
  private final String mEvent;

  private WXDomObject mUpdatedDom;

  AddEventAction(String ref, String event) {
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
        instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, WXErrorCode.WX_ERR_DOM_ADDEVENT);
      }
      return;
    }
    domObject.addEvent(mEvent);
    mUpdatedDom = domObject;

    if (WXTracing.isAvailable() && mBeginEvent != null) {
      submitPerformance("addEventToDom", "X", instance.getInstanceId(), Stopwatch.tack(), Stopwatch.lastTickStamp(), true);
    }

    context.postRenderTask(this);

    if (instance != null) {
      instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, WXErrorCode.WX_SUCCESS);
    }
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
