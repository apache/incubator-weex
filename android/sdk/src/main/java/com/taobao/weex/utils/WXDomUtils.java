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

package com.taobao.weex.utils;

import com.taobao.weex.dom.CSSConstants;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.dom.CSSShorthand;

public class WXDomUtils {

  /**
   * Get the content width of the dom.
   * @return the width of the dom that excludes left-padding, left-border-width,
   * right-border-width and right-padding.
   */
  public static float getContentWidth(WXComponent component) {
    float rawWidth = component.getLayoutWidth();
    float leftPadding, rightPadding, leftBorder, rightBorder;
    CSSShorthand padding = component.getPadding();
    CSSShorthand border = component.getBorder();

    if (!CSSConstants.isUndefined((leftPadding = padding.get(CSSShorthand.EDGE.LEFT)))) {
      rawWidth -= leftPadding;
    }
    if (!CSSConstants.isUndefined((rightPadding = padding.get(CSSShorthand.EDGE.RIGHT)))) {
      rawWidth -= rightPadding;
    }

    if (!CSSConstants.isUndefined(leftBorder = border.get(CSSShorthand.EDGE.LEFT))) {
      rawWidth -= leftBorder;
    }
    if (!CSSConstants.isUndefined(rightBorder = border.get(CSSShorthand.EDGE.RIGHT))) {
      rawWidth -= rightBorder;
    }
    return rawWidth;
  }

  /**
   * Get the content height of the dom.
   * @return the height of the dom that excludes top-padding, top-border-width, bottom-padding
   * and bottom-border-width.
   */
  public static float getContentHeight(WXComponent component) {
    float rawHeight = component.getLayoutHeight();
    float topPadding, bottomPadding, topBorder, bottomBorder;
    CSSShorthand padding = component.getPadding();
    CSSShorthand border = component.getBorder();

    if (!CSSConstants.isUndefined((topPadding = padding.get(CSSShorthand.EDGE.TOP)))) {
      rawHeight -= topPadding;
    }
    if (!CSSConstants.isUndefined((bottomPadding = padding.get(CSSShorthand.EDGE.BOTTOM)))) {
      rawHeight -= bottomPadding;
    }

    if (!CSSConstants.isUndefined(topBorder = border.get(CSSShorthand.EDGE.TOP))) {
      rawHeight -= topBorder;
    }
    if (!CSSConstants.isUndefined(bottomBorder = border.get(CSSShorthand.EDGE.BOTTOM))) {
      rawHeight -= bottomBorder;
    }
    return rawHeight;
  }

  public static float getContentWidth(WXStyle style, float layoutWidth, int viewport){
    float leftPadding, rightPadding, leftBorder, rightBorder;

    if (!CSSConstants.isUndefined((leftPadding = style.getPaddingLeft(viewport)))) {
      layoutWidth -= leftPadding;
    }
    if (!CSSConstants.isUndefined((rightPadding = style.getPaddingRight(viewport)))) {
      layoutWidth -= rightPadding;
    }

    if (!CSSConstants.isUndefined(leftBorder = style.getBorderLeftWidth(viewport))) {
      layoutWidth -= leftBorder;
    }
    if (!CSSConstants.isUndefined(rightBorder = style.getBorderRightWidth(viewport))) {
      layoutWidth -= rightBorder;
    }
    return layoutWidth;
  }
}
