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
        stickyPositions.add(0);
        stickyPositions.add(11);
    }

    /**
     * dispatch scroll event
     * */
    public void onBeforeScroll(int dx, int dy) {
        if(stickyPositions == null || stickyPositions.size() == 0){
           // return;
        }
        BounceRecyclerView bounceRecyclerView = recyclerTemplateList.getHostView();
        RecyclerView recyclerView = recyclerTemplateList.getHostView().getInnerView();
        RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
        int position = -1;
        if (layoutManager instanceof LinearLayoutManager) {
            position = ((LinearLayoutManager) layoutManager).findFirstVisibleItemPosition();
        }else if (layoutManager instanceof StaggeredGridLayoutManager) {
            int [] firstVisibleItemPositions = new int[3];//max 3 column
            position = ((StaggeredGridLayoutManager) layoutManager).findFirstVisibleItemPositions(firstVisibleItemPositions)[0];
        }
        if(position < 0){
            return;
        }

        TemplateViewHolder firstVisibleItemHolder = (TemplateViewHolder) recyclerView.findViewHolderForAdapterPosition(position);
        if(firstVisibleItemHolder == null){
            return;
        }
        View stickyFakeView = bounceRecyclerView.getChildAt(bounceRecyclerView.getChildCount() - 1);
        //firstvisible is sticky
        if(firstVisibleItemHolder.getComponent().isSticky()){
            if(!(stickyFakeView.getTag() instanceof TemplateViewHolder)){
                TemplateViewHolder fakeStickyHolder = stickyHolderCache.get(position);
                if(fakeStickyHolder == null){
                    fakeStickyHolder = recyclerTemplateList.onCreateViewHolder(recyclerView, recyclerTemplateList.getItemViewType(position));
                    stickyHolderCache.put(position, fakeStickyHolder);
                }
                recyclerTemplateList.onBindViewHolder(fakeStickyHolder, position);
                fakeStickyHolder.itemView.setTranslationY(0);
                fakeStickyHolder.itemView.setTag(fakeStickyHolder);
                bounceRecyclerView.addView(fakeStickyHolder.itemView);
                firstVisibleItemHolder.itemView.setVisibility(View.INVISIBLE);
                return;
            }
            TemplateViewHolder stickyFakeViewHolder = (TemplateViewHolder) stickyFakeView.getTag();
            if(stickyFakeViewHolder.getHolderPosition() != position){
                //remove previous sticky header
                bounceRecyclerView.removeView(stickyFakeViewHolder.itemView);
                stickyFakeViewHolder.itemView.setTranslationY(0);
                for(int i=0; i<recyclerView.getChildCount(); i++){
                    View itemView = recyclerView.getChildAt(i);
                    if(itemView.getVisibility() != View.VISIBLE){
                        TemplateViewHolder itemHolder = (TemplateViewHolder) recyclerView.getChildViewHolder(itemView);
                        if(itemHolder == null){
                            continue;
                        }
                        if(itemHolder.getAdapterPosition() == stickyFakeViewHolder.getHolderPosition()){
                            //// FIXME: 2017/8/24
                            itemView.setVisibility(View.VISIBLE);
                        }
                    }
                }

                //create new sticky header for position
                TemplateViewHolder fakeStickyHolder = stickyHolderCache.get(position);
                if(fakeStickyHolder == null){
                    fakeStickyHolder = recyclerTemplateList.onCreateViewHolder(recyclerView, recyclerTemplateList.getItemViewType(position));
                    stickyHolderCache.put(position, fakeStickyHolder);
                }
                recyclerTemplateList.onBindViewHolder(fakeStickyHolder, position);
                fakeStickyHolder.itemView.setTranslationY(0);
                fakeStickyHolder.itemView.setTag(fakeStickyHolder);
                bounceRecyclerView.addView(fakeStickyHolder.itemView);
                firstVisibleItemHolder.itemView.setVisibility(View.INVISIBLE);
                return;
            }

            if(firstVisibleItemHolder.itemView.getY() < 0){
                if(firstVisibleItemHolder.itemView.getVisibility() != View.INVISIBLE) {
                    firstVisibleItemHolder.itemView.setVisibility(View.INVISIBLE);
                }
                if(stickyFakeView.getVisibility() != View.VISIBLE) {
                    stickyFakeView.setVisibility(View.VISIBLE);
                }
                stickyFakeView.bringToFront();
                return;
            }else{
                if(firstVisibleItemHolder.itemView.getVisibility() != View.VISIBLE) {
                    firstVisibleItemHolder.itemView.setVisibility(View.VISIBLE);
                }
                if(stickyFakeView.getVisibility() != View.GONE) {
                    stickyFakeView.setVisibility(View.GONE);
                }
            }
            return;
        }

        //none has sticky, check has sticky previous
        if(!(stickyFakeView.getTag() instanceof  TemplateViewHolder)){
            int preStickyHeaderPosition = -1;
            for (Integer headerPosition : stickyPositions){
                if(headerPosition == null){
                    continue;
                }
                if(headerPosition <= position){
                    preStickyHeaderPosition = Math.max(preStickyHeaderPosition, headerPosition);
                }
            }
            if(preStickyHeaderPosition < 0){
                return;
            }

            //
            //create new sticky header for position
            TemplateViewHolder fakeStickyHolder = stickyHolderCache.get(preStickyHeaderPosition);
            if(fakeStickyHolder == null){
                fakeStickyHolder = recyclerTemplateList.onCreateViewHolder(recyclerView, recyclerTemplateList.getItemViewType(preStickyHeaderPosition));
                stickyHolderCache.put(position, fakeStickyHolder);
            }
            recyclerTemplateList.onBindViewHolder(fakeStickyHolder, position);
            fakeStickyHolder.itemView.setTranslationY(0);
            fakeStickyHolder.itemView.setTag(fakeStickyHolder);
            bounceRecyclerView.addView(fakeStickyHolder.itemView);
            return;
        }

        TemplateViewHolder stickyFakeViewHolder = (TemplateViewHolder) stickyFakeView.getTag();
        if(position < stickyFakeViewHolder.getHolderPosition()){
             //none need sticky, resume list header
            bounceRecyclerView.removeView(stickyFakeViewHolder.itemView);
            stickyFakeViewHolder.itemView.setTranslationY(0);
            for(int i=0; i<recyclerView.getChildCount(); i++){
                View itemView = recyclerView.getChildAt(i);
                if(itemView.getVisibility() != View.VISIBLE){
                    TemplateViewHolder itemHolder = (TemplateViewHolder) recyclerView.getChildViewHolder(itemView);
                    if(itemHolder == null){
                        continue;
                    }
                    if(itemHolder.getAdapterPosition() == stickyFakeViewHolder.getHolderPosition()){
                        //// FIXME: 2017/8/24
                        itemView.setVisibility(View.VISIBLE);
                    }
                }
            }

            //check has previous head, resume previous header.
            int preStickyHeaderPosition = -1;
            for (Integer headerPosition : stickyPositions){
                if(headerPosition == null){
                    continue;
                }
                if(headerPosition <= position){
                    preStickyHeaderPosition = Math.max(preStickyHeaderPosition, headerPosition);
                }
            }
            if(preStickyHeaderPosition < 0){
               return;
            }
            //create new sticky header for position
            TemplateViewHolder fakeStickyHolder = stickyHolderCache.get(preStickyHeaderPosition);
            if(fakeStickyHolder == null){
                fakeStickyHolder = recyclerTemplateList.onCreateViewHolder(recyclerView, recyclerTemplateList.getItemViewType(preStickyHeaderPosition));
                stickyHolderCache.put(position, fakeStickyHolder);
            }
            recyclerTemplateList.onBindViewHolder(fakeStickyHolder, position);
            fakeStickyHolder.itemView.setTranslationY(0);
            fakeStickyHolder.itemView.setTag(fakeStickyHolder);
            bounceRecyclerView.addView(fakeStickyHolder.itemView);
            return;
        }

        //check is match postion
        int matchStickyPosition = -1;
        for (Integer headerPosition : stickyPositions){
            if(headerPosition == null){
                continue;
            }
            if(headerPosition <= position){
                matchStickyPosition = Math.max(matchStickyPosition, headerPosition);
            }
        }
        if(matchStickyPosition != stickyFakeViewHolder.getHolderPosition()){
                //remove previous sticky header
                bounceRecyclerView.removeView(stickyFakeViewHolder.itemView);
                stickyFakeViewHolder.itemView.setTranslationY(0);
                for(int i=0; i<recyclerView.getChildCount(); i++){
                    View itemView = recyclerView.getChildAt(i);
                    if(itemView.getVisibility() != View.VISIBLE){
                        TemplateViewHolder itemHolder = (TemplateViewHolder) recyclerView.getChildViewHolder(itemView);
                        if(itemHolder == null){
                            continue;
                        }
                        if(itemHolder.getAdapterPosition() == stickyFakeViewHolder.getHolderPosition()){
                            //// FIXME: 2017/8/24
                            itemView.setVisibility(View.VISIBLE);
                        }
                    }
                }


                //create new sticky header for position
                TemplateViewHolder fakeStickyHolder = stickyHolderCache.get(matchStickyPosition);
                if(fakeStickyHolder == null){
                    fakeStickyHolder = recyclerTemplateList.onCreateViewHolder(recyclerView, recyclerTemplateList.getItemViewType(position));
                    stickyHolderCache.put(matchStickyPosition, fakeStickyHolder);
                }
                recyclerTemplateList.onBindViewHolder(fakeStickyHolder, matchStickyPosition);
                fakeStickyHolder.itemView.setTranslationY(0);
                fakeStickyHolder.itemView.setTag(fakeStickyHolder);
                bounceRecyclerView.addView(fakeStickyHolder.itemView);
                return;
        }

        //handle sticky is related
        int nextStickyPostion = matchStickyPosition + 1;
        if(!stickyPositions.contains(nextStickyPostion)){
            //return;
        }
        TemplateViewHolder nextStickyHolder = (TemplateViewHolder) recyclerView.findViewHolderForAdapterPosition(nextStickyPostion);
        if(nextStickyHolder == null || nextStickyHolder.getComponent() == null){
            return;
        }
        if(!nextStickyHolder.getComponent().isSticky()){
            if(stickyFakeViewHolder.itemView.getTranslationY() < 0){
                stickyFakeViewHolder.itemView.setTranslationY(0);
            }
            return;
        }

        int translationY = (int)(nextStickyHolder.itemView.getY() - stickyFakeViewHolder.itemView.getMeasuredHeight());
        if(translationY <= 0){
            stickyFakeViewHolder.itemView.setTranslationY(translationY);
        }else{
            stickyFakeViewHolder.itemView.setTranslationY(0);
        }


        /***
        if(nextItemHolder == null || nextItemHolder.getComponent() == null){
            Log.e("weex ", "trans" + nextPosition + "  is null");
            return;
        }



        View transView = firstVisibleItemHolder.itemView;

        Log.e("weex", "weex " + firstVisibleItemHolder.getComponent());

        if(firstVisibleItemHolder.getComponent().isSticky()){
            View stickyView = bounceRecyclerView.getChildAt(bounceRecyclerView.getChildCount() -1);
            if(stickyView.getTag() instanceof  ListBaseViewHolder
                    && ((ListBaseViewHolder) stickyView.getTag()).getAdapterPosition() == position){
                if(transView.getTop() < 0){
                    transView.setVisibility(View.INVISIBLE);
                    stickyView.setVisibility(View.VISIBLE);
                    stickyView.bringToFront();
                }else{
                    transView.setVisibility(View.VISIBLE);
                    stickyView.setVisibility(View.INVISIBLE);
                }
                Log.e("weex", "sticky " + stickyView.getTop()  + " " + stickyView.getTranslationY());
            }else {
                ListBaseViewHolder fakeHolder = recyclerTemplateList.onCreateViewHolder(recyclerView, recyclerTemplateList.getItemViewType(position));
                recyclerTemplateList.onBindViewHolder(fakeHolder, position);
                fakeHolder.itemView.setTag(fakeHolder);
                if(stickyView.getTag() instanceof  ListBaseViewHolder){
                    bounceRecyclerView.removeView(stickyView);
                    for(int i=0; i<recyclerView.getChildCount(); i++){
                        View itemView = recyclerView.getChildAt(i);
                        if(itemView.getVisibility() != View.VISIBLE){
                            ListBaseViewHolder itemHolder = (ListBaseViewHolder) recyclerView.getChildViewHolder(itemView);
                            if(itemHolder.getAdapterPosition() == ((ListBaseViewHolder) stickyView.getTag()).getAdapterPosition()){
                                itemView.setVisibility(View.VISIBLE);
                            }
                        }
                    }
                }
                bounceRecyclerView.addView(fakeHolder.itemView);
                transView.setVisibility(View.INVISIBLE);
                Log.e("weex", "sticky faked");
            }
        }else{
            View stickyView = bounceRecyclerView.getChildAt(bounceRecyclerView.getChildCount() -1);
            if(stickyView  != null && stickyView.getTag() instanceof  ListBaseViewHolder) {
                ListBaseViewHolder fakeHolder = (ListBaseViewHolder) stickyView.getTag();
                if(position < fakeHolder.getAdapterPosition()){
                    bounceRecyclerView.removeView(stickyView);
                    for(int i=0; i<recyclerView.getChildCount(); i++){
                        View itemView = recyclerView.getChildAt(i);
                        if(itemView.getVisibility() != View.VISIBLE){
                            ListBaseViewHolder itemHolder = (ListBaseViewHolder) recyclerView.getChildViewHolder(itemView);
                            if(itemHolder.getAdapterPosition() == fakeHolder.getAdapterPosition()){
                                itemView.setVisibility(View.VISIBLE);
                            }
                        }
                    }
                    //fixme

                    //
                }else{
                    int nextPosition = position + 1;
                    ListBaseViewHolder nextItemHolder = (ListBaseViewHolder) recyclerView.findViewHolderForAdapterPosition(nextPosition);
                    if(nextItemHolder == null || nextItemHolder.getComponent() == null){
                        Log.e("weex ", "trans" + nextPosition + "  is null");
                        return;
                    }
                    if(nextItemHolder.getComponent().isSticky()){
                        Log.e("weex ", "trans" + nextPosition + "  is sticky");
                        int translationY = (int) (nextItemHolder.itemView.getY() - stickyView.getMeasuredHeight());
                        Log.e("weex ", "trans" + translationY + "" +
                                "top " + nextItemHolder.itemView.getTop() +
                                " height " + stickyView.getMeasuredHeight()
                                +"  " + nextItemHolder.itemView.getY());
                        if(translationY < 0){
                            stickyView.setTranslationY(translationY);
                        }
                    }else{
                        Log.e("weex ", "trans" + nextPosition + "  is not sticky");
                    }
                }
            }
            Log.e("weex", "sticky faked11");
        }*/
    }
}
