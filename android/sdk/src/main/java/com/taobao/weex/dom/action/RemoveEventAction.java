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
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.ui.IWXRenderTask;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXExceptionUtils;

/**
 * Created by sospartan on 02/03/2017.
 */
class RemoveEventAction implements DOMAction, RenderAction {
  private final String mRef;
  private final String mEvent;

  private WXDomObject mUpdatedDomObject;

  RemoveEventAction(String ref, Object event) {
    this.mRef = ref;
    this.mEvent = WXEvent.getEventName(event);
  }


  @Override
  public void executeDom(DOMActionContext context) {
    if (context.isDestory()) {
      return;
    }
    WXSDKInstance instance = context.getInstance();
    WXDomObject domObject = context.getDomByRef(mRef);
    if (domObject == null) {
      if (instance != null) {
		WXExceptionUtils.commitCriticalExceptionRT(instance.getInstanceId(),
				WXErrorCode.WX_KEY_EXCEPTION_DOM_REMOVE_EVENT.getErrorCode(),
				"updateAttr",
				WXErrorCode.WX_KEY_EXCEPTION_DOM_REMOVE_EVENT.getErrorMsg() + "domObject is null",null);
      }
      return;
    }
    domObject.removeEvent(mEvent);
    mUpdatedDomObject = domObject;
    context.postRenderTask(this);
  }

  @Override
  public void executeRender(RenderActionContext context) {
    WXComponent comp = context.getComponent(mRef);
    if(comp != null){
      //sync dom change to component
      comp.updateDom(mUpdatedDomObject);
      comp.removeEvent(mEvent);
    }
  }
}
