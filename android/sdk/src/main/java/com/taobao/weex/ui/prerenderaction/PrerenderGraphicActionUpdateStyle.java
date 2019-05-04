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
package com.taobao.weex.ui.prerenderaction;

import android.support.v4.util.ArrayMap;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.dom.transition.WXTransition;
import com.taobao.weex.ui.action.BasicGraphicAction;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.node.WXComponentNode;

import java.util.Map;

public class PrerenderGraphicActionUpdateStyle extends BasicGraphicAction {

  private Map<String, Object> mStyle;
  private WXComponentNode mNode;
  private boolean mIsCausedByPesudo;
  private boolean mIsBorderSet;

  PrerenderGraphicActionUpdateStyle(WXSDKInstance instance, String ref,
                                           Map<String, Object> style,
                                           Map<String, String> paddings,
                                           Map<String, String> margins,
                                           Map<String, String> borders) {
    this(instance, ref, style, paddings, margins, borders, false);
  }

  public PrerenderGraphicActionUpdateStyle(WXSDKInstance instance, String ref,
                                           Map<String, Object> style,
                                           CSSShorthand paddings,
                                           CSSShorthand margins,
                                           CSSShorthand borders, boolean byPesudo) {
    super(instance, ref);
    this.mStyle = style;
    this.mIsCausedByPesudo = byPesudo;

    mNode = WXSDKManager.getInstance().getWXRenderManager().getWXComponentNode(getPageId(), getRef());
    if (mNode == null) {
      return;
    }
    if (null != mStyle) {
      mNode.getComponentData().getStyles().updateStyle(mStyle, mIsCausedByPesudo);
      if (!mNode.getWxInstance().getNeedInterceptRender() && mNode.data != null) {
        if (style.containsKey(Constants.Name.TRANSFORM) && mNode.data.getTransition() == null) {
          Map<String, Object> animationMap = new ArrayMap<>(2);
          animationMap.put(Constants.Name.TRANSFORM, style.get(Constants.Name.TRANSFORM));
          animationMap
                  .put(Constants.Name.TRANSFORM_ORIGIN, style.get(Constants.Name.TRANSFORM_ORIGIN));
          mNode.data.addAnimationForElement(animationMap);
        }
      }
    }

    if (null != paddings) {
      mNode.getComponentData().setPaddings(paddings);
    }

    if (null != margins) {
      mNode.getComponentData().setMargins(margins);
    }

    if (null != borders) {
      mIsBorderSet = true;
      mNode.getComponentData().setBorders(borders);
    }
  }

  public PrerenderGraphicActionUpdateStyle(WXSDKInstance instance, String ref,
                                           Map<String, Object> style,
                                           Map<String, String> paddings,
                                           Map<String, String> margins,
                                           Map<String, String> borders, boolean byPesudo) {
    super(instance, ref);
    this.mStyle = style;
    this.mIsCausedByPesudo = byPesudo;

    mNode = WXSDKManager.getInstance().getWXRenderManager().getWXComponentNode(getPageId(), getRef());
    if (mNode == null) {
      return;
    }
    if (null != mStyle) {
      mNode.getComponentData().addStyle(mStyle, mIsCausedByPesudo);
      if (!mNode.getWxInstance().getNeedInterceptRender() && mNode.data != null) {
        if (style.containsKey(Constants.Name.TRANSFORM) && mNode.data.getTransition() == null) {
          Map<String, Object> animationMap = new ArrayMap<>(2);
          animationMap.put(Constants.Name.TRANSFORM, style.get(Constants.Name.TRANSFORM));
          animationMap.put(Constants.Name.TRANSFORM_ORIGIN, style.get(Constants.Name.TRANSFORM_ORIGIN));
          mNode.data.addAnimationForElement(animationMap);
          WXBridgeManager.getInstance().markDirty(getPageId(), getRef(), true);
        }
      }
    }

    if (null != paddings) {
      mNode.getComponentData().addShorthand(paddings);
    }

    if (null != margins) {
      mNode.getComponentData().addShorthand(margins);
    }

    if (null != borders) {
      mIsBorderSet = true;
      mNode.getComponentData().addShorthand(borders);
    }
  }

  @Override
  public void executeAction() { // need merge
    mNode = WXSDKManager.getInstance().getWXRenderManager().getWXComponentNode(getPageId(), getRef());
    if (mNode == null) {
      return;
    }

    if (!mNode.getWxInstance().getNeedInterceptRender() && mNode.data != null) {
      WXComponent component = mNode.data;
      if (mStyle != null) {
        if (component.getTransition() != null) {
          component.getTransition().updateTranstionParams(mStyle);
          if (component.getTransition().hasTransitionProperty(mStyle)) {
            component.getTransition().startTransition(mStyle);
          }
        } else {
          component.setTransition(WXTransition.fromMap(mStyle, component));
          component.updateStyles(mStyle);
        }
      } else if (mIsBorderSet) {
        component.updateStyles(component);
      }
    }
  }
}

