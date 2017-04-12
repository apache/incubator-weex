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

import com.taobao.weex.dom.ImmutableDomObject;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.flex.CSSConstants;
import com.taobao.weex.dom.flex.Spacing;

public class WXDomUtils {

  /**
   * Get the content width of the dom.
   * @return the width of the dom that excludes left-padding, left-border-width,
   * right-border-width and right-padding.
   */
  public static float getContentWidth(ImmutableDomObject domObject) {
    float rawWidth = domObject.getLayoutWidth();
    float leftPadding, rightPadding, leftBorder, rightBorder;
    Spacing padding = domObject.getPadding();
    Spacing border = domObject.getBorder();

    if (!CSSConstants.isUndefined((leftPadding = padding.get(Spacing.LEFT)))) {
      rawWidth -= leftPadding;
    }
    if (!CSSConstants.isUndefined((rightPadding = padding.get(Spacing.RIGHT)))) {
      rawWidth -= rightPadding;
    }

    if (!CSSConstants.isUndefined(leftBorder = border.get(Spacing.LEFT))) {
      rawWidth -= leftBorder;
    }
    if (!CSSConstants.isUndefined(rightBorder = border.get(Spacing.RIGHT))) {
      rawWidth -= rightBorder;
    }
    return rawWidth;
  }

  /**
   * Get the content height of the dom.
   * @return the height of the dom that excludes top-padding, top-border-width, bottom-padding
   * and bottom-border-width.
   */
  public static float getContentHeight(ImmutableDomObject domObject) {
    float rawHeight = domObject.getLayoutHeight();
    float topPadding, bottomPadding, topBorder, bottomBorder;
    Spacing padding = domObject.getPadding();
    Spacing border = domObject.getBorder();

    if (!CSSConstants.isUndefined((topPadding = padding.get(Spacing.TOP)))) {
      rawHeight -= topPadding;
    }
    if (!CSSConstants.isUndefined((bottomPadding = padding.get(Spacing.BOTTOM)))) {
      rawHeight -= bottomPadding;
    }

    if (!CSSConstants.isUndefined(topBorder = border.get(Spacing.TOP))) {
      rawHeight -= topBorder;
    }
    if (!CSSConstants.isUndefined(bottomBorder = border.get(Spacing.BOTTOM))) {
      rawHeight -= bottomBorder;
    }
    return rawHeight;
  }
}
