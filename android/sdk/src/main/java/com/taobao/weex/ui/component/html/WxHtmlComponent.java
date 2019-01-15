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
package com.taobao.weex.ui.component.html;

import android.content.Context;
import android.support.annotation.NonNull;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.CSSShorthand;
import com.taobao.weex.dom.binding.JSONUtils;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentProp;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by Bruce Too
 * On 2019/1/5.
 * At 10:51
 * A Component that handle unsupported tags by custom native view
 * Util now, we use {@link TextView} to map normal text-string, support
 * <ol> <ul> <li> in {@link TextView} (Not handle nested li now...WIP)
 * And can extend {@link WxHtmlTagHandler} to support more tag
 *
 * ImageView to map <img> tag
 * WebView to map <table> tag with custom wrap html template {@link HtmlComponent#HTML_TEMPLATE}
 */
public class WxHtmlComponent extends WXComponent<ScrollView> {

  private static final int DEFAULT_PAGE_EDGE = 20;
  private List<HtmlComponent> mHtmlComponents = new ArrayList<>();
  private String mImageResize = "cover";
  private String mTableTemplate = HtmlComponent.HTML_TEMPLATE;
  private int mLeftEdge = DEFAULT_PAGE_EDGE;
  private int mRightEdge = DEFAULT_PAGE_EDGE;

  public WxHtmlComponent(
      WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
  }

  public WxHtmlComponent(
      WXSDKInstance instance,
      WXVContainer parent,
      int type,
      BasicComponentData basicComponentData) {
    super(instance, parent, type, basicComponentData);
  }

  @Override
  protected ScrollView initComponentHostView(@NonNull final Context context) {
    LinearLayout layout = new LinearLayout(context);
    layout.setOrientation(LinearLayout.VERTICAL);
    ScrollView scrollView = new ScrollView(context);
    scrollView.addView(
        layout, FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT);
    return scrollView;
  }

  /**
   * image: { resize: 'cover' }, table: { template: "" } template related to {@link
   * HtmlComponent#HTML_TEMPLATE}
   *
   * @param htmlOption options for html
   */
  @WXComponentProp(name = "htmlOption")
  public void setOption(String htmlOption) {
    extractOption(htmlOption);
  }

  public void extractOption(String htmlOption) {
    try {
      JSONObject option = JSONUtils.toJSON(htmlOption);
      if (option != null) {
        // image
        JSONObject image = option.getJSONObject("image");
        mImageResize = WXUtils.getString(image.get(Constants.Name.RESIZE), "cover");
        // table
        mTableTemplate = WXUtils.getString(image.get("template"), HtmlComponent.HTML_TEMPLATE);
      }

      CSSShorthand padding = getBasicComponentData().getPadding();
      CSSShorthand margin = getBasicComponentData().getMargin();
      mLeftEdge =
          (int)
              (padding.get(CSSShorthand.EDGE.LEFT)
                  + padding.get(CSSShorthand.EDGE.ALL)
                  + margin.get(CSSShorthand.EDGE.LEFT)
                  + margin.get(CSSShorthand.EDGE.ALL));

      mRightEdge =
          (int)
              (padding.get(CSSShorthand.EDGE.RIGHT)
                  + padding.get(CSSShorthand.EDGE.ALL)
                  + margin.get(CSSShorthand.EDGE.RIGHT)
                  + margin.get(CSSShorthand.EDGE.ALL));

      mLeftEdge = mLeftEdge == 0 ? WXViewUtils.dip2px(DEFAULT_PAGE_EDGE) : mLeftEdge;
      mRightEdge = mRightEdge == 0 ? WXViewUtils.dip2px(DEFAULT_PAGE_EDGE) : mRightEdge;
    } catch (Exception e) {
      WXLogUtils.e("extractOption failed:" + e.getMessage());
    }
  }

  @WXComponentProp(name = "htmlText")
  public void setHtml(String htmlText) {

    extractOption(WXUtils.getString(getAttrs().get("htmlOption"), ""));

    ViewGroup layout = (ViewGroup) getHostView().getChildAt(0);
    if (layout.getChildCount() != 0) {
      layout.removeAllViews();
    }

    if (mHtmlComponents.size() == 0) {
      mHtmlComponents.addAll(
          HtmlComponent.parseTags(htmlText, HtmlComponent.TAG_IMAGE, HtmlComponent.TAG_TABLE));
    }

    LinearLayout.LayoutParams params =
        new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
    for (HtmlComponent htmlComponent : mHtmlComponents) {
      layout.addView(
          getInstance()
              .getHtmlTextAdapter()
              .getHtmlTagView(getContext(), this, htmlComponent.tagName, htmlComponent.info),
          params);
    }
  }

  public String getImageResize() {
    return mImageResize;
  }

  public String getTableTemplate() {
    return mTableTemplate;
  }

  /**
   * left/right params,use to get the real view width in screen
   *
   * @return width of edges(left/right)
   */
  public int getEdgesWidth() {
    return mLeftEdge + mRightEdge;
  }

  @Override
  public void destroy() {
    super.destroy();
    mHtmlComponents.clear();
  }
}
