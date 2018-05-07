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

import android.support.v4.util.ArrayMap;

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.dom.transition.WXTransition;
import com.taobao.weex.ui.component.WXComponent;

import java.util.Map;

public class GraphicActionUpdateStyle extends BasicGraphicAction {

  private Map<String, Object> mStyle;
  private WXComponent component;
  private boolean mIsCausedByPesudo;

  public GraphicActionUpdateStyle(String pageId, String ref,
                                  Map<String, Object> style,
                                  Map<String, String> paddings,
                                  Map<String, String> margins,
                                  Map<String, String> borders) {
    this(pageId, ref, style, paddings, margins, borders, false);
  }

  public GraphicActionUpdateStyle(String pageId, String ref,
                                  Map<String, Object> style,
                                  CSSShorthand paddings,
                                  CSSShorthand margins,
                                  CSSShorthand borders, boolean byPesudo) {
    super(pageId, ref);
    this.mStyle = style;
    this.mIsCausedByPesudo = byPesudo;

    component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), getRef());
    if (component == null) {
      return;
    }
    if (null != mStyle) {
      component.updateStyle(mStyle, mIsCausedByPesudo);
      if(style.containsKey(Constants.Name.TRANSFORM) && component.getTransition() == null) {
        Map<String, Object> animationMap = new ArrayMap<>(2);
        animationMap.put(Constants.Name.TRANSFORM, style.get(Constants.Name.TRANSFORM));
        animationMap
            .put(Constants.Name.TRANSFORM_ORIGIN, style.get(Constants.Name.TRANSFORM_ORIGIN));
        component.addAnimationForElement(animationMap);
      }
    }

    if (null != paddings) {
      component.setPaddings(paddings);
    }

    if (null != margins) {
      component.setMargins(margins);
    }

    if (null != borders) {
      component.setBorders(borders);
    }
  }

  public GraphicActionUpdateStyle(String pageId, String ref,
                                  Map<String, Object> style,
                                  Map<String, String> paddings,
                                  Map<String, String> margins,
                                  Map<String, String> borders, boolean byPesudo) {
    super(pageId, ref);
    this.mStyle = style;
    this.mIsCausedByPesudo = byPesudo;

    component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getPageId(), getRef());
    if (component == null) {
      return;
    }
    if (null != mStyle) {
      component.addStyle(mStyle, mIsCausedByPesudo);
      if(style.containsKey(Constants.Name.TRANSFORM) && component.getTransition() == null){
        Map<String, Object> animationMap = new ArrayMap<>(2);
        animationMap.put(Constants.Name.TRANSFORM, style.get(Constants.Name.TRANSFORM));
        animationMap.put(Constants.Name.TRANSFORM_ORIGIN, style.get(Constants.Name.TRANSFORM_ORIGIN));
        component.addAnimationForElement(animationMap);
        WXBridgeManager.getInstance().markDirty(component.getInstanceId(), component.getRef(), true);
      }
    }

    if (null != paddings) {
      component.addShorthand(paddings);
    }

    if (null != margins) {
      component.addShorthand(margins);
    }

    if (null != borders) {
      component.addShorthand(borders);
    }
  }

  @Override
  public void executeAction() {
    if (component == null) {
      return;
    }
    if(component.getTransition() != null){
      component.getTransition().updateTranstionParams(mStyle);
      if(component.getTransition().hasTransitionProperty(mStyle)){
        component.getTransition().startTransition(mStyle);
      }
    } else {
      component.setTransition(WXTransition.fromMap(mStyle, component));
      component.updateStyles(mStyle);
    }
  }
}

