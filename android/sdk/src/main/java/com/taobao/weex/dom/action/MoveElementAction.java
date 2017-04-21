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
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;

/**
 * Created by sospartan on 01/03/2017.
 */
final class MoveElementAction implements DOMAction, RenderAction {
  private final String mRef;
  private final String mParentRef;
  private final int mIndex;

  private int mNewIndex;

  MoveElementAction(String ref, String parentRef, int index) {
    mRef = ref;
    mParentRef = parentRef;
    mIndex = index;
    mNewIndex = index;
  }

  @Override
  public void executeDom(DOMActionContext context) {
    if (context.isDestory()) {
      return;
    }
    WXSDKInstance instance = context.getInstance();
    WXDomObject domObject = context.getDomByRef(mRef);
    WXDomObject parentObject = context.getDomByRef(mParentRef);
    if (domObject == null || domObject.parent == null
        || parentObject == null || parentObject.hasNewLayout()) {
      if (instance != null) {
        instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, WXErrorCode.WX_ERR_DOM_MOVEELEMENT);
      }
      return;
    }
    int index = mIndex;
    if (domObject.parent.equals(parentObject)) {
      if(parentObject.index(domObject) == index) {
        return;
      } else if(domObject.parent.index(domObject)< index){
        index = index -1;
      }
    }

    mNewIndex = index;
    domObject.parent.remove(domObject);
    parentObject.add(domObject, mNewIndex);

    context.postRenderTask(this);
    if (instance != null) {
      instance.commitUTStab(IWXUserTrackAdapter.DOM_MODULE, WXErrorCode.WX_SUCCESS);
    }
  }

  @Override
  public void executeRender(RenderActionContext context) {
    WXComponent component = context.getComponent(mRef);
    WXComponent newParent = context.getComponent(mParentRef);
    if (component == null || component.getParent() == null
        || newParent == null || !(newParent instanceof WXVContainer)) {
      return;
    }
    WXVContainer oldParent = component.getParent();
    oldParent.remove(component,false);
    ((WXVContainer) newParent).addChild(component, mNewIndex);
  }
}
