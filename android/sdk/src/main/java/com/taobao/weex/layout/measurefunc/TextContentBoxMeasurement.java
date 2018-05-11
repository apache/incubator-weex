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
package com.taobao.weex.layout.measurefunc;

import android.graphics.Canvas;
import android.os.Build;
import android.os.Looper;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.Editable;
import android.text.Layout;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.SpannedString;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.text.TextUtils;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.AlignmentSpan;
import android.text.style.ForegroundColorSpan;

import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.TextDecorationSpan;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXCustomStyleSpan;
import com.taobao.weex.dom.WXLineHeightSpan;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.layout.ContentBoxMeasurement;
import com.taobao.weex.layout.MeasureMode;
import com.taobao.weex.layout.MeasureSize;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXText;
import com.taobao.weex.ui.component.WXTextDecoration;
import com.taobao.weex.utils.WXDomUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXResourceUtils;

import java.lang.ref.WeakReference;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

import static com.taobao.weex.dom.WXStyle.UNSET;
import static com.taobao.weex.utils.WXUtils.isUndefined;

/**
 * Created by miomin on 2018/3/9.
 */

public class TextContentBoxMeasurement extends ContentBoxMeasurement {

  private static final Canvas DUMMY_CANVAS = new Canvas();

  public TextContentBoxMeasurement(WXComponent component) {
    super(component);
  }

  class SetSpanOperation {

    protected final int start, end, flag;
    protected final Object what;

    SetSpanOperation(int start, int end, Object what) {
      this(start, end, what, Spanned.SPAN_INCLUSIVE_EXCLUSIVE);
    }

    SetSpanOperation(int start, int end, Object what, int flag) {
      this.start = start;
      this.end = end;
      this.what = what;
      this.flag = flag;
    }

    public void execute(Spannable sb) {
      sb.setSpan(what, start, end, flag);
    }
  }

  private static final String ELLIPSIS = "\u2026";
  private boolean mIsColorSet = false;
  private boolean hasBeenMeasured = false;
  private int mColor;
  private int mFontStyle = UNSET;
  private int mFontWeight = UNSET;
  private int mNumberOfLines = UNSET;
  private int mFontSize = UNSET;
  private int mLineHeight = UNSET;
  private float previousWidth = Float.NaN;
  private String mFontFamily = null;
  private String mText = null;
  private TextUtils.TruncateAt textOverflow;
  private Layout.Alignment mAlignment;
  private WXTextDecoration mTextDecoration = WXTextDecoration.NONE;
  private TextPaint mTextPaint;
  private @Nullable
  Spanned spanned;
  private @Nullable
  Layout layout;
  private AtomicReference<Layout> atomicReference = new AtomicReference<>();

  /**
   * uiThread = false
   **/
  @Override
  public void layoutBefore() {
    mTextPaint = new TextPaint(TextPaint.ANTI_ALIAS_FLAG);
    hasBeenMeasured = false;
    updateStyleAndText();
    spanned = createSpanned(mText);
  }

  /**
   * uiThread = false
   **/
  @Override
  public void measureInternal(float width, float height, int widthMeasureMode, int heightMeasureMode) {
    float measureWidth = width, measureHeight = height;
    hasBeenMeasured = true;
    float textWidth = getTextWidth(mTextPaint, width, widthMeasureMode == MeasureMode.EXACTLY);

    if (textWidth > 0 && mText != null) {
      layout = createLayout(textWidth,null);
      previousWidth = layout.getWidth();
      if (Float.isNaN(width)) {
        measureWidth = layout.getWidth();
      } else {
        measureWidth = Math.min(layout.getWidth(), measureWidth);
      }

      if (Float.isNaN(height)) {
        measureHeight = layout.getHeight();
      }
    } else {
      if (widthMeasureMode == MeasureMode.UNSPECIFIED) {
        measureWidth = 0;
      }
      if (heightMeasureMode == MeasureMode.UNSPECIFIED) {
        measureHeight = 0;
      }
    }
    mMeasureWidth = measureWidth;
    mMeasureHeight = measureHeight;
  }

  /**
   * uiThread = false
   **/
  @Override
  public void layoutAfter(float computedWidth, float computedHeight) {
    if(mComponent!=null) {
      if (hasBeenMeasured) {
        if (layout != null &&
            WXDomUtils
                .getContentWidth(mComponent.getPadding(), mComponent.getBorder(), computedWidth)
                != previousWidth) {
          recalculateLayout(computedWidth);
        }
      } else {
        updateStyleAndText();
        recalculateLayout(computedWidth);
      }
      hasBeenMeasured = false;
      if (layout != null && !layout.equals(atomicReference.get()) &&
          Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
        if (Thread.currentThread() != Looper.getMainLooper().getThread()) {
          warmUpTextLayoutCache(layout);
        }
      }
      swap();
      WXSDKManager.getInstance().getWXRenderManager().postOnUiThread(new Runnable() {
        @Override
        public void run() {
          mComponent.updateExtra(atomicReference.get());
        }
      }, mComponent.getInstanceId());
    }
  }

