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
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Created by sospartan on 22/02/2017.
 */

abstract class AbstractAddElementAction implements DOMAction, RenderAction {


  protected WXComponent generateComponentTree(DOMActionContext context, WXDomObject dom, WXVContainer parent) {
    if (dom == null) {
      return null;
    }
    WXComponent component = WXComponentFactory.newInstance(context.getInstance(), dom, parent);

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
    WXErrorCode errCode = getErrorCode();
    if (dom == null) {
      instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, errCode);
    }

    //only non-root has parent.
    WXDomObject domObject = WXDomObject.parse(dom, instance);

    if (domObject == null || context.getDomByRef(domObject.getRef()) != null) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e("[DOMActionContextImpl] " + getStatementName() + " error,DOM object is null or already registered!!");
      }
      instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, errCode);
      return;
    }
    appendDomToTree(context, domObject);

    domObject.traverseTree(
        context.getAddDOMConsumer(),
        context.getApplyStyleConsumer()
    );

    //Create component in dom thread
    WXComponent component = createComponent(context, domObject);
    if (component == null) {
      instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, errCode);
      //stop redner, some fatal happened.
      return;
    }
    context.addDomInfo(domObject.getRef(), component);
    context.postRenderTask(this);
    addAnimationForDomTree(context, domObject);

    instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, WXErrorCode.WX_SUCCESS);
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
}
