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

import android.support.v4.util.ArrayMap;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.RenderActionContext;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXExceptionUtils;

import java.util.Map;

/**
 * Created by sospartan on 28/02/2017.
 */

class UpdateStyleAction extends TraceableAction implements DOMAction, RenderAction {
  private final String mRef;
  private final JSONObject mData;
  private final boolean mIsCausedByPesudo;

  private Spacing mPadding;
  private Spacing mBorder;

  UpdateStyleAction(String ref, JSONObject data) {
    this(ref, data, false);
  }

  UpdateStyleAction(String ref, JSONObject data, boolean byPesudo) {
    this.mRef = ref;
    this.mData = data;
    this.mIsCausedByPesudo = byPesudo;
  }

  @Override
  public void executeDom(DOMActionContext context) {
    if (context.isDestory() || mData == null) {
      return;
    }
    WXSDKInstance instance = context.getInstance();
    WXDomObject domObject = context.getDomByRef(mRef);
    if (domObject == null) {
      if (instance != null) {
		WXExceptionUtils.commitCriticalExceptionRT(instance.getInstanceId(),
				WXErrorCode.WX_KEY_EXCEPTION_DOM_UPDATE_STYLE.getErrorCode(),
				"updateStyle",
				WXErrorCode.WX_KEY_EXCEPTION_DOM_UPDATE_STYLE.getErrorMsg() + "domObject is null",null);
      }
      return;
    }
    mPadding = domObject.getPadding();
    mBorder = domObject.getBorder();

    if(mData.get(WXDomObject.TRANSFORM) != null || mData.get(WXDomObject.TRANSFORM_ORIGIN) != null){
      if(domObject.getTransition() == null) {
        Map<String, Object> animationMap = new ArrayMap<>(2);
        animationMap.put(WXDomObject.TRANSFORM, mData.get(WXDomObject.TRANSFORM));
        animationMap.put(WXDomObject.TRANSFORM_ORIGIN, mData.get(WXDomObject.TRANSFORM_ORIGIN));
        context.addAnimationForElement(mRef, animationMap);
      }
    }


    if (!mData.isEmpty()) {
      domObject.updateStyle(mData, mIsCausedByPesudo);
      domObject.applyStyle(mData);
      if(!mData.isEmpty()) {
        context.postRenderTask(this);
      }
    }
  }

  @Override
  public void executeRender(RenderActionContext context) {
    WXComponent component = context.getComponent(mRef);
    if (component == null) {
      return;
    }
    component.updateProperties(mData);

    if (mData.containsKey(Constants.Name.PADDING) ||
        mData.containsKey(Constants.Name.PADDING_TOP) ||
        mData.containsKey(Constants.Name.PADDING_LEFT) ||
        mData.containsKey(Constants.Name.PADDING_RIGHT) ||
        mData.containsKey(Constants.Name.PADDING_BOTTOM) ||
        mData.containsKey(Constants.Name.BORDER_WIDTH)) {
      Spacing padding = mPadding;
      Spacing border = mBorder;
      component.setPadding(padding, border);
    }
  }
}
