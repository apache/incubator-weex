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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Canvas;
import android.graphics.Typeface;
import android.os.Build;
import android.os.Looper;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.content.LocalBroadcastManager;
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

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.flex.CSSConstants;
import com.taobao.weex.dom.flex.CSSNode;
import com.taobao.weex.dom.flex.FloatUtil;
import com.taobao.weex.dom.flex.MeasureOutput;
import com.taobao.weex.ui.component.WXText;
import com.taobao.weex.ui.component.WXTextDecoration;
import com.taobao.weex.utils.StaticLayoutProxy;
import com.taobao.weex.utils.TypefaceUtil;
import com.taobao.weex.utils.WXDomUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXResourceUtils;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

import static com.taobao.weex.dom.WXStyle.UNSET;

/**
 * Class for calculating a given text's height and width. The calculating of width and height of
 * text is done by {@link Layout}.
 */
public class WXTextDomObject extends WXDomObject {

  /**
   * Command object for setSpan
   */
  private static class SetSpanOperation {

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

  /**
   * Object for calculating text's width and height. This class is an anonymous class of
   * implementing {@link com.taobao.weex.dom.flex.CSSNode.MeasureFunction}
   */
  /** package **/ static final CSSNode.MeasureFunction TEXT_MEASURE_FUNCTION = new CSSNode.MeasureFunction() {
    @Override
    public void measure(CSSNode node, float width, @NonNull MeasureOutput measureOutput) {
      WXTextDomObject textDomObject = (WXTextDomObject) node;
      if (CSSConstants.isUndefined(width)) {
        width = node.cssstyle.maxWidth;
      }
      boolean forceWidth = false;
      if(width > 0){
         if(node.getParent() != null && textDomObject.mAlignment == Layout.Alignment.ALIGN_CENTER){
            forceWidth = FloatUtil.floatsEqual(width, node.getParent().getLayoutWidth());
         }
      }
      textDomObject.hasBeenMeasured = true;
      width = textDomObject.getTextWidth(textDomObject.mTextPaint,width, forceWidth);
      if(width > 0 && textDomObject.mText != null) {
        textDomObject.layout = textDomObject.createLayout(width, true, null);
        textDomObject.previousWidth = textDomObject.layout.getWidth();
        measureOutput.height = textDomObject.layout.getHeight();
        measureOutput.width = textDomObject.previousWidth;
      }else{
        measureOutput.height = 0;
        measureOutput.width = 0;
      }
    }
  };


  private static final Canvas DUMMY_CANVAS = new Canvas();
  private static final String ELLIPSIS = "\u2026";
  private boolean mIsColorSet = false;
  private boolean hasBeenMeasured = false;
  private int mColor;
  /**
   * mFontStyle can be {@link Typeface#NORMAL} or {@link Typeface#ITALIC}.
   */
  private int mFontStyle = UNSET;
  /**
   * mFontWeight can be {@link Typeface#NORMAL} or {@link Typeface#BOLD}.
   */
  private int mFontWeight = UNSET;
  private int mNumberOfLines = UNSET;
  private int mFontSize = UNSET;
  private int mLineHeight = UNSET;
  private float previousWidth = Float.NaN;
  private String mFontFamily = WXEnvironment.getGlobalFontFamilyName();
  private String mText = null;
  private TextUtils.TruncateAt textOverflow;
  private Layout.Alignment mAlignment;
  private WXTextDecoration mTextDecoration = WXTextDecoration.NONE;
  private TextPaint mTextPaint = new TextPaint();
  private @Nullable Spanned spanned;
  private @Nullable Layout layout;
  private AtomicReference<Layout> atomicReference = new AtomicReference<>();

  private BroadcastReceiver mTypefaceObserver;

  /**
   * Create an instance of current class, and set {@link #TEXT_MEASURE_FUNCTION} as the
   * measureFunction
   * @see CSSNode#setMeasureFunction(MeasureFunction)
   */
  public WXTextDomObject() {
    super();
    mTextPaint.setFlags(TextPaint.ANTI_ALIAS_FLAG);
    setMeasureFunction(TEXT_MEASURE_FUNCTION);
    registerTypefaceObserverIfNeed(WXStyle.getFontFamily(getStyles()));
  }

