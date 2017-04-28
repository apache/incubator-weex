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
package com.taobao.weex.dom;

import android.text.TextPaint;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.flex.CSSConstants;
import com.taobao.weex.dom.flex.CSSNode;
import com.taobao.weex.dom.flex.MeasureOutput;
import com.taobao.weex.ui.component.WXText;
import com.taobao.weex.utils.TypefaceUtil;
import com.taobao.weex.utils.WXViewUtils;

import static com.taobao.weex.dom.WXStyle.UNSET;

/**
 * Created by sospartan on 7/12/16.
 */
public class BasicEditTextDomObject extends WXDomObject {

  private TextPaint mPaint = new TextPaint();

  //  private int mFontSize = UNSET;
  private int mLineHeight = UNSET;
//  private String mFontFamily = null;

  public BasicEditTextDomObject() {
    super();
    mPaint.setTextSize(WXViewUtils.getRealPxByWidth(WXText.sDEFAULT_SIZE,getViewPortWidth()));
    setMeasureFunction(new MeasureFunction() {
      @Override
      public void measure(CSSNode node, float width, MeasureOutput measureOutput) {
        if (CSSConstants.isUndefined(width)) {
          width = node.cssstyle.maxWidth;
        }
        measureOutput.height = getMeasureHeight();
        measureOutput.width = width;
      }
    });
  }

  @Override
  public void layoutBefore() {
    super.layoutBefore();
    updateStyleAndAttrs();
  }

  protected final float getMeasuredLineHeight() {
    return mLineHeight != UNSET && mLineHeight > 0 ? mLineHeight : mPaint.getFontMetrics(null);
  }

  protected float getMeasureHeight() {
    return getMeasuredLineHeight();
  }

  protected void updateStyleAndAttrs() {
    if (getStyles().size() > 0) {
      int fontSize = UNSET, fontStyle = UNSET, fontWeight = UNSET;
      String fontFamily = null;
      if (getStyles().containsKey(Constants.Name.FONT_SIZE)) {
        fontSize = WXStyle.getFontSize(getStyles(),getViewPortWidth());
      }

      if (getStyles().containsKey(Constants.Name.FONT_FAMILY)) {
        fontFamily = WXStyle.getFontFamily(getStyles());
      }

      if (getStyles().containsKey(Constants.Name.FONT_STYLE)) {
        fontStyle = WXStyle.getFontStyle(getStyles());
      }

      if (getStyles().containsKey(Constants.Name.FONT_WEIGHT)) {
        fontWeight = WXStyle.getFontWeight(getStyles());
      }

      int lineHeight = WXStyle.getLineHeight(getStyles(),getViewPortWidth());
      if (lineHeight != UNSET)
        mLineHeight = lineHeight;

      if (fontSize != UNSET)
        mPaint.setTextSize(fontSize);

      if (fontFamily != null) {
        TypefaceUtil.applyFontStyle(mPaint, fontStyle, fontWeight, fontFamily);
      }

      this.dirty();
    }
  }

}
