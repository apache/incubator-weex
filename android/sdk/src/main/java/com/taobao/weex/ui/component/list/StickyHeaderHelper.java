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
package com.taobao.weex.ui.component.list;

import android.view.View;
import android.view.ViewGroup;

import com.taobao.weex.common.WXThread;
import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * Created by sospartan on 17/03/2017.
 */

public class StickyHeaderHelper {
  private final ViewGroup mParent;
  private Map<String,View> mHeaderViews = new HashMap<>(); // map for <ref,view>
  private Map<String,WXCell> mHeaderComps = new HashMap<>(); // map for <ref,comp>
  private String mCurrentStickyRef = null;

  public StickyHeaderHelper(ViewGroup parent){
    this.mParent = parent;
  }

  /**
   * @param component
   */
  public void notifyStickyShow(WXCell component) {
    if (component == null)
      return;
    mHeaderComps.put(component.getRef(),component);
    if(mCurrentStickyRef != null){
      WXCell cell = mHeaderComps.get(mCurrentStickyRef);
      if(cell ==null || component.getScrollPositon() > cell.getScrollPositon()){
        mCurrentStickyRef = component.getRef();
      }
    }else{
      mCurrentStickyRef = component.getRef();
    }

    {
      if(mCurrentStickyRef==null){
        WXLogUtils.e("Current Sticky ref is null.");
        return;
      }

      WXCell headComponent = mHeaderComps.get(mCurrentStickyRef);
      final View headerView = headComponent.getRealView();
      if (headerView == null) {
        WXLogUtils.e("Sticky header's real view is null.");
        return;
      }
      View header = mHeaderViews.get(headComponent.getRef());
      if( header != null){
        //already there
        header.bringToFront();
      }else {
        mHeaderViews.put(headComponent.getRef(), headerView);
        //record translation, it should not change after transformation
        final float translationX = headerView.getTranslationX();
        final float translationY = headerView.getTranslationY();
        headComponent.removeSticky();

        ViewGroup existedParent;
        if ((existedParent = (ViewGroup) headerView.getParent()) != null) {
          existedParent.removeView(headerView);
        }
        mParent.addView(headerView);
        //recover translation, sometimes it will be changed on fling
        headerView.setTranslationX(translationX);
        headerView.setTranslationY(translationY);

      }
      if (headComponent.getDomObject().getEvents().contains("sticky")) {
        headComponent.fireEvent("sticky");
      }
    }
  }


  public void notifyStickyRemove(WXCell compToRemove) {
    if (compToRemove == null)
      return;
    final WXCell component = mHeaderComps.remove(compToRemove.getRef());
    final View headerView = mHeaderViews.remove(compToRemove.getRef());


    if(component == null || headerView == null){
      WXLogUtils.e(" sticky header to remove not found."+compToRemove.getRef());
      return;
    }
    if(mCurrentStickyRef != null && mCurrentStickyRef.equals(compToRemove.getRef())){
      mCurrentStickyRef = null;
    }
    mParent.post(WXThread.secure(new Runnable() {
      @Override
      public void run() {
        mParent.removeView(headerView);
        component.recoverySticky();
      }
    }));
    if (component.getDomObject().getEvents().contains("unsticky")) {
      component.fireEvent("unsticky");
    }
  }


  public void updateStickyView(int currentStickyPos) {
    Iterator<Map.Entry<String, WXCell>> iterator = mHeaderComps.entrySet().iterator();
    List<WXCell> toRemove = new ArrayList<>();
    while(iterator.hasNext()){
      Map.Entry<String, WXCell> next = iterator.next();
      WXCell cell = next.getValue();
      int pos = cell.getScrollPositon();
      if(pos > currentStickyPos){
        toRemove.add(cell);
      }else if(pos == currentStickyPos){
        mCurrentStickyRef = cell.getRef();
        View view = mHeaderViews.get(cell.getRef());
        if(view != null){
          view.bringToFront();
        }
      }
    }
    for(WXCell cell:toRemove){
      notifyStickyRemove(cell);
    }
  }
}
