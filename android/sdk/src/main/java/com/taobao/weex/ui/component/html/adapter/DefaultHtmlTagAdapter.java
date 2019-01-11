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
package com.taobao.weex.ui.component.html.adapter;

import android.content.Context;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.URLSpan;
import android.view.View;
import android.webkit.WebView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWxHtmlTagAdapter;
import com.taobao.weex.ui.component.html.AtMostWebView;
import com.taobao.weex.ui.component.html.HtmlComponent;
import com.taobao.weex.ui.component.html.JellyBeanSpanFixTextView;
import com.taobao.weex.ui.component.html.WxHtmlComponent;
import com.taobao.weex.ui.component.html.WxHtmlTagHandler;
import com.taobao.weex.ui.component.html.htmlcompat.HtmlCompat;
import com.taobao.weex.utils.WXViewUtils;

import java.util.HashMap;
import java.util.Map;

/** Created by Bruce Too On 2019/1/10. At 09:56 */
public class DefaultHtmlTagAdapter implements IWxHtmlTagAdapter {

  protected static final float DEFAULT_IMAGE_ASPECT_RATIO = 0.75f; // 3 * 1.0f/ 4;
  protected Context context;
  protected WxHtmlComponent component;
  /** All <img> wrapped native views with there "src" value */
  protected Map<View, String> mImageMap = new HashMap<>();

  @Override
  public View getHtmlTagView(Context context, WxHtmlComponent component, int tagType, String html) {
    this.context = context;
    this.component = component;
    View tagView = null;
    switch (tagType) {
      case HtmlComponent.TEXT_VIEW:
        tagView = getDefaultTextView(html);
        break;
      case HtmlComponent.TABLE_VIEW:
        tagView = getDefaultTabView(html);
        break;
      case HtmlComponent.IMAGE_VIEW:
        tagView = getDefaultImageView(html);
        break;
      case HtmlComponent.VIDEO_VIEW:
      default: // text
        View extendTagView = getExtendTagView(html);
        tagView = extendTagView == null ? getDefaultTextView(html) : extendTagView;
        break;
    }
    return tagView;
  }

  @Override
  public View.OnClickListener getTagViewClickListener(int tagType, String html) {
    //    View.OnClickListener onClickListener = null;
    //    switch (tagType) {
    //      case HtmlComponent.TEXT_VIEW:
    //        break;
    //      case HtmlComponent.TABLE_VIEW:
    //        break;
    //      case HtmlComponent.IMAGE_VIEW:
    //        onClickListener = new EmptyClickListener();
    //        break;
    //      case HtmlComponent.VIDEO_VIEW:
    //        onClickListener = new EmptyClickListener();
    //        break;
    //      default: // text
    //        break;
    //    }
    return new EmptyClickListener();
  }

  /**
   * For custom view
   *
   * @param info html string
   * @return tag's native view
   */
  protected View getExtendTagView(String info) {
    return null;
  }

  protected WebView getDefaultTabView(String info) {
    AtMostWebView webView = new AtMostWebView(context);
    webView.setOnLongClickListener(
        new View.OnLongClickListener() {
          @Override
          public boolean onLongClick(View view) {
            return true;
          }
        });
    webView.loadDataWithBaseURL(
        null,
        HtmlComponent.getHtmlTemplateForTable(component.getTableTemplate(), info),
        "text" + "/html",
        "utf-8",
        null);
    return webView;
  }

  protected ImageView getDefaultImageView(String info) {
    ImageView imageView = new ImageView(context);
    imageView.setScaleType(getResizeMode(component.getImageResize()));
    int hegiht = -1;
    int width = -1;
    try {
      // [a-zA-Z]{2}$ for unit of px or wx
      hegiht =
          Integer.valueOf(
              HtmlComponent.getAttributeValue("height", info).replaceAll("[a-zA-Z]{2}$", ""));
      width =
          Integer.valueOf(
              HtmlComponent.getAttributeValue("width", info).replaceAll("[a-zA-Z]{2}$", ""));
    } catch (Exception e) {
      // Not the case we care now,ignore
    }
    LinearLayout.LayoutParams params =
        new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
    params.width = WXViewUtils.getScreenWidth(context) - component.getEdgesWidth();
    if (hegiht == -1 || width == -1) { // don't have explicit height or width
      params.height = (int) (params.width * DEFAULT_IMAGE_ASPECT_RATIO);
    } else {
      params.height = (int) (params.width * hegiht * 1.0f / width);
    }
    imageView.setLayoutParams(params);
    String src = HtmlComponent.getAttributeValue("src", info);
    WXSDKManager.getInstance().getIWXImgLoaderAdapter().setImage(src, imageView, null, null);

    imageView.setOnClickListener(getTagViewClickListener(HtmlComponent.IMAGE_VIEW, info));
    mImageMap.put(imageView, src);
    return imageView;
  }

  protected View getDefaultTextView(String html) {
    JellyBeanSpanFixTextView textView = new JellyBeanSpanFixTextView(context);
    WxHtmlTagHandler tagHandler = new WxHtmlTagHandler();
    textView.setText(
        HtmlCompat.fromHtml(context, html, HtmlCompat.FROM_HTML_MODE_LEGACY, null, tagHandler));

    textView.setMovementMethod(LinkMovementMethod.getInstance());
    CharSequence text = textView.getText();
    if (text instanceof Spannable) {
      Spannable sp = (Spannable) text;
      URLSpan[] oldUrlSpans = sp.getSpans(0, text.length(), URLSpan.class);
      SpannableStringBuilder ssb = new SpannableStringBuilder(text);
      for (URLSpan oldUrlSpan : oldUrlSpans) {
        //        ssb.removeSpan(oldUrlSpan);
        DefaultURLSpan defaultURLSpan = new DefaultURLSpan(oldUrlSpan.getURL());
        ssb.setSpan(
            defaultURLSpan,
            sp.getSpanStart(oldUrlSpan),
            sp.getSpanEnd(oldUrlSpan),
            Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
      }
      textView.setText(ssb);
    }
    return textView;
  }

  private class DefaultURLSpan extends ClickableSpan {
    private String url;

    DefaultURLSpan(String url) {
      this.url = url;
    }

    @Override
    public void onClick(View view) {
      getTagViewClickListener(HtmlComponent.TEXT_VIEW, url).onClick(view);
    }
  }

  private ImageView.ScaleType getResizeMode(String resizeMode) {
    ImageView.ScaleType scaleType = ImageView.ScaleType.FIT_XY;
    if (TextUtils.isEmpty(resizeMode)) {
      return scaleType;
    }

    switch (resizeMode) {
      case "cover":
        scaleType = ImageView.ScaleType.CENTER_CROP;
        break;
      case "contain":
        scaleType = ImageView.ScaleType.FIT_CENTER;
        break;
      case "stretch":
        scaleType = ImageView.ScaleType.FIT_XY;
        break;
      default:
        break;
    }
    return scaleType;
  }

  private class EmptyClickListener implements View.OnClickListener {
    @Override
    public void onClick(View v) {
      if (WXEnvironment.isApkDebugable()) {
        Toast.makeText(context, "you click the view", Toast.LENGTH_SHORT).show();
      }
    }
  }
}
