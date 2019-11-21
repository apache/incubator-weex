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
package org.apache.weex.ui.component.basic;

import android.support.annotation.NonNull;
import android.view.View;
import org.apache.weex.dom.CSSShorthand;
import org.apache.weex.dom.WXAttr;
import org.apache.weex.dom.WXEvent;
import org.apache.weex.dom.WXStyle;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.action.GraphicPosition;
import org.apache.weex.ui.action.GraphicSize;
import org.apache.weex.ui.component.WXComponent;
import java.util.Map;
import java.util.Set;

public abstract class WXBasicComponent<T extends View> {

  private Object mExtra;
  private String mComponentType;
  private String mRef;
  private GraphicPosition mLayoutPosition;
  private GraphicSize mLayoutSize;
  private boolean mIsLayoutRTL;
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

  protected void bindComponent(WXComponent component) {
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

  public final void updateStyle(Map<String, Object> styles, boolean byPesudo){
    if (styles == null || styles.isEmpty()) {
      return;
    }
    mBasicComponentData.getStyles().updateStyle(styles, byPesudo);
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

  public void setIsLayoutRTL(boolean isRTL) {
    mIsLayoutRTL = isRTL;
  }

  public boolean isLayoutRTL() {
    return mIsLayoutRTL;
  }

  public GraphicPosition getLayoutPosition() {
    if (mLayoutPosition == null) {
      mLayoutPosition = new GraphicPosition(0, 0, 0, 0);
    }
    return mLayoutPosition;
  }

  protected void setLayoutPosition(GraphicPosition mLayoutPosition) {
    this.mLayoutPosition = mLayoutPosition;
  }

  public GraphicSize getLayoutSize() {
    if (mLayoutSize == null) {
      mLayoutSize = new GraphicSize(0, 0);
    }
    return mLayoutSize;
  }

  protected void setLayoutSize(GraphicSize mLayoutSize) {
    this.mLayoutSize = mLayoutSize;
  }

  public float getCSSLayoutTop() {
    return mLayoutPosition == null ? 0 : mLayoutPosition.getTop();
  }

  public float getCSSLayoutBottom() {
    return mLayoutPosition == null ? 0 : mLayoutPosition.getBottom();
  }

  public float getCSSLayoutLeft() {
    return mLayoutPosition == null ? 0 : mLayoutPosition.getLeft();
  }

  public float getCSSLayoutRight() {
    return mLayoutPosition == null ? 0 : mLayoutPosition.getRight();
  }

  public float getLayoutWidth() {
    return mLayoutSize == null ? 0 : mLayoutSize.getWidth();
  }

  public float getLayoutHeight() {
    return mLayoutSize == null ? 0 : mLayoutSize.getHeight();
  }
}
