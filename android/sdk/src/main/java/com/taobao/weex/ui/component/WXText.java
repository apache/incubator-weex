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
package com.taobao.weex.ui.component;

import static com.taobao.weex.dom.WXStyle.UNSET;
import static com.taobao.weex.utils.WXUtils.isUndefined;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Canvas;
import android.graphics.Paint;
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
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.annotation.Component;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.TextDecorationSpan;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXCustomStyleSpan;
import com.taobao.weex.dom.WXLineHeightSpan;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.layout.ContentBoxMeasurement;
import com.taobao.weex.layout.MeasureMode;
import com.taobao.weex.layout.MeasureSize;
import com.taobao.weex.layout.measurefunc.TextContentBoxMeasurement;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.flat.FlatComponent;
import com.taobao.weex.ui.flat.widget.TextWidget;
import com.taobao.weex.ui.view.WXTextView;
import com.taobao.weex.utils.FontDO;
import com.taobao.weex.utils.TypefaceUtil;
import com.taobao.weex.utils.WXDomUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXResourceUtils;
import java.lang.reflect.InvocationTargetException;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Text component
 */
@Component(lazyload = false)
public class WXText extends WXComponent<WXTextView> implements FlatComponent<TextWidget> {

  private TextWidget mTextWidget;

  /**
   * The default text size
   **/
  public static final int sDEFAULT_SIZE = 32;
  private BroadcastReceiver mTypefaceObserver;
  private String mFontFamily;

  @Override
  public boolean promoteToView(boolean checkAncestor) {
    if (null != getInstance().getFlatUIContext()) {
      return getInstance().getFlatUIContext().promoteToView(this, checkAncestor, WXText.class);
    }
    return false;
  }

  @Override
  @NonNull
  public TextWidget getOrCreateFlatWidget() {
    if (mTextWidget == null) {
      mTextWidget = new TextWidget(getInstance().getFlatUIContext());
    }
    return mTextWidget;
  }

  @Override
  public boolean isVirtualComponent() {
    return !promoteToView(true);
  }

  public static class Creator implements ComponentCreator {
    public WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
      return new WXText(instance, parent, basicComponentData);
    }
  }

  @Deprecated
  public WXText(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
    this(instance, parent, basicComponentData);
  }

  public WXText(WXSDKInstance instance,
                WXVContainer parent, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
    setContentBoxMeasurement(new TextContentBoxMeasurement(this));
  }

  @Override
  protected WXTextView initComponentHostView(@NonNull Context context) {
    WXTextView textView = new WXTextView(context);
    textView.holdComponent(this);
    return textView;
  }

  @Override
  public void updateExtra(Object extra) {
    super.updateExtra(extra);
    if(extra instanceof Layout) {
      final Layout layout = (Layout) extra;
      if (!promoteToView(true)) {
        getOrCreateFlatWidget().updateTextDrawable(layout);
      } else if (getHostView() != null && !extra.equals(getHostView().getTextLayout())) {
        getHostView().setTextLayout(layout);
        getHostView().invalidate();
      }
    }
  }

  @Override
  protected void setAriaLabel(String label) {
    WXTextView text = getHostView();
    if (text != null) {
      text.setAriaLabel(label);
    }
  }

  @Override
  public void refreshData(WXComponent component) {
    super.refreshData(component);
    if (component instanceof WXText) {
      updateExtra(component.getExtra());
    }
  }

  @Override
  protected boolean setProperty(String key, Object param) {
    switch (key) {
      case Constants.Name.LINES:
      case Constants.Name.FONT_SIZE:
      case Constants.Name.FONT_WEIGHT:
      case Constants.Name.FONT_STYLE:
      case Constants.Name.COLOR:
      case Constants.Name.TEXT_DECORATION:
      case Constants.Name.TEXT_ALIGN:
      case Constants.Name.TEXT_OVERFLOW:
      case Constants.Name.LINE_HEIGHT:
      case Constants.Name.VALUE:
        return true;
      case Constants.Name.FONT_FAMILY:
        if (param != null) {
          registerTypefaceObserver(param.toString());
        }
        return true;
      default:
        return super.setProperty(key, param);
    }
  }

  @Override
  protected Object convertEmptyProperty(String propName, Object originalValue) {
    switch (propName) {
      case Constants.Name.FONT_SIZE:
        return WXText.sDEFAULT_SIZE;
      case Constants.Name.COLOR:
        return "black";
    }
    return super.convertEmptyProperty(propName, originalValue);
  }

  @Override
  protected void createViewImpl() {
    if(promoteToView(true)) {
      super.createViewImpl();
    }
  }

  @Override
  public void destroy() {
    super.destroy();
    if (WXEnvironment.getApplication() != null && mTypefaceObserver != null) {
      WXLogUtils.d("WXText", "Unregister the typeface observer");
      LocalBroadcastManager.getInstance(WXEnvironment.getApplication()).unregisterReceiver(mTypefaceObserver);
      mTypefaceObserver = null;
    }
  }

  private void registerTypefaceObserver(String desiredFontFamily) {
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

        FontDO fontDO = TypefaceUtil.getFontDO(fontFamily);
        if (fontDO != null && fontDO.getTypeface() != null && getHostView() != null) {
          WXTextView hostView = getHostView();
          Layout layout = hostView.getTextLayout();
          if (layout != null) {
            layout.getPaint().setTypeface(fontDO.getTypeface());
            WXLogUtils.d("WXText", "Apply font family " + fontFamily + " to paint");
          } else {
            WXLogUtils.w("WXText", "Layout not created");
          }
          hostView.invalidate();
        }
        WXLogUtils.d("WXText", "Font family " + fontFamily + " is available");
      }
    };

    LocalBroadcastManager.getInstance(WXEnvironment.getApplication()).registerReceiver(mTypefaceObserver, new IntentFilter(TypefaceUtil.ACTION_TYPE_FACE_AVAILABLE));
  }
}