  private void updateStyleAndText() {
    updateStyleImp(mComponent.getStyles());
    mText = WXAttr.getValue(mComponent.getAttrs());
  }

  /**
   * Record the property according to the given style
   *
   * @param style the give style.
   */
  private void updateStyleImp(Map<String, Object> style) {
    if (style != null) {
      if (style.containsKey(Constants.Name.LINES)) {
        int lines = WXStyle.getLines(style);
        mNumberOfLines = lines > 0 ? lines : UNSET;
      }
      if (style.containsKey(Constants.Name.FONT_SIZE)) {
        mFontSize = WXStyle.getFontSize(style, mComponent.getViewPortWidth());
      }
      if (style.containsKey(Constants.Name.FONT_WEIGHT)) {
        mFontWeight = WXStyle.getFontWeight(style);
      }
      if (style.containsKey(Constants.Name.FONT_STYLE)) {
        mFontStyle = WXStyle.getFontStyle(style);
      }
      if (style.containsKey(Constants.Name.COLOR)) {
        mColor = WXResourceUtils.getColor(WXStyle.getTextColor(style));
        mIsColorSet = mColor != Integer.MIN_VALUE;
      }
      if (style.containsKey(Constants.Name.TEXT_DECORATION)) {
        mTextDecoration = WXStyle.getTextDecoration(style);
      }
      if (style.containsKey(Constants.Name.FONT_FAMILY)) {
        mFontFamily = WXStyle.getFontFamily(style);
      }
      mAlignment = WXStyle.getTextAlignment(style);
      textOverflow = WXStyle.getTextOverflow(style);
      int lineHeight = WXStyle.getLineHeight(style, mComponent.getViewPortWidth());
      if (lineHeight != UNSET) {
        mLineHeight = lineHeight;
      }
    }
  }

  /**
   * Update {@link #spanned} according to the give charSequence and styles
   *
   * @param text the give raw text.
   * @return an Spanned contains text and spans
   */
  protected
  @NonNull
  Spanned createSpanned(String text) {
    if (!TextUtils.isEmpty(text)) {
      SpannableString spannable = new SpannableString(text);
      updateSpannable(spannable, Spanned.SPAN_INCLUSIVE_EXCLUSIVE);
      return spannable;
    }
    return new SpannableString("");
  }

  protected void updateSpannable(Spannable spannable, int spanFlag) {
    int end = spannable.length();
    if (mFontSize == UNSET) {
      setSpan(spannable, new AbsoluteSizeSpan(WXText.sDEFAULT_SIZE), 0, end, spanFlag);
    }
    else{
      setSpan(spannable,  new AbsoluteSizeSpan(mFontSize), 0, end, spanFlag);
    }

    if (mLineHeight != UNSET) {
      setSpan(spannable, new WXLineHeightSpan(mLineHeight), 0, end, spanFlag);
    }

    setSpan(spannable, new AlignmentSpan.Standard(mAlignment), 0, end, spanFlag);

    if (mFontStyle != UNSET || mFontWeight != UNSET || mFontFamily != null) {
      setSpan(spannable, new WXCustomStyleSpan(mFontStyle, mFontWeight, mFontFamily), 0, end, spanFlag);
    }

    if (mIsColorSet) {
      setSpan(spannable, new ForegroundColorSpan(mColor), 0, end, spanFlag);
    }

    if (mTextDecoration == WXTextDecoration.UNDERLINE || mTextDecoration == WXTextDecoration.LINETHROUGH) {
      setSpan(spannable, new TextDecorationSpan(mTextDecoration), 0, end, spanFlag);
    }
  }

  private void setSpan(Spannable spannable, Object what, int start, int end, int spanFlag){
    spannable.setSpan(what, start, end, spanFlag);
  }

  /**
   * Get text width according to constrain of outerWidth with and forceToDesired
   *
   * @param textPaint      paint used to measure text
   * @param outerWidth     the width that css-layout desired.
   * @param forceToDesired if set true, the return value will be outerWidth, no matter what the width
   *                       of text is.
   * @return if forceToDesired is false, it will be the minimum value of the width of text and
   * outerWidth in case of outerWidth is defined, in other case, it will be outer width.
   */
  private float getTextWidth(TextPaint textPaint, float outerWidth, boolean forceToDesired) {
    if (mText == null) {
      if (forceToDesired) {
        return outerWidth;
      }
      return 0;
    }
    else {
      float textWidth;
      if (forceToDesired) {
        textWidth = outerWidth;
      } else {
        float desiredWidth = Layout.getDesiredWidth(spanned, textPaint);
        if (isUndefined(outerWidth) || desiredWidth < outerWidth) {
          textWidth = desiredWidth;
        } else {
          textWidth = outerWidth;
        }
      }
      return textWidth;
    }
  }

