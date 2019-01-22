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
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.FrameLayout;
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
import java.util.regex.Pattern;

/** Created by Bruce Too On 2019/1/10. At 09:56 */
public class DefaultHtmlTagAdapter implements IWxHtmlTagAdapter {

  protected static final float DEFAULT_IMAGE_ASPECT_RATIO = 0.5625f; // 16:9;
  protected Context context;
  protected WxHtmlComponent component;
  /** All <img> wrapped native views with there "src" value */
  protected Map<View, String> mImageMap = new HashMap<>();

  private static Pattern sWidthPattern;
  private static Pattern sHeightPattern;

  @Override
  public View getHtmlTagView(
      Context context, WxHtmlComponent component, String tagName, String html) {
    this.context = context;
    this.component = component;
    View tagView = null;
    switch (tagName) {
      case HtmlComponent.TAG_DEFAULT:
        tagView = getDefaultTextView(html);
        break;
      case HtmlComponent.TAG_TABLE:
        tagView = getDefaultTabView(html);
        break;
      case HtmlComponent.TAG_IMAGE:
        tagView = getDefaultImageView(html);
        break;
      case HtmlComponent.TAG_VIDEO:
        tagView = getDefaultVideo(html);
        break;
      default: // text
        View extendTagView = getExtendTagView(tagName,html);
        tagView = extendTagView == null ? getDefaultTextView(html) : extendTagView;
        break;
    }
    return tagView;
  }

  @Override
  public View.OnClickListener getTagViewClickListener(View clickView, String tagName, String html) {
    return new EmptyClickListener();
  }

  /**
   * For custom view
   *
   * @param info html string
   * @return tag's native view
   */
  protected View getExtendTagView(String tagName,String info) {
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

  /**
   * In my case,the video tag info may like below: <video style="width:**;height:**" src=""
   * poster="">
   *
   * @param info video tag raw string
   * @return return video view self
   */
  protected View getDefaultVideo(String info) {
    FrameLayout layout = new FrameLayout(context);
    ImageView icon = new ImageView(context);
    icon.setImageResource(android.R.drawable.ic_media_play);
    ImageView bg = new ImageView(context);
    bg.setScaleType(getResizeMode(component.getImageResize()));
    String poster = HtmlComponent.getAttributeValue("poster", info);
    mImageMap.put(bg, poster);
    WXSDKManager.getInstance().getIWXImgLoaderAdapter().setImage(poster, bg, null, null);
    LinearLayout.LayoutParams params =
        new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
    params.width = WXViewUtils.getScreenWidth(context) - component.getEdgesWidth();
    params.height = (int) (params.width * DEFAULT_IMAGE_ASPECT_RATIO);
    layout.addView(bg, params);
    layout.addView(
        icon,
        new FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT,
            Gravity.CENTER));
    layout.setOnClickListener(getTagViewClickListener(layout,HtmlComponent.TAG_VIDEO, info));
    return layout;
  }

  protected View getDefaultImageView(String info) {
    FrameLayout layout = new FrameLayout(context);
    ImageView imageView = new ImageView(context);
    imageView.setScaleType(getResizeMode(component.getImageResize()));
    int height = -1;
    int width = -1;
    try {
      // [a-zA-Z]{2}$ for unit of px or wx
      height =
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
    params.height = (int) (params.width * DEFAULT_IMAGE_ASPECT_RATIO);
    if (height == -1 || width == -1) { // don't have explicit height or width
      params.height = (int) (params.width * DEFAULT_IMAGE_ASPECT_RATIO);
    } else {
      params.height = (int) (params.width * height * 1.0f / width);
    }
    layout.addView(imageView,params);
    String src = HtmlComponent.getAttributeValue("src", info);
    WXSDKManager.getInstance().getIWXImgLoaderAdapter().setImage(src, imageView, null, null);

    imageView.setOnClickListener(getTagViewClickListener(imageView,HtmlComponent.TAG_IMAGE, info));
    mImageMap.put(imageView, src);
    return layout;
  }

  protected View getDefaultTextView(String html) {
    JellyBeanSpanFixTextView textView = new JellyBeanSpanFixTextView(context);
    WxHtmlTagHandler tagHandler = new WxHtmlTagHandler();
    SpannableStringBuilder span = (SpannableStringBuilder) HtmlCompat.fromHtml(context, html, HtmlCompat.FROM_HTML_MODE_LEGACY, tagHandler);
    // remove the last "\n" and add it to start
    if (span.length() > 0) {
      span.replace(span.length() - "\n".length(), span.length(), "");
      span.insert(0, "\n");
      textView.setText(span);
    }

    textView.setMovementMethod(LinkMovementMethod.getInstance());
    CharSequence text = textView.getText();
    if (text instanceof Spannable) {
      Spannable sp = (Spannable) text;
      URLSpan[] oldUrlSpans = sp.getSpans(0, text.length(), URLSpan.class);
      SpannableStringBuilder ssb = new SpannableStringBuilder(text);
      for (URLSpan oldUrlSpan : oldUrlSpans) {
        ssb.removeSpan(oldUrlSpan);
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
      getTagViewClickListener(view,HtmlComponent.TAG_DEFAULT, url).onClick(view);
    }
  }

  protected ImageView.ScaleType getResizeMode(String resizeMode) {
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

  protected static Pattern getWidthPattern() {
    if (sWidthPattern == null) {
      sWidthPattern = Pattern.compile("(?:\\s+|\\A)width\\s*:\\s*(\\S*)\\b");
    }
    return sWidthPattern;
  }

  protected static Pattern getHeightPattern() {
    if (sHeightPattern == null) {
      sHeightPattern = Pattern.compile("(?:\\s+|\\A)height\\s*:\\s*(\\S*)\\b");
    }
    return sHeightPattern;
  }
}
