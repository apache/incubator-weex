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
import android.graphics.Color;
import android.graphics.Rect;
import android.support.annotation.NonNull;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import com.alibaba.fastjson.JSONArray;
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
 * And can extend {@link WxHtmlTagHandler} to support more tag, and we can
 * add header & footer view inside this component,like below:
 * <html-text :html-text="formatHtml"
 *            :html-option="htmlOption">
 *      <div> Any Header View</div>
 *      <div> Any Footer View</div>
 * </html-text>
 * <html-text> only parse({@link #addSubView(View, int)})the first two direct views,
 * the first one as header({@link #mHeaderContainer})
 * the second one as footer({@link #mFooterContainer})
 */
public class WxHtmlComponent extends WXVContainer<ScrollView> {

  private static final int DEFAULT_PAGE_EDGE = 20;
  private static final String TAG_DIRECT_CHILD = "direct_child";
  private static final int TAG_DIRECT_CHILD_ID = 1 + 2 << 24;
  private List<HtmlComponent> mHtmlComponents = new ArrayList<>();
  private String mImageResize = "cover";
  private String mTableTemplate = HtmlComponent.HTML_TEMPLATE;
  private int mLeftEdge = DEFAULT_PAGE_EDGE;
  private int mRightEdge = DEFAULT_PAGE_EDGE;
  private FrameLayout mHeaderContainer;
  private LinearLayout mCenterContainer;
  private FrameLayout mFooterContainer;
  private String[] mSupportedTags =new String[]{HtmlComponent.TAG_IMAGE, HtmlComponent.TAG_TABLE,
      HtmlComponent.TAG_VIDEO};
  //this params for adjust the second child view's top margin
  private int mFirstChildViewHeight;
  private ViewTreeObserver.OnScrollChangedListener mHeaderScrollListener = new HeaderScrollListener();
  private ViewTreeObserver.OnScrollChangedListener mFooterScrollListener = new FooterScrollListener();
  private Rect mHeaderRect = new Rect();
  private Rect mFooterRect = new Rect();
  private Rect mRootRect = new Rect();
  private boolean mIsHeaderShowing;
  private boolean mIsFooterShowing;

  public WxHtmlComponent(
      WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
    super(instance, parent, basicComponentData);
  }

  @Override
  protected ScrollView initComponentHostView(@NonNull final Context context) {
    LinearLayout container = new LinearLayout(context);
    mHeaderContainer = new FrameLayout(getContext());
    mCenterContainer = new LinearLayout(getContext());
    mFooterContainer = new FrameLayout(getContext());
    mFooterContainer.setBackgroundColor(Color.LTGRAY);
    container.addView(mHeaderContainer);
    container.addView(mCenterContainer);
    container.addView(mFooterContainer);
    mCenterContainer.setOrientation(LinearLayout.VERTICAL);
    container.setOrientation(LinearLayout.VERTICAL);
    ScrollView scrollView = new ScrollView(context);
    scrollView.setVerticalScrollBarEnabled(false);
    scrollView.addView(
        container, FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT);
    return scrollView;
  }

  @Override
  protected void onFinishLayout() {
    super.onFinishLayout();
    getHostView().getLocalVisibleRect(mRootRect);
  }

  @Override
  public void addSubView(View child, int index) {
    if (child == null || getRealView() == null) {
      return;
    }
    //remark the direct sub view
    child.setTag(TAG_DIRECT_CHILD_ID, TAG_DIRECT_CHILD);
    if(index == 0){ //header view
      mHeaderContainer.removeAllViews();
      mHeaderContainer.getViewTreeObserver().addOnScrollChangedListener(mHeaderScrollListener);
      mHeaderContainer.addView(child,FrameLayout.LayoutParams.MATCH_PARENT,
                               FrameLayout.LayoutParams.WRAP_CONTENT);
    }else if(index == 1){ //footer view
      mFooterContainer.removeAllViews();
      mFooterContainer.getViewTreeObserver().addOnScrollChangedListener(mFooterScrollListener);
      mFooterContainer.addView(child,FrameLayout.LayoutParams.MATCH_PARENT,
                               FrameLayout.LayoutParams.WRAP_CONTENT);
    }
    //ignore other index..
  }

  @Override
  public ViewGroup.LayoutParams getChildLayoutParams(WXComponent child, View childView, int width, int height, int left, int right, int top, int bottom) {
    ViewGroup.LayoutParams lp = childView.getLayoutParams();
    if (lp == null) {
      lp = new FrameLayout.LayoutParams(width, height);
    } else {
      lp.width = width;
      lp.height = height;
    }

    if (lp instanceof ViewGroup.MarginLayoutParams) {
      View hostView = child.getHostView();
      //reset the opposite top margin of the direct sub view
      if (hostView != null && TAG_DIRECT_CHILD.equals(hostView.getTag(TAG_DIRECT_CHILD_ID))) {
        this.setMarginsSupportRTL((ViewGroup.MarginLayoutParams) lp, left, top - mFirstChildViewHeight, right,
                                  bottom);
        if (mFirstChildViewHeight == 0) {
          mFirstChildViewHeight = height;
        }
      }
    }
    return lp;
  }

  /**
   * htmlOption: { image: { resize: 'cover' }, table: { template: '' },
   * tags:['image','table','video'] }
   * Template related to {@link HtmlComponent#HTML_TEMPLATE}
   * Tags indicate the support one,and default contains image,video,table
   * Once add new tag support, need custom native view in {@link
   * com.taobao.weex.ui.component.html.adapter.DefaultHtmlTagAdapter#getExtendTagView(String)}
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
        if (image != null) {
          mImageResize = WXUtils.getString(image.get(Constants.Name.RESIZE), "cover");
        }
        // table
        JSONObject table = option.getJSONObject("table");
        if (table != null) {
          mTableTemplate = WXUtils.getString(table.get("template"), HtmlComponent.HTML_TEMPLATE);
        }
        //tags
        JSONArray tags = option.getJSONArray("tags");
        if (tags != null && tags.size() != 0) {
          mSupportedTags = new String[tags.size()];
          for (int i = 0; i < tags.size(); i++) {
            mSupportedTags[i] = tags.getString(i);
          }
        }
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

    if (mCenterContainer.getChildCount() != 0) {
      mCenterContainer.removeAllViews();
    }

    if (mHtmlComponents.size() == 0) {
      mHtmlComponents.addAll(
          HtmlComponent.parseTags(htmlText, mSupportedTags));
    }

    LinearLayout.LayoutParams params =
        new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
    for (HtmlComponent htmlComponent : mHtmlComponents) {
      mCenterContainer.addView(
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
    mHeaderContainer.getViewTreeObserver().removeOnScrollChangedListener(mHeaderScrollListener);
    mFooterContainer.getViewTreeObserver().removeOnScrollChangedListener(mFooterScrollListener);
    mHtmlComponents.clear();
  }

  private class HeaderScrollListener implements ViewTreeObserver.OnScrollChangedListener {
    @Override
    public void onScrollChanged() {
      if (mHeaderContainer != null && mHeaderContainer.getChildCount() != 0) {
        mHeaderContainer.getLocalVisibleRect(mHeaderRect);
        if (mHeaderRect.width() != 0) {
          if (mHeaderRect.bottom < 0) { // out top
            if (mIsHeaderShowing) {
              getInstance().fireEvent(getRef(),Constants.Event.HEADER_DISAPPEAR);
              mIsHeaderShowing = false;
              WXLogUtils.i("ScrollChange", "header disappear");
            }
          } else { // inside
            if (!mIsHeaderShowing) {
              getInstance().fireEvent(getRef(),Constants.Event.HEADER_APPEAR);
              mIsHeaderShowing = true;
              WXLogUtils.i("ScrollChange", "header appear");
            }
          }
        }
      }
    }
  }

  private class FooterScrollListener implements ViewTreeObserver.OnScrollChangedListener {
    @Override
    public void onScrollChanged() {
      if (mFooterContainer != null && mFooterContainer.getChildCount() != 0) {
        mFooterContainer.getLocalVisibleRect(mFooterRect);
        if (mFooterRect.width() != 0) {
          if (mFooterRect.top > mRootRect.bottom) { // out bottom
            if (mIsFooterShowing) {
              getInstance().fireEvent(getRef(),Constants.Event.FOOTER_DISAPPEAR);
              mIsFooterShowing = false;
              WXLogUtils.i("ScrollChange", "footer disappear");
            }
          } else { // inside
            if (!mIsFooterShowing) {
              getInstance().fireEvent(getRef(),Constants.Event.FOOTER_APPEAR);
              mIsFooterShowing = true;
              WXLogUtils.i("ScrollChange", "footer appear");
            }
          }
        }
      }
    }
  }

}
