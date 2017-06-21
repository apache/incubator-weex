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
package com.taobao.weex.dom;

import android.support.annotation.NonNull;

import com.taobao.weex.dom.flex.Spacing;

/**
 * Created by sospartan on 22/06/2017.
 */

class ResultDomObject implements ImmutableDomObject {
  private final String mRef;
  private final Spacing mMargin;
  private final Spacing mBorder;
  private final Spacing mPadding;
  private final float mLayoutWidth;
  private final float mLayoutHeight;
  private final float mLayoutX;
  private final float mLayoutY;
  private final boolean mIsFixed;
  private final WXStyle mStyles;
  private final WXEvent mEvents;
  private final WXAttr mAttrs;
  private final Object mExtra;
  private final String mType;

  @Override
  public String getRef() {
    return mRef;
  }

  @NonNull
  @Override
  public Spacing getMargin() {
    return mMargin;
  }

  @Override
  public float getLayoutWidth() {
    return mLayoutWidth;
  }

  @Override
  public float getLayoutHeight() {
    return mLayoutHeight;
  }

  @Override
  public float getLayoutX() {
    return mLayoutX;
  }

  @Override
  public float getLayoutY() {
    return mLayoutY;
  }

  @Override
  public boolean isFixed() {
    return mIsFixed;
  }

  @NonNull
  @Override
  public WXStyle getStyles() {
    return mStyles;
  }

  @NonNull
  @Override
  public WXEvent getEvents() {
    return mEvents;
  }

  @NonNull
  @Override
  public WXAttr getAttrs() {
    return mAttrs;
  }

  @NonNull
  @Override
  public Spacing getPadding() {
    return mPadding;
  }

  @NonNull
  @Override
  public Spacing getBorder() {
    return mBorder;
  }

  @Override
  public Object getExtra() {
    return mExtra;
  }

  @Override
  public String getType() {
    return mType;
  }

  ResultDomObject(String ref, Spacing margin, Spacing border, Spacing padding, float layoutWidth, float layoutHeight,
                  float layoutX, float layoutY, boolean isFixed, WXStyle styles, WXEvent events, WXAttr attrs,
                  Object extra, String type) {
    mRef = ref;
    mMargin = margin;
    mBorder = border;
    mPadding = padding;
    mLayoutWidth = layoutWidth;
    mLayoutHeight = layoutHeight;
    mLayoutX = layoutX;
    mLayoutY = layoutY;
    mIsFixed = isFixed;
    mStyles = styles;
    mEvents = events;
    mAttrs = attrs;
    mExtra = extra;
    mType = type;
  }

  public static ImmutableDomObject create(ImmutableDomObject src) {
    return new ResultDomObject(
        src.getRef(),
        src.getMargin(),
        src.getBorder(),
        src.getPadding(),
        src.getLayoutWidth(),
        src.getLayoutHeight(),
        src.getLayoutX(),
        src.getLayoutY(),
        src.isFixed(),
        src.getStyles().clone(),
        src.getEvents().clone(),
        src.getAttrs().clone(),
        src.getExtra(),
        src.getType());
  }
}