  public TextPaint getTextPaint() {
    return mTextPaint;
  }

  /**
   * Prepare the text {@link Spanned} for calculating text's size. This is done by setting
   * various text span to the text.
   * @see android.text.style.CharacterStyle
   */
  @Override
  public void layoutBefore() {
    hasBeenMeasured = false;
    updateStyleAndText();
    spanned = createSpanned(mText);
    if(hasNewLayout()){
        if(WXEnvironment.isApkDebugable()) {
          WXLogUtils.d("Previous csslayout was ignored! markLayoutSeen() never called");
        }
        markUpdateSeen();
    }
    super.dirty();
    super.layoutBefore();
  }

  @Override
  public void layoutAfter() {
    if (hasBeenMeasured) {
      if (layout != null &&
          !FloatUtil.floatsEqual(WXDomUtils.getContentWidth(this), previousWidth)) {
        recalculateLayout();
      }
    } else {
      updateStyleAndText();
      recalculateLayout();
    }
    hasBeenMeasured = false;
    if (layout != null && !layout.equals(atomicReference.get()) &&
        Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
      if(Thread.currentThread() != Looper.getMainLooper().getThread()){
          warmUpTextLayoutCache(layout);
      }
    }
    swap();
    super.layoutAfter();
  }

  @Override
  public Layout getExtra() {
    return atomicReference.get();
  }

  @Override
  public void updateAttr(Map<String, Object> attrs) {
    swap();
    super.updateAttr(attrs);
    if (attrs.containsKey(Constants.Name.VALUE)) {
      mText = WXAttr.getValue(attrs);
    }
  }

  @Override
  public void updateStyle(Map<String, Object> styles) {
    swap();
    super.updateStyle(styles);
    updateStyleImp(styles);
  }

  @Override
  public WXTextDomObject clone() {
    if(isCloneThis()){
      return  this;
    }
    WXTextDomObject dom = null;
    try {
      dom = new WXTextDomObject();
      copyFields(dom);
      dom.hasBeenMeasured = hasBeenMeasured;
      dom.atomicReference = new AtomicReference<>(atomicReference.get());
    } catch (Exception e) {
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.e("WXTextDomObject clone error: ", e);
      }
    }
    if (dom != null) {
      dom.spanned = spanned;
    }
    return dom;
  }

  /**
   * RecalculateLayout.
   */
  private void recalculateLayout() {
    float contentWidth = WXDomUtils.getContentWidth(this);
    if (contentWidth > 0) {
      spanned = createSpanned(mText);
      if(mText != null){
         layout = createLayout(contentWidth, true, layout);
         previousWidth = layout.getWidth();
      }else{
         previousWidth = 0;
      }
    }
  }

  /**
   * Update style and text.
   */
  private void updateStyleAndText() {
    updateStyleImp(getStyles());
    mText = WXAttr.getValue(getAttrs());
  }

