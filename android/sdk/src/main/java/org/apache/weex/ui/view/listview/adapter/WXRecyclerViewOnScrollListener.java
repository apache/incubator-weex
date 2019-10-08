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
package org.apache.weex.ui.view.listview.adapter;

import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;

import org.apache.weex.utils.WXLogUtils;

import java.lang.ref.WeakReference;


/**
 * Listener for scroll event of recyclerView
 */
public class WXRecyclerViewOnScrollListener extends RecyclerView.OnScrollListener {

  /**
   * type of layoutManager
   */
  protected LAYOUT_MANAGER_TYPE layoutManagerType;

  /**
   * The last position
   */
  private int[] mLastPositions;
  /**
   * The first position
   */
  private int[] mFirstPositions;


  /**
   * The location of last visible item
   */
  private int mLastVisibleItemPosition;

  /**
   * The location of last visible item
   */
  private int mFirstVisibleItemPosition;

  /**
   * The state of scroll status
   */
  private int mCurrentScrollState = 0;

  private WeakReference<IOnLoadMoreListener> listener;

  public WXRecyclerViewOnScrollListener(IOnLoadMoreListener listener) {
    this.listener = new WeakReference<>(listener);
  }

  @Override
  public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
    super.onScrollStateChanged(recyclerView, newState);
    mCurrentScrollState = newState;
    RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
    int visibleItemCount = layoutManager.getChildCount();
    int totalItemCount = layoutManager.getItemCount();

    if (visibleItemCount != 0) {
      int bottomOffset = (totalItemCount - mLastVisibleItemPosition - 1) * (recyclerView.getHeight()) / visibleItemCount;
      if (visibleItemCount > 0 && mCurrentScrollState == RecyclerView.SCROLL_STATE_IDLE) {
        if (listener != null && listener.get() != null) {
          listener.get().onLoadMore(bottomOffset);
        }
      }
    }
  }

  @Override
  public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
    super.onScrolled(recyclerView, dx, dy);
    RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
    if(listener == null){
      return;
    }
    IOnLoadMoreListener iOnLoadMoreListener = listener.get();

    if(iOnLoadMoreListener!=null) {

      iOnLoadMoreListener.onBeforeScroll(dx, dy);

      if (layoutManager instanceof LinearLayoutManager) {
        layoutManagerType = LAYOUT_MANAGER_TYPE.LINEAR;
        LinearLayoutManager linearLayoutManager = (LinearLayoutManager) layoutManager;
        mLastVisibleItemPosition = linearLayoutManager.findLastVisibleItemPosition();
        int firstVisible = linearLayoutManager.findFirstVisibleItemPosition();
        iOnLoadMoreListener.notifyAppearStateChange(firstVisible
            , mLastVisibleItemPosition
            , dx
            , dy);
      } else if (layoutManager instanceof GridLayoutManager) {
        layoutManagerType = LAYOUT_MANAGER_TYPE.GRID;
        GridLayoutManager gridLayoutManager = (GridLayoutManager) layoutManager;
        mLastVisibleItemPosition = gridLayoutManager.findLastVisibleItemPosition();
        iOnLoadMoreListener.notifyAppearStateChange(gridLayoutManager.findFirstVisibleItemPosition()
            , mLastVisibleItemPosition
            , dx
            , dy);

      } else if (layoutManager instanceof StaggeredGridLayoutManager) {
        layoutManagerType = LAYOUT_MANAGER_TYPE.STAGGERED_GRID;
        StaggeredGridLayoutManager staggeredGridLayoutManager = (StaggeredGridLayoutManager) layoutManager;
        int newSpanCount = staggeredGridLayoutManager.getSpanCount();
        if (mLastPositions == null || newSpanCount != mLastPositions.length) {
          mLastPositions = new int[newSpanCount];
        }
        if (mFirstPositions == null || newSpanCount != mFirstPositions.length) {
          mFirstPositions = new int[newSpanCount];
        }
        //avoid crash of support-v7 original bug
        try{
          staggeredGridLayoutManager.findFirstVisibleItemPositions(mFirstPositions);
          mFirstVisibleItemPosition = findMin(mFirstPositions);
          staggeredGridLayoutManager.findLastVisibleItemPositions(mLastPositions);
          mLastVisibleItemPosition = findMax(mLastPositions);
          iOnLoadMoreListener.notifyAppearStateChange(
              mFirstVisibleItemPosition
              , mLastVisibleItemPosition
              , dx
              , dy);

        }catch(Exception e){
          e.printStackTrace();
          WXLogUtils.e(e.toString());
        }

      } else {
        throw new RuntimeException(
            "Unsupported LayoutManager used. Valid ones are LinearLayoutManager, GridLayoutManager and StaggeredGridLayoutManager");
      }
    }
  }

  private int findMax(int[] lastPositions) {
    int max = lastPositions[0];
    for (int value : lastPositions) {
      if (value > max) {
        max = value;
      }
    }
    return max;
  }

  private int findMin(int[] firstPositions) {
    int min = firstPositions[0];
    for (int value : firstPositions) {
      if (value < min) {
        min = value;
      }
    }
    return min;
  }

  public enum LAYOUT_MANAGER_TYPE {
    LINEAR,
    GRID,
    STAGGERED_GRID
  }
}