  /**
   * Update layout according to {@link #mText} and span
   *
   * @param width          the specified width.
   * @param forceWidth     If true, force the text width to the specified width, otherwise, text width
   *                       may equals to or be smaller than the specified width.
   * @param previousLayout the result of previous layout, could be null.
   */
  private
  @NonNull
  Layout createLayout(final float textWidth, @Nullable Layout previousLayout) {
    Layout layout;
    if (previousWidth != textWidth || previousLayout == null) {
      layout = new StaticLayout(spanned, mTextPaint, (int) Math.ceil(textWidth),
              Layout.Alignment.ALIGN_NORMAL, 1, 0, false);
    } else {
      layout = previousLayout;
    }
    if (mNumberOfLines != UNSET && mNumberOfLines > 0 && mNumberOfLines < layout.getLineCount()) {
      int lastLineStart, lastLineEnd;
      lastLineStart = layout.getLineStart(mNumberOfLines - 1);
      lastLineEnd = layout.getLineEnd(mNumberOfLines - 1);
      if (lastLineStart < lastLineEnd) {
        SpannableStringBuilder builder = null;
        if (lastLineStart > 0) {
          builder = new SpannableStringBuilder(spanned.subSequence(0, lastLineStart));
        } else {
          builder = new SpannableStringBuilder();
        }
        Editable lastLine = new SpannableStringBuilder(spanned.subSequence(lastLineStart, lastLineEnd));
        builder.append(truncate(lastLine, mTextPaint, (int) Math.ceil(textWidth), textOverflow));
        adjustSpansRange(spanned, builder);
        spanned = builder;
        return new StaticLayout(spanned, mTextPaint, (int) Math.ceil(textWidth),
                Layout.Alignment.ALIGN_NORMAL, 1, 0, false);
      }
    }
    return layout;
  }

  /**
   * Truncate the source span to the specified lines.
   * Caller of this method must ensure that the lines of text is <strong>greater than desired lines and need truncate</strong>.
   * Otherwise, unexpected behavior may happen.
   *
   * @param source     The source span.
   * @param paint      the textPaint
   * @param desired    specified lines.
   * @param truncateAt truncate method, null value means clipping overflow text directly, non-null value means using ellipsis strategy to clip
   * @return The spans after clipped.
   */
  private
  @NonNull
  Spanned truncate(@Nullable Editable source, @NonNull TextPaint paint,
                   int desired, @Nullable TextUtils.TruncateAt truncateAt) {
    Spanned ret = new SpannedString("");
    if (!TextUtils.isEmpty(source) && source.length() > 0) {
      if (truncateAt != null) {
        source.append(ELLIPSIS);
        Object[] spans = source.getSpans(0, source.length(), Object.class);
        for (Object span : spans) {
          int start = source.getSpanStart(span);
          int end = source.getSpanEnd(span);
          if (start == 0 && end == source.length() - 1) {
            source.removeSpan(span);
            source.setSpan(span, 0, source.length(), source.getSpanFlags(span));
          }
        }
      }

      StaticLayout layout;
      int startOffset;

      while (source.length() > 1) {
        startOffset = source.length() - 1;
        if (truncateAt != null) {
          startOffset -= 1;
        }
        source.delete(startOffset, startOffset + 1);
        layout = new StaticLayout(source, paint, desired, Layout.Alignment.ALIGN_NORMAL, 1, 0, false);
        if (layout.getLineCount() <= 1) {
          ret = source;
          break;
        }
      }
    }
    return ret;
  }

  /**
   * Adjust span range after truncate due to the wrong span range during span copy and slicing.
   *
   * @param beforeTruncate The span before truncate
   * @param afterTruncate  The span after truncate
   */
  private void adjustSpansRange(@NonNull Spanned beforeTruncate, @NonNull Spannable afterTruncate) {
    Object[] spans = beforeTruncate.getSpans(0, beforeTruncate.length(), Object.class);
    for (Object span : spans) {
      int start = beforeTruncate.getSpanStart(span);
      int end = beforeTruncate.getSpanEnd(span);
      if (start == 0 && end == beforeTruncate.length()) {
        afterTruncate.removeSpan(span);
        afterTruncate.setSpan(span, 0, afterTruncate.length(), beforeTruncate.getSpanFlags(span));
      }
    }
  }

  private void recalculateLayout(float computedWidth) {
    float contentWidth = WXDomUtils.getContentWidth(mComponent.getPadding(), mComponent.getBorder(), computedWidth);
    if (contentWidth > 0) {
      spanned = createSpanned(mText);
      if (mText != null) {
        layout = createLayout(contentWidth, layout);
        previousWidth = layout.getWidth();
      } else {
        previousWidth = 0;
      }
    }
  }

  /**
   * As warming up TextLayoutCache done in the DOM thread may manipulate UI operation,
   * there may be some exception, in which case the exception is ignored. After all,
   * this is just a warm up operation.
   *
   * @return false for warm up failure, otherwise returns true.
   */
  private boolean warmUpTextLayoutCache(Layout layout) {
    boolean result;
    try {
      layout.draw(DUMMY_CANVAS);
      result = true;
    } catch (Exception e) {
      WXLogUtils.eTag("TextWarmUp", e);
      result = false;
    }
    return result;
  }

  /**
   * Move the reference of current layout to the {@link AtomicReference} for further use,
   * then clear current layout.
   */
  private void swap() {
    if (layout != null) {
      atomicReference.set(layout);
      layout = null;
    }
    hasBeenMeasured = false;
  }
}
