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
package com.taobao.weex.ui.component.basic;

import android.support.annotation.NonNull;
import android.view.View;

import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXUtils;

import java.util.Map;
import java.util.Set;

public abstract class WXBasicComponent<T extends View> {

  private Object mExtra;
  private String mComponentType;
  private String mRef;
  private GraphicPosition mLayoutPosition = new GraphicPosition(0, 0, 0, 0);
  private GraphicSize mLayoutSize = new GraphicSize(0, 0);
  private BasicComponentData mBasicComponentData;

  private int mViewPortWidth = 750;

  public WXBasicComponent(BasicComponentData basicComponentData) {
    this.mBasicComponentData = basicComponentData;
    this.mRef = basicComponentData.mRef;
    this.mComponentType = basicComponentData.mComponentType;
  }

  public BasicComponentData getBasicComponentData() {
    return mBasicComponentData;
  }

  protected void copyData(WXComponent component) {
    mComponentType = component.getComponentType();
    mRef = component.getRef();
  }

  public final @NonNull
  WXStyle getStyles() {
    return mBasicComponentData.getStyles();
  }

  public final @NonNull
  WXAttr getAttrs() {
    return mBasicComponentData.getAttrs();
  }

  public final @NonNull
  WXEvent getEvents() {
    return mBasicComponentData.getEvents();
  }

  /**
   * Get this node's margin, as defined by cssstyle + default margin.
   */
  public final @NonNull
  CSSShorthand getMargin() {
    return mBasicComponentData.getMargin();
  }

  /**
   * Get this node's padding, as defined by cssstyle + default padding.
   */
  public final @NonNull
  CSSShorthand getPadding() {
    return mBasicComponentData.getPadding();
  }

  /**
   * Get this node's border, as defined by cssstyle.
   */
  public @NonNull
  CSSShorthand getBorder() {
    return mBasicComponentData.getBorder();
  }

  public final void setMargins(@NonNull CSSShorthand margins) {
    mBasicComponentData.setMargins(margins);
  }

  public final void setPaddings(@NonNull CSSShorthand paddings) {
    mBasicComponentData.setPaddings(paddings);
  }

  public final void setBorders(@NonNull CSSShorthand borders) {
    mBasicComponentData.setBorders(borders);
  }

  public final void addAttr(Map<String, Object> attrs) {
    if (attrs == null || attrs.isEmpty()) {
      return;
    }
    mBasicComponentData.addAttr(attrs);
  }

  public final void addStyle(Map<String, Object> styles) {
    if (styles == null || styles.isEmpty()) {
      return;
    }
    mBasicComponentData.addStyle(styles);
  }

  public final void addStyle(Map<String, Object> styles, boolean byPesudo) {
    if (styles == null || styles.isEmpty()) {
      return;
    }
    mBasicComponentData.addStyle(styles, byPesudo);
  }

  public final void addEvent(Set<String> events) {
    if (events == null || events.isEmpty()) {
      return;
    }

    mBasicComponentData.addEvent(events);
  }

  public final void addShorthand(Map<String, String> shorthand) {
    if (!shorthand.isEmpty() && mBasicComponentData != null) {
      mBasicComponentData.addShorthand(shorthand);
    }
  }

  public int getViewPortWidth() {
    return mViewPortWidth;
  }

  public void setViewPortWidth(int mViewPortWidth) {
    this.mViewPortWidth = mViewPortWidth;
  }

  public Object getExtra() {
    return mExtra;
  }

  public void updateExtra(Object extra) {
    this.mExtra = extra;
  }

  public String getComponentType() {
    return mComponentType;
  }

  public String getRef() {
    return mRef;
  }

  protected GraphicPosition getLayoutPosition() {
    return mLayoutPosition;
  }

  protected void setLayoutPosition(GraphicPosition mLayoutPosition) {
    this.mLayoutPosition = mLayoutPosition;
  }

  protected GraphicSize getLayoutSize() {
    return mLayoutSize;
  }

  protected void setLayoutSize(GraphicSize mLayoutSize) {
    this.mLayoutSize = mLayoutSize;
  }

  public float getCSSLayoutTop() {
    return mLayoutPosition.getTop();
  }

  public float getCSSLayoutBottom() {
    return mLayoutPosition.getBottom();
  }

  public float getCSSLayoutLeft() {
    return mLayoutPosition.getLeft();
  }

  public float getCSSLayoutRight() {
    return mLayoutPosition.getRight();
  }

  public float getLayoutWidth() {
    return mLayoutSize == null ? 0 : mLayoutSize.getWidth();
  }

  public float getLayoutHeight() {
    return mLayoutSize == null ? 0 : mLayoutSize.getHeight();
  }

  public float getMinWidth() {
    return WXUtils.getFloat(mBasicComponentData.getStyles().get(Constants.Name.MIN_WIDTH));
  }

  public float getMaxWidth() {
    return WXUtils.getFloat(mBasicComponentData.getStyles().get(Constants.Name.MAX_WIDTH));
  }

  public float getMinHeight() {
    return WXUtils.getFloat(mBasicComponentData.getStyles().get(Constants.Name.MIN_HEIGHT));
  }

  public float getMaxHeight() {
    return WXUtils.getFloat(mBasicComponentData.getStyles().get(Constants.Name.MAX_HEIGHT));
  }
}
