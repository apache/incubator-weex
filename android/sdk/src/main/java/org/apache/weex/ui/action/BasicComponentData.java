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
package org.apache.weex.ui.action;

import android.support.annotation.NonNull;
import android.view.View;

import org.apache.weex.common.Constants;
import org.apache.weex.dom.CSSShorthand;
import org.apache.weex.dom.WXAttr;
import org.apache.weex.dom.WXEvent;
import org.apache.weex.dom.WXStyle;
import org.apache.weex.ui.component.list.template.jni.NativeRenderObjectUtils;
import org.apache.weex.utils.WXUtils;

import java.util.Map;
import java.util.Set;

public class BasicComponentData<T extends View> {

  public String mRef;
  public String mComponentType;
  public String mParentRef;
  private WXStyle mStyles;
  private WXAttr mAttributes;
  private WXEvent mEvents;
  private CSSShorthand mMargins;
  private CSSShorthand mPaddings;
  private CSSShorthand mBorders;
  private long renderObjectPr = 0;

  public BasicComponentData(String ref, String componentType, String parentRef) {
    this.mRef = ref;
    this.mComponentType = componentType;
    this.mParentRef = parentRef;
  }

  public void addStyle(Map<String, Object> styles) {
    addStyle(styles, false);
  }

  public final void addStyle(Map<String, Object> styles, boolean byPesudo) {
    if (styles == null || styles.isEmpty()) {
      return;
    }
    if (mStyles == null) {
      mStyles = new WXStyle(styles);
    }
    else {
      mStyles.putAll(styles, byPesudo);
    }
  }

  public final void addAttr(Map<String, Object> attrs) {
    if (attrs == null || attrs.isEmpty()) {
      return;
    }
    if (mAttributes == null) {
      mAttributes = new WXAttr(attrs, 0);
    }
    else {
      mAttributes.putAll(attrs);
    }
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

  public final void addShorthand(float[] shorthand, CSSShorthand.TYPE type) {
    if (shorthand == null) {
      shorthand = new float[] {0, 0, 0, 0};
    }
    if (shorthand.length == 4) {
      switch (type) {
        case MARGIN:
          if (mMargins == null) {
            mMargins = new CSSShorthand(shorthand);
          } else {
            mMargins.replace(shorthand);
          }
          break;
        case PADDING:
          if (mPaddings == null) {
            mPaddings = new CSSShorthand(shorthand);
          } else {
            mPaddings.replace(shorthand);
          }
          break;
        case BORDER:
          if (mBorders == null) {
            mBorders = new CSSShorthand(shorthand);
          } else {
            mBorders.replace(shorthand);
          }
          break;
        default:
          break;
      }
    }
  }

  public final void addShorthand(Map<String, String> shorthand) {
    if (shorthand != null && !shorthand.isEmpty()) {
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



  @Override
  public BasicComponentData clone() throws CloneNotSupportedException {
    BasicComponentData basicComponentData = new BasicComponentData(mRef, mComponentType, mParentRef);
    basicComponentData.setBorders(getBorder().clone());
    basicComponentData.setMargins(getMargin().clone());
    basicComponentData.setPaddings(getPadding().clone());
    if(mAttributes != null){
      basicComponentData.mAttributes = mAttributes.clone();
    }
    if(mStyles != null){
      basicComponentData.mStyles = mStyles.clone();
    }
    if(mEvents != null){
      basicComponentData.mEvents = mEvents.clone();
    }

    if(renderObjectPr != 0){
      basicComponentData.setRenderObjectPr(NativeRenderObjectUtils.nativeCopyRenderObject(renderObjectPr));
    }
    return basicComponentData;
  }

  public long getRenderObjectPr() {
    return renderObjectPr;
  }

  public boolean isRenderPtrEmpty(){
    return  renderObjectPr == 0;
  }

  public synchronized void setRenderObjectPr(long renderObjectPr) {
    if(this.renderObjectPr != renderObjectPr){
      if(this.renderObjectPr != 0){
        throw  new  RuntimeException("RenderObjectPr has " + renderObjectPr + " old renderObjectPtr " + this.renderObjectPr);
      }
      this.renderObjectPr = renderObjectPr;
    }
  }
}
