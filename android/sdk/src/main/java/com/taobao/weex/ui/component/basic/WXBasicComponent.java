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

  private WXStyle mStyles;
  private WXAttr mAttributes;
  private WXEvent mEvents;
  private CSSShorthand mMargins;
  private CSSShorthand mPaddings;
  private CSSShorthand mBorders;

  private Object mExtra;
  private String mComponentType;
  private String mRef;
  private GraphicPosition mLayoutPosition = new GraphicPosition(0, 0, 0, 0);
  private GraphicSize mLayoutSize = new GraphicSize(0, 0);

  private int mViewPortWidth =750;

  public WXBasicComponent(BasicComponentData basicComponentData) {
    this.mRef = basicComponentData.mRef;
    this.mComponentType = basicComponentData.mComponentType;
  }

  protected void copyData(WXComponent component) {
    mComponentType = component.getComponentType();
    mRef = component.getRef();
  }

  public final @NonNull
  WXStyle getStyles() {
    if (mStyles == null) {
      mStyles = new WXStyle();
    }
    return mStyles;
  }

  public final @NonNull
  WXAttr getAttrs() {
    if (mAttributes == null) {
      mAttributes = new WXAttr();
    }
    return mAttributes;
  }

  public final @NonNull
  WXEvent getEvents() {
    if (mEvents == null) {
      mEvents = new WXEvent();
    }
    return mEvents;
  }

  /**
   * Get this node's margin, as defined by cssstyle + default margin.
   */
  public final @NonNull
  CSSShorthand getMargin() {
    if (mMargins == null) {
      mMargins = new CSSShorthand();
    }
    return mMargins;
  }

  /**
   * Get this node's padding, as defined by cssstyle + default padding.
   */
  public final @NonNull
  CSSShorthand getPadding() {
    if (mPaddings == null) {
      mPaddings = new CSSShorthand();
    }
    return mPaddings;
  }

  /**
   * Get this node's border, as defined by cssstyle.
   */
  public @NonNull
  CSSShorthand getBorder() {
    if (mBorders == null) {
      mBorders = new CSSShorthand();
    }
    return mBorders;
  }

  public final void setMargins(@NonNull CSSShorthand mMargins) {
    this.mMargins = mMargins;
  }

  public final void setPaddings(@NonNull CSSShorthand mPaddings) {
    this.mPaddings = mPaddings;
  }

  public final void setBorders(@NonNull CSSShorthand mBorders) {
    this.mBorders = mBorders;
  }

  public final void addAttr(Map<String, Object> attrs) {
    if (attrs == null || attrs.isEmpty()) {
      return;
    }
    if (mAttributes == null) {
      mAttributes = new WXAttr();
    }
    mAttributes.putAll(attrs);
  }

  public final void addStyle(Map<String, Object> styles) {
    if (styles == null || styles.isEmpty()) {
      return;
    }
    if (mStyles == null) {
      mStyles = new WXStyle();
    }
    addStyle(styles, false);
  }

  public final void addStyle(Map<String, Object> styles, boolean byPesudo) {
    if (styles == null || styles.isEmpty()) {
      return;
    }
    if (mStyles == null) {
      mStyles = new WXStyle();
    }
    mStyles.putAll(styles, byPesudo);
  }

  public final void addEvent(Set<String> events) {
    if (events == null || events.isEmpty()) {
      return;
    }
    if (mEvents == null) {
      mEvents = new WXEvent();
    }
    mEvents.addAll(events);
  }

  public final void addShorthand(Map<String, String> shorthand) {
    if (!shorthand.isEmpty()) {
      for (Map.Entry<String, String> item : shorthand.entrySet()) {
        String key = item.getKey();
        switch (key) {
          case Constants.Name.MARGIN:
            addMargin(CSSShorthand.EDGE.ALL, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.MARGIN_LEFT:
            addMargin(CSSShorthand.EDGE.LEFT, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.MARGIN_TOP:
            addMargin(CSSShorthand.EDGE.TOP, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.MARGIN_RIGHT:
            addMargin(CSSShorthand.EDGE.RIGHT, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.MARGIN_BOTTOM:
            addMargin(CSSShorthand.EDGE.BOTTOM,WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.BORDER_WIDTH:
            addBorder(CSSShorthand.EDGE.ALL, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.BORDER_TOP_WIDTH:
            addBorder(CSSShorthand.EDGE.TOP, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.BORDER_RIGHT_WIDTH:
            addBorder(CSSShorthand.EDGE.RIGHT, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.BORDER_BOTTOM_WIDTH:
            addBorder(CSSShorthand.EDGE.BOTTOM, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.BORDER_LEFT_WIDTH:
            addBorder(CSSShorthand.EDGE.LEFT, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.PADDING:
            addPadding(CSSShorthand.EDGE.ALL, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.PADDING_LEFT:
            addPadding(CSSShorthand.EDGE.LEFT, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.PADDING_TOP:
            addPadding(CSSShorthand.EDGE.TOP, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.PADDING_RIGHT:
            addPadding(CSSShorthand.EDGE.RIGHT, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
          case Constants.Name.PADDING_BOTTOM:
            addPadding(CSSShorthand.EDGE.BOTTOM, WXUtils.fastGetFloat(shorthand.get(key)));
            break;
        }
      }
    }
  }

  private void addMargin(CSSShorthand.EDGE spacingType, float margin) {
    if (mMargins == null) {
      mMargins = new CSSShorthand();
    }
    mMargins.set(spacingType, margin);
  }

  private void addPadding(CSSShorthand.EDGE spacingType, float padding) {
    if (mPaddings == null) {
      mPaddings = new CSSShorthand();
    }
    mPaddings.set(spacingType, padding);
  }

  private void addBorder(CSSShorthand.EDGE spacingType, float border) {
    if (mBorders == null) {
      mBorders = new CSSShorthand();
    }
    mBorders.set(spacingType, border);
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
}
