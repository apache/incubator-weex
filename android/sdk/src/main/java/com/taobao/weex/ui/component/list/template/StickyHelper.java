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
package com.taobao.weex.ui.component.list.template;

import android.support.v4.util.ArrayMap;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;
import android.view.View;

import com.taobao.weex.common.Constants;
import com.taobao.weex.ui.view.refresh.wrapper.BounceRecyclerView;
import java.util.ArrayList;
import java.util.List;

/**
 * mStickyHelper
 * Created by furture on 2017/8/24.
 */
public class StickyHelper {
    private WXRecyclerTemplateList recyclerTemplateList;
    private List<Integer> stickyPositions;
    private ArrayMap<Integer, TemplateViewHolder>   stickyHolderCache;


    public StickyHelper(WXRecyclerTemplateList recyclerTemplateList) {
        this.recyclerTemplateList = recyclerTemplateList;
        this.stickyPositions = new ArrayList<>();
        this.stickyHolderCache = new ArrayMap();
    }

    /**
     * dispatch scroll event
     * */
    public void onBeforeScroll(int dx, int dy) {
        if(stickyPositions == null || stickyPositions.size() == 0){
           return;
        }
        BounceRecyclerView bounceRecyclerView = recyclerTemplateList.getHostView();
        RecyclerView recyclerView = recyclerTemplateList.getHostView().getInnerView();
        RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
        int firstVisiblePosition = -1;
        if (layoutManager instanceof LinearLayoutManager) {
            firstVisiblePosition = ((LinearLayoutManager) layoutManager).findFirstVisibleItemPosition();
        }else if (layoutManager instanceof StaggeredGridLayoutManager) {
            int [] firstVisibleItemPositions = new int[3];//max 3 column
            firstVisiblePosition = ((StaggeredGridLayoutManager) layoutManager).findFirstVisibleItemPositions(firstVisibleItemPositions)[0];
        }
        if(firstVisiblePosition < 0){
            return;
        }

        TemplateViewHolder firstVisibleItemHolder = (TemplateViewHolder) recyclerView.findViewHolderForAdapterPosition(firstVisiblePosition);
        if(firstVisibleItemHolder == null){
            return;
        }

        //find match sticky position
        int matchStickyPosition = -1;
        for (Integer headerPosition : stickyPositions){
            if(headerPosition == null){
                continue;
            }
            if(headerPosition <= firstVisiblePosition){
                matchStickyPosition = Math.max(matchStickyPosition, headerPosition);
            }else{
                break;
            }
        }
        if(matchStickyPosition < 0){
            //create holder for match position if not exist
            View stickyFakeView = bounceRecyclerView.getChildAt(bounceRecyclerView.getChildCount() - 1);
            if(stickyFakeView.getTag() instanceof TemplateViewHolder){
                TemplateViewHolder stickyFakeViewHolder = (TemplateViewHolder) stickyFakeView.getTag();
                bounceRecyclerView.removeView(stickyFakeViewHolder.itemView);
                stickyFakeViewHolder.itemView.setTranslationY(0);
                if(stickyFakeViewHolder.getComponent() != null
                        && stickyFakeViewHolder.getComponent().getDomObject() != null
                        && stickyFakeViewHolder.getComponent().getDomObject().getEvents().contains(Constants.Event.UNSTICKY)){
                    stickyFakeViewHolder.getComponent().fireEvent(Constants.Event.UNSTICKY);
                }
            }
            return;
        }

        //create holder for match position if not exist
        View stickyFakeView = bounceRecyclerView.getChildAt(bounceRecyclerView.getChildCount() - 1);
        if(!(stickyFakeView.getTag() instanceof  TemplateViewHolder)
                || ((TemplateViewHolder) stickyFakeView.getTag()).getHolderPosition() != matchStickyPosition){

             //remove previous sticky header
            if(stickyFakeView.getTag() instanceof TemplateViewHolder &&
                    ((TemplateViewHolder) stickyFakeView.getTag()).getHolderPosition() != matchStickyPosition){
                TemplateViewHolder stickyFakeViewHolder = (TemplateViewHolder) stickyFakeView.getTag();
                bounceRecyclerView.removeView(stickyFakeViewHolder.itemView);
                stickyFakeViewHolder.itemView.setTranslationY(0);
                if(stickyFakeViewHolder.getComponent() != null
                        && stickyFakeViewHolder.getComponent().getDomObject() != null
                        && stickyFakeViewHolder.getComponent().getDomObject().getEvents().contains(Constants.Event.UNSTICKY)){
                    stickyFakeViewHolder.getComponent().fireEvent(Constants.Event.UNSTICKY);
                }
            }

            //create new sticky
            TemplateViewHolder fakeStickyHolder = stickyHolderCache.get(matchStickyPosition);
            if(fakeStickyHolder == null || fakeStickyHolder.getItemViewType() != recyclerTemplateList.getItemViewType(matchStickyPosition)){
                fakeStickyHolder = recyclerTemplateList.onCreateViewHolder(recyclerView, recyclerTemplateList.getItemViewType(matchStickyPosition));
                stickyHolderCache.put(matchStickyPosition, fakeStickyHolder);
            }
            recyclerTemplateList.onBindViewHolder(fakeStickyHolder, matchStickyPosition);
            fakeStickyHolder.itemView.setTranslationY(0);
            fakeStickyHolder.itemView.setTag(fakeStickyHolder);
            bounceRecyclerView.addView(fakeStickyHolder.itemView);
            stickyFakeView = fakeStickyHolder.itemView;
            if(fakeStickyHolder.getComponent() != null
                    && fakeStickyHolder.getComponent().getDomObject() != null
                    && fakeStickyHolder.getComponent().getDomObject().getEvents().contains(Constants.Event.STICKY)){
                fakeStickyHolder.getComponent().fireEvent(Constants.Event.STICKY);
            }
        }
        TemplateViewHolder stickyFakeViewHolder = (TemplateViewHolder) stickyFakeView.getTag();
        for(int i=0; i<recyclerView.getChildCount(); i++){
            View itemView = recyclerView.getChildAt(i);
            TemplateViewHolder itemHolder = (TemplateViewHolder) recyclerView.getChildViewHolder(itemView);
            if(itemHolder == null){
                continue;
            }
            int adapterPosition = itemHolder.getAdapterPosition();
            if(!stickyPositions.contains(adapterPosition)){
                continue;
            }
            if(adapterPosition == stickyFakeViewHolder.getHolderPosition()){
                if(itemView.getVisibility() != View.INVISIBLE) {
                    itemView.setVisibility(View.INVISIBLE);
                }
            }else{
                if(itemView.getVisibility() != View.VISIBLE) {
                    itemView.setVisibility(View.VISIBLE);
                }
            }
        }

        if(firstVisibleItemHolder.getComponent().isSticky()){
            if(firstVisibleItemHolder.itemView.getY() < 0){
                if(firstVisibleItemHolder.itemView.getVisibility() != View.INVISIBLE) {
                    firstVisibleItemHolder.itemView.setVisibility(View.INVISIBLE);
                }
                if(stickyFakeView.getVisibility() != View.VISIBLE) {
                    stickyFakeView.setVisibility(View.VISIBLE);
                }
                stickyFakeView.bringToFront();
            }else{
                if(firstVisibleItemHolder.itemView.getVisibility() != View.VISIBLE) {
                    firstVisibleItemHolder.itemView.setVisibility(View.VISIBLE);
                }
                if(stickyFakeView.getVisibility() != View.GONE) {
                    stickyFakeView.setVisibility(View.GONE);
                }
            }
        }else{
            if(stickyFakeView.getVisibility() != View.VISIBLE){
                stickyFakeView.setVisibility(View.VISIBLE);
            }
        }

        //handle sticky is related
        int nextVisiblePostion = firstVisiblePosition + 1;
        if(!stickyPositions.contains(nextVisiblePostion)){
            if(stickyFakeViewHolder.itemView.getTranslationY() < 0){
                stickyFakeViewHolder.itemView.setTranslationY(0);
            }
            return;
        }
        TemplateViewHolder nextStickyHolder = (TemplateViewHolder) recyclerView.findViewHolderForAdapterPosition(nextVisiblePostion);
        if(nextStickyHolder == null
                || nextStickyHolder.getComponent() == null){
            return;
        }
        int translationY = (int)(nextStickyHolder.itemView.getY() - stickyFakeViewHolder.itemView.getMeasuredHeight());
        if(translationY <= 0){
            stickyFakeViewHolder.itemView.setTranslationY(translationY);
        }else{
            stickyFakeViewHolder.itemView.setTranslationY(0);
        }
    }

    public List<Integer> getStickyPositions() {
        if(stickyPositions == null){
            stickyPositions = new ArrayList<>();
        }
        return stickyPositions;
    }
}