  /**
   * Record the property according to the given style
   * @param style the give style.
   */
  private void updateStyleImp(Map<String, Object> style) {
    if (style != null) {
      if (style.containsKey(Constants.Name.LINES)) {
        int lines = WXStyle.getLines(style);
        mNumberOfLines = lines > 0 ? lines : UNSET;
      }
      if (style.containsKey(Constants.Name.FONT_SIZE)) {
        mFontSize = WXStyle.getFontSize(style,getViewPortWidth());
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
      int lineHeight = WXStyle.getLineHeight(style,getViewPortWidth());
      if (lineHeight != UNSET) {
        mLineHeight = lineHeight;
      }
      registerTypefaceObserverIfNeed(mFontFamily);
    }
  }

  /**
   * Update layout according to {@link #mText} and span
   * @param width the specified width.
   * @param forceWidth If true, force the text width to the specified width, otherwise, text width
   *                   may equals to or be smaller than the specified width.
   * @param previousLayout the result of previous layout, could be null.
   */
  private
  @NonNull
  Layout createLayout(float width, boolean forceWidth, @Nullable Layout previousLayout) {
    float textWidth;
    textWidth = getTextWidth(mTextPaint, width, forceWidth);
    Layout layout;
    if (!FloatUtil.floatsEqual(previousWidth, textWidth) || previousLayout == null) {
      boolean forceRtl = false;
      Object direction = getStyles().get(Constants.Name.DIRECTION);
      if (direction != null && "text".equals(mType)) {
        forceRtl = direction.equals(Constants.Name.RTL);
      }
      layout = StaticLayoutProxy.create(spanned, mTextPaint, (int) Math.ceil(textWidth),
          Layout.Alignment.ALIGN_NORMAL, 1, 0, false, forceRtl);
    } else {
      layout = previousLayout;
    }
    if (mNumberOfLines != UNSET && mNumberOfLines > 0 && mNumberOfLines < layout.getLineCount()) {
      int lastLineStart, lastLineEnd;
      lastLineStart = layout.getLineStart(mNumberOfLines - 1);
      lastLineEnd = layout.getLineEnd(mNumberOfLines - 1);
      if (lastLineStart < lastLineEnd) {
        SpannableStringBuilder builder = null;
        if(lastLineStart > 0) {
          builder = new SpannableStringBuilder(spanned.subSequence(0, lastLineStart));
        }else{
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
   * @param source The source span.
   * @param paint the textPaint
   * @param desired specified lines.
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
        for(Object span:spans){
          int start = source.getSpanStart(span);
          int end = source.getSpanEnd(span);
          if(start == 0 && end == source.length()-1){
             source.removeSpan(span);
             source.setSpan(span, 0, source.length(), source.getSpanFlags(span));
          }
        }
      }

      StaticLayout layout;
      int startOffset;

      while (source.length() > 1) {
        startOffset = source.length() -1;
        if (truncateAt != null) {
          startOffset -= 1;
        }
        source.delete(startOffset, startOffset+1);
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
   * @param beforeTruncate The span before truncate
   * @param afterTruncate The span after truncate
   */
  private void adjustSpansRange(@NonNull Spanned beforeTruncate, @NonNull Spannable afterTruncate){
    Object[] spans = beforeTruncate.getSpans(0, beforeTruncate.length(), Object.class);
    for(Object span:spans){
      int start = beforeTruncate.getSpanStart(span);
      int end = beforeTruncate.getSpanEnd(span);
      if(start == 0 && end == beforeTruncate.length()){
        afterTruncate.removeSpan(span);
        afterTruncate.setSpan(span, 0, afterTruncate.length(), beforeTruncate.getSpanFlags(span));
      }
    }
  }

  /**
   * Get text width according to constrain of outerWidth with and forceToDesired
   * @param textPaint paint used to measure text
   * @param outerWidth the width that css-layout desired.
   * @param forceToDesired if set true, the return value will be outerWidth, no matter what the width
   *                   of text is.
   * @return if forceToDesired is false, it will be the minimum value of the width of text and
   * outerWidth in case of outerWidth is defined, in other case, it will be outer width.
   */
   float getTextWidth(TextPaint textPaint,float outerWidth, boolean forceToDesired) {
     if(mText == null){
       if(forceToDesired){
         return  outerWidth;
       }
        return  0;
     }
    float textWidth;
    if (forceToDesired) {
      textWidth = outerWidth;
    } else {
      float desiredWidth = Layout.getDesiredWidth(spanned, textPaint);
      if (CSSConstants.isUndefined(outerWidth) || desiredWidth < outerWidth) {
        textWidth = desiredWidth;
      } else {
        textWidth = outerWidth;
      }
    }
    return textWidth;
  }

  /**
   * Update {@link #spanned} according to the give charSequence and styles
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
    List<SetSpanOperation> ops = createSetSpanOperation(spannable.length(), spanFlag);
    if (mFontSize == UNSET) {
      ops.add(new SetSpanOperation(0, spannable.length(),
          new AbsoluteSizeSpan(WXText.sDEFAULT_SIZE), spanFlag));
    }
    Collections.reverse(ops);
    for (SetSpanOperation op : ops) {
      op.execute(spannable);
    }
  }

  /**
   * Create a task list which contains {@link SetSpanOperation}. The task list will be executed
   * in other method.
   * @param end the end character of the text.
   * @return a task list which contains {@link SetSpanOperation}.
   */
  private List<SetSpanOperation> createSetSpanOperation(int end, int spanFlag) {
    List<SetSpanOperation> ops = new LinkedList<>();
    int start = 0;
    if (end >= start) {
      if (mTextDecoration == WXTextDecoration.UNDERLINE || mTextDecoration == WXTextDecoration.LINETHROUGH) {
        ops.add(new SetSpanOperation(start, end, new TextDecorationSpan(mTextDecoration), spanFlag));
      }
      if (mIsColorSet) {
        ops.add(new SetSpanOperation(start, end,
            new ForegroundColorSpan(mColor), spanFlag));
      }
      if (mFontSize != UNSET) {
        ops.add(new SetSpanOperation(start, end, new AbsoluteSizeSpan(mFontSize), spanFlag));
      }
      if (mFontStyle != UNSET
          || mFontWeight != UNSET
          || mFontFamily != null) {
        ops.add(new SetSpanOperation(start, end,
            new WXCustomStyleSpan(mFontStyle, mFontWeight, mFontFamily),
            spanFlag));
      }
      ops.add(new SetSpanOperation(start, end, new AlignmentSpan.Standard(mAlignment), spanFlag));
      if (mLineHeight != UNSET) {
        ops.add(new SetSpanOperation(start, end, new WXLineHeightSpan(mLineHeight), spanFlag));
      }
    }
    return ops;
  }

  /**
   * Move the reference of current layout to the {@link AtomicReference} for further use,
   * then clear current layout.
   */
  private void swap() {
    if (layout != null) {
      atomicReference.set(layout);
      layout = null;
      mTextPaint = new TextPaint(mTextPaint);
    }
    hasBeenMeasured = false;
  }

  /**
   * As warming up TextLayoutCache done in the DOM thread may manipulate UI operation,
   there may be some exception, in which case the exception is ignored. After all,
   this is just a warm up operation.
   * @return false for warm up failure, otherwise returns true.
   */
  private boolean warmUpTextLayoutCache(Layout layout) {
    boolean result;
    try {
      layout.draw(DUMMY_CANVAS);
      result = true;
    } catch (Exception e) {
      WXLogUtils.eTag(TAG, e);
      result = false;
    }
    return result;
  }

  @Override
  public void destroy() {
    if (WXEnvironment.getApplication() != null && mTypefaceObserver != null) {
      WXLogUtils.d("WXText", "Unregister the typeface observer");
      LocalBroadcastManager.getInstance(WXEnvironment.getApplication()).unregisterReceiver(mTypefaceObserver);
      mTypefaceObserver = null;
    }
    super.destroy();
  }

  private void registerTypefaceObserverIfNeed(String desiredFontFamily) {
    if(TextUtils.isEmpty(desiredFontFamily)){
      return;
    }
    if (WXEnvironment.getApplication() == null) {
      WXLogUtils.w("WXText", "ApplicationContent is null on register typeface observer");
      return;
    }
    mFontFamily = desiredFontFamily;
    if (mTypefaceObserver != null) {
      return;
    }

    mTypefaceObserver = new BroadcastReceiver() {
      @Override
      public void onReceive(Context context, Intent intent) {
        String fontFamily = intent.getStringExtra("fontFamily");
        if (!mFontFamily.equals(fontFamily)) {
          return;
        }
        if(isDestroy() || getDomContext() == null){
          return;
        }

        DOMActionContext domActionContext = WXSDKManager.getInstance().getWXDomManager().getDomContext(getDomContext().getInstanceId());
        if(domActionContext == null){
          return;
        }
        WXDomObject domObject = domActionContext.getDomByRef(getRef());
        if(domObject == null){
          return;
        }
        domObject.markDirty();
        domActionContext.markDirty();
        WXSDKManager.getInstance().getWXDomManager().sendEmptyMessageDelayed(WXDomHandler.MsgType.WX_DOM_START_BATCH, 2);
        if(WXEnvironment.isApkDebugable()) {
          WXLogUtils.d("WXText", "Font family " + fontFamily + " is available");
        }
      }
    };
    if(WXEnvironment.isApkDebugable()) {
         WXLogUtils.d("WXText", "Font family register " + desiredFontFamily + " is available" + getRef());
    }
    LocalBroadcastManager.getInstance(WXEnvironment.getApplication()).registerReceiver(mTypefaceObserver, new IntentFilter(TypefaceUtil.ACTION_TYPE_FACE_AVAILABLE));
  }
}
