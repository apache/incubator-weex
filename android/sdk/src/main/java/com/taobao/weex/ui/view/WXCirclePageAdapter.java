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
package com.taobao.weex.ui.view;

import android.support.v4.view.PagerAdapter;
import android.view.View;
import android.view.ViewGroup;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class WXCirclePageAdapter extends PagerAdapter {

  /**
   * Subviews
   */
  private List<View> views = new ArrayList<>();
  private List<View> shadow = new ArrayList<>();
  private boolean needLoop = true;

  public boolean isRTL = false;
  private List<View> originalViews = new ArrayList<>();

  public WXCirclePageAdapter(List<View> views, boolean needLoop) {
    super();
    this.views = new ArrayList<>(views);
    this.originalViews = new ArrayList<>(views);
    this.needLoop = needLoop;
  }

  public void setLayoutDirectionRTL(boolean isRTL) {
    if (isRTL == this.isRTL) return;
    this.isRTL = isRTL;
    this.views = new ArrayList<>(this.originalViews);
    if (isRTL) {
      Collections.reverse(this.views);
    }
    ensureShadow();
  }

  public WXCirclePageAdapter() {
    this(true);
  }

  public WXCirclePageAdapter(boolean needLoop) {
    super();
    this.needLoop = needLoop;
  }

  public void addPageView(View view) {
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("onPageSelected >>>> addPageView");
    }

    originalViews.add(view);
    if (this.isRTL) {
      views.add(0, view);
    } else {
      views.add(view);
    }
    ensureShadow();
  }

  public void removePageView(View view) {
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("onPageSelected >>>> removePageView");
    }
    views.remove(view);
    originalViews.remove(view);
    ensureShadow();
  }

  public void replacePageView(View oldView, View newView) {
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("onPageSelected >>>> replacePageView");
    }

    int index = views.indexOf(oldView);
    views.remove(index);
    views.add(index, newView);
    ensureShadow();

    index = originalViews.indexOf(oldView);
    originalViews.remove(index);
    originalViews.add(index, newView);
  }

  @Override
  public int getCount() {
    return shadow.size();
  }

  public int getRealCount() {
    return views.size();
  }

  @Override
  public Object instantiateItem(ViewGroup container, int position) {
    View pageView = null;
    try {
      pageView = shadow.get(position);
      if (WXEnvironment.isApkDebugable()) {
        WXLogUtils.d("onPageSelected >>>> instantiateItem >>>>> position:" + position + ",position % getRealCount()" + position % getRealCount());
      }
      if (pageView.getParent() == null) {
        container.addView(pageView);
      } else {
        ((ViewGroup) pageView.getParent()).removeView(pageView);
        container.addView(pageView);
      }
    } catch (Exception e) {
      WXLogUtils.e("[CirclePageAdapter] instantiateItem: ", e);
    }
    return pageView;
  }

  @Override
  public void destroyItem(ViewGroup container, int position, Object object) {
    if (WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("onPageSelected >>>> destroyItem >>>>> position:" + position);
    }
    // container.removeView((View) object);
  }

  @Override
  public boolean isViewFromObject(View view, Object object) {
    return view == object;
  }

  @Override
  public int getItemPosition(Object object) {
    return POSITION_NONE;
  }

  public int getPagePosition(View page) {
    return views.indexOf(page);
  }

  public int getItemIndex(Object object) {
    if (object instanceof View) {
      return views.indexOf(object);
    } else {
      return -1;
    }
  }

  public List<View> getViews(){
    return views;
  }

  private void ensureShadow() {
    List<View> temp = new ArrayList<>();
    if (needLoop && views.size() > 2) {
      temp.add(0, views.get(views.size() - 1));
      for (View view : views) {
        temp.add(view);
      }
      temp.add(views.get(0));
    } else {
      temp.addAll(views);
    }
    shadow.clear();
    notifyDataSetChanged();
    shadow.addAll(temp);
    notifyDataSetChanged();
  }

  public int getRealPosition(int shadowPosition) {
    if (shadowPosition < 0 || shadowPosition >= shadow.size()) {
      return -1;
    } else {
      return getItemIndex(shadow.get(shadowPosition));
    }
  }

  public int getFirst() {
    if (needLoop && views.size() > 2) {
      return 1;
    } else {
      return 0;
    }
  }
}
