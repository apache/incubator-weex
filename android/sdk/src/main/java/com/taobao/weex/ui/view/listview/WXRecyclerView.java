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
package com.taobao.weex.ui.view.listview;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.support.annotation.Nullable;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.OrientationHelper;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;
import android.view.MotionEvent;
import android.view.View;

import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;

public class WXRecyclerView extends RecyclerView implements WXGestureObservable {

  public static final int TYPE_LINEAR_LAYOUT = 1;
  public static final int TYPE_GRID_LAYOUT = 2;
  public static final int TYPE_STAGGERED_GRID_LAYOUT = 3;
  private WXGesture mGesture;
  private boolean scrollable = true;

  public WXRecyclerView(Context context) {
    super(context);
  }

  public boolean isScrollable() {
    return scrollable;
  }

  public void setScrollable(boolean scrollable) {
    this.scrollable = scrollable;
  }

  @Override
  public boolean postDelayed(Runnable action, long delayMillis) {
    return super.postDelayed(WXThread.secure(action), delayMillis);
  }
  public void initView(Context context, int type,int orientation) {
    initView(context,type, Constants.Value.COLUMN_COUNT_NORMAL,Constants.Value.COLUMN_GAP_NORMAL,orientation);
  }


  /**
   *
   * @param context
   * @param type
   * @param orientation should be {@link OrientationHelper#HORIZONTAL} or {@link OrientationHelper#VERTICAL}
   */
  @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
  public void initView(Context context, int type, int columnCount, float columnGap, int orientation) {
    if (type == TYPE_GRID_LAYOUT) {
      setLayoutManager(new GridLayoutManager(context, columnCount,orientation,false));
    } else if (type == TYPE_STAGGERED_GRID_LAYOUT) {
      setLayoutManager(new ExtendedStaggeredGridLayoutManager(columnCount, orientation));
    } else if (type == TYPE_LINEAR_LAYOUT) {
      setLayoutManager(new ExtendedLinearLayoutManager(context,orientation,false));
    }
  }

  @Override
  public void registerGestureListener(@Nullable WXGesture wxGesture) {
    mGesture = wxGesture;
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    if(!scrollable) {
      return true;
    }
    boolean result = super.onTouchEvent(event);
    if (mGesture != null) {
      result |= mGesture.onTouch(this, event);
    }
    return result;
  }


  public void scrollTo(boolean smooth, int position, final  int offset, final int orientation){
    if (!smooth) {
      RecyclerView.LayoutManager layoutManager = getLayoutManager();
      if (layoutManager instanceof LinearLayoutManager) {
        //GridLayoutManager is also instance of LinearLayoutManager
        ((LinearLayoutManager) layoutManager).scrollToPositionWithOffset(position, -offset);
      } else if (layoutManager instanceof StaggeredGridLayoutManager) {
        ((StaggeredGridLayoutManager) layoutManager).scrollToPositionWithOffset(position, -offset);
      }
      //Any else?
    } else {
      if (offset != 0) {
        setOnSmoothScrollEndListener(new ExtendedLinearLayoutManager.OnSmoothScrollEndListener() {
          @Override
          public void onStop() {
            if (orientation == Constants.Orientation.VERTICAL) {
                smoothScrollBy(0, offset);
            } else {
                smoothScrollBy(offset, 0);
            }
          }
        });
      }
      smoothScrollToPosition(position);
    }
  }

  public void setOnSmoothScrollEndListener(final ExtendedLinearLayoutManager.OnSmoothScrollEndListener onSmoothScrollEndListener){
    if(getLayoutManager() instanceof ExtendedLinearLayoutManager){
       ExtendedLinearLayoutManager extendedLinearLayoutManager = (ExtendedLinearLayoutManager)getLayoutManager();
       extendedLinearLayoutManager.setOnScrollEndListener(onSmoothScrollEndListener);
    }else{
      addOnScrollListener(new RecyclerView.OnScrollListener() {
        @Override
        public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
          if (newState == RecyclerView.SCROLL_STATE_IDLE) {
              recyclerView.removeOnScrollListener(this);
              if(onSmoothScrollEndListener != null){
                   onSmoothScrollEndListener.onStop();
              }
          }
        }
      });
    }
  }

}
