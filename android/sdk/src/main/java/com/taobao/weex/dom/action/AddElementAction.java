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
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Created by sospartan on 22/02/2017.
 */

final class AddElementAction extends AbstractAddElementAction {
  private final String mParentRef;
  private final int mAddIndex;
  private final JSONObject mData;

  private String mRef;


  AddElementAction(JSONObject data, String parentRef, int index) {
    mParentRef = parentRef;
    mAddIndex = index;
    mData = data;
  }

  @Override
  protected WXComponent createComponent(DOMActionContext context, WXDomObject domObject) {
    WXComponent comp = context.getCompByRef(mParentRef);
    if (comp == null || !(comp instanceof WXVContainer)) {
      return null;
    }
    return generateComponentTree(context, domObject, (WXVContainer) comp);
  }

  @Override
  protected void appendDomToTree(DOMActionContext context, WXDomObject domObject) {
    WXDomObject parent;
    mRef = domObject.getRef();
    if ((parent = context.getDomByRef(mParentRef)) == null) {
      context.getInstance().commitUTStab(IWXUserTrackAdapter.DOM_MODULE, getErrorCode());
      return;
    } else {
      //non-root and parent exist
      parent.add(domObject, mAddIndex);
    }
  }

  @Override
  protected String getStatementName() {
    return "addDom";
  }

  @Override
  protected WXErrorCode getErrorCode() {
    return WXErrorCode.WX_ERR_DOM_ADDELEMENT;
  }

  @Override
  public void executeDom(DOMActionContext context) {
    addDomInternal(context, mData);
  }

  @Override
  public void executeRender(RenderActionContext context) {
    WXComponent component = context.getComponent(mRef);
    WXSDKInstance instance = context.getInstance();
    if (instance == null || instance.getContext() == null) {
      WXLogUtils.e("instance is null or instance is destroy!");
      return;
    }
    try {
      WXVContainer parent = (WXVContainer) context.getComponent(mParentRef);
      if (parent == null || component == null) {
        return;
      }

      parent.addChild(component, mAddIndex);
      parent.createChildViewAt(mAddIndex);
      component.applyLayoutAndEvent(component);
      component.bindData(component);
    } catch (Exception e) {
      WXLogUtils.e("add component failed.", e);
    }
  }
}
