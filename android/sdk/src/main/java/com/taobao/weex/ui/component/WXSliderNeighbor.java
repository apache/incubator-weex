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
package com.taobao.weex.ui.component;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v4.view.ViewPager;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.WXThread;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.view.WXCircleIndicator;
import com.taobao.weex.ui.view.WXCirclePageAdapter;
import com.taobao.weex.ui.view.WXCircleViewPager;
import com.taobao.weex.utils.WXUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.lang.reflect.InvocationTargetException;
import java.util.List;

/**
 * Known Issus: In auto play mode, neighbor view not scaled or aplhaed rarely.
 *
 * Created by xingjiu on 16/8/18.
 */
public class WXSliderNeighbor extends WXSlider {
    public static final String NEIGHBOR_SCALE = "neighborScale"; // the init scale of neighbor page
    public static final String NEIGHBOR_ALPHA = "neighborAlpha"; // the init alpha of neighbor page
    public static final String NEIGHBOR_SPACE = "neighborSpace"; // the init space of neighbor page
    public static final String CURRENT_ITEM_SCALE = "currentItemScale"; // the scale of middle item

    private static final int DEFAULT_NEIGHBOR_SPACE = 25;
    private static final float DEFAULT_NEIGHBOR_SCALE = 0.8F;
    private static final float DEFAULT_NEIGHBOR_ALPHA = 0.6F;
    private static final float DEFAULT_CURRENT_ITEM_SCALE = 0.9F;

    private float mNeighborScale = DEFAULT_NEIGHBOR_SCALE;
    private float mNeighborAlpha = DEFAULT_NEIGHBOR_ALPHA;
    private float mNeighborSpace = DEFAULT_NEIGHBOR_SPACE;
    private float mCurrentItemScale = DEFAULT_CURRENT_ITEM_SCALE;

    private ZoomTransformer mCachedTransformer;

    public WXSliderNeighbor(WXSDKInstance instance, WXDomObject node, WXVContainer parent) {
        super(instance, node, parent);
    }

    public static class Creator implements ComponentCreator {
        public WXComponent createInstance(WXSDKInstance instance, WXDomObject node, WXVContainer parent) throws IllegalAccessException, InvocationTargetException, InstantiationException {
            return new WXSliderNeighbor(instance, node, parent);
        }
    }

    @Override
    public void bindData(WXComponent component) {
        super.bindData(component);
    }

    @Override
    protected FrameLayout initComponentHostView(@NonNull Context context) {
        FrameLayout view = new FrameLayout(context);

        // init view pager
        FrameLayout.LayoutParams pagerParams = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT);
        pagerParams.gravity = Gravity.CENTER;
        mViewPager = new WXCircleViewPager(getContext());
        mViewPager.setLayoutParams(pagerParams);

        // init adapter
        mAdapter = new WXCirclePageAdapter();
        mViewPager.setAdapter(mAdapter);

        // add to parent
        view.addView(mViewPager);
        mViewPager.addOnPageChangeListener(mPageChangeListener);

        mViewPager.setOverScrollMode(View.OVER_SCROLL_NEVER);
        registerActivityStateListener();

        mViewPager.setPageTransformer(false, createTransformer());

        return view;
    }

    ZoomTransformer createTransformer() {
        if(mCachedTransformer == null) {
            mCachedTransformer = new ZoomTransformer();
        }
        return mCachedTransformer;
    }

    @Override
    public void addSubView(View view, final int index) {
        if (view == null || mAdapter == null) {
            return;
        }

        if (view instanceof WXCircleIndicator) {
            return;
        }

        FrameLayout wrapper = new FrameLayout(getContext());
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        params.gravity = Gravity.CENTER;
        view.setLayoutParams(params);
        wrapper.addView(view);
        super.addSubView(wrapper,index);

        updateAdapterScaleAndAlpha(mNeighborAlpha, mNeighborScale); // we need to set neighbor view status when added.

        mViewPager.postDelayed(WXThread.secure(new Runnable() {
            @Override
            public void run() {
                try {
                    if(mViewPager.getRealCount() > 0 && index > 2) { // index > 2 mean more than two times, then need a fake drag
                        // prevent a bug of init status. ZoomTransformer no called as excepted.
                        mViewPager.beginFakeDrag();
                        mViewPager.fakeDragBy(1); // must be 1
                    }
                }catch (IndexOutOfBoundsException e){
                    // do nothing
                } finally {
                    try {
                        mViewPager.endFakeDrag();
                    }catch (Exception e) {
                        // do nothing
                    }
                }
            }
        }), 50);
    }

    private void updateScaleAndAlpha(View view, float alpha, float scale) {
        if(null == view) {
            return;
        }
        if(alpha >= 0 && view.getAlpha() != alpha) {
            view.setAlpha(alpha);
        }
        if(scale >= 0 && view.getScaleX() != scale) {
            view.setScaleX(scale);
            view.setScaleY(scale);
        }
    }

    private void updateAdapterScaleAndAlpha(final float alpha, final float scale) {
        final List<View> pageViews = mAdapter.getViews();
        final int curPos = mViewPager.getCurrentItem();

        if(pageViews.size() > 0) {
            final View currentPage = pageViews.get(curPos);
            updateScaleAndAlpha(((ViewGroup)currentPage).getChildAt(0), 1.0F, mCurrentItemScale);

            if(pageViews.size() < 2) {
                return;
            }
            //make sure View's width & height are measured.
            currentPage.postDelayed(WXThread.secure(new Runnable() {
                @Override
                public void run() {
                    //change left and right page's translation
                    updateNeighbor(currentPage, alpha, scale);

                }
            }), 17);

            // make sure only display view current, left, right.
            int left = (curPos == 0) ? pageViews.size()-1 : curPos-1;
            int right = (curPos == pageViews.size()-1) ? 0 : curPos+1;
            for(int i =0; i<mAdapter.getRealCount(); i++) {
                if(i != left && i != curPos && i != right) {
                    ((ViewGroup)pageViews.get(i)).getChildAt(0).setAlpha(0F);
                }
            }
        }
    }

    private void updateNeighbor(View currentPage, final float alpha, final float scale) {
        final List<View> pageViews = mAdapter.getViews();
        final int curPos = mViewPager.getCurrentItem();

        float translation = calculateTranslation(currentPage);
        int left = (curPos == 0) ? pageViews.size()-1 : curPos-1;
        View leftPage = pageViews.get(left);
        int right = (curPos == pageViews.size()-1) ? 0 : curPos+1;
        View rightPage = pageViews.get(right);

        if(pageViews.size() == 2) {
            if(curPos == 0) {
                moveRight(rightPage, translation, alpha, scale);
            }else if(curPos == 1) {
                moveLeft(leftPage, translation, alpha, scale);
            }
        } else {
            moveLeft(leftPage, translation, alpha, scale);
            moveRight(rightPage, translation, alpha, scale);
        }
    }

    private void moveLeft(View page, float translation, float alpha, float scale) {
        updateScaleAndAlpha(((ViewGroup)page).getChildAt(0), alpha, scale);
        page.setTranslationX(translation);
        ((ViewGroup)page).getChildAt(0).setTranslationX(translation);
    }
    private void moveRight(View page, float translation, float alpha, float scale) {
        moveLeft(page, -translation, alpha, scale);
    }

    @WXComponentProp(name = NEIGHBOR_SCALE)
    public void setNeighborScale(String input) {
        float neighborScale = DEFAULT_NEIGHBOR_SCALE;
        if (!TextUtils.isEmpty(input)) {
            try {
                neighborScale = Float.parseFloat(input);
            } catch (NumberFormatException e) {
            }
        }

        // addSubView is called before setProperty, so we need to modify the neighbor view in mAdapter.
        if(this.mNeighborScale != neighborScale) {
            this.mNeighborScale = neighborScale;
            updateAdapterScaleAndAlpha(-1, neighborScale);
        }
    }

    @WXComponentProp(name = NEIGHBOR_ALPHA)
    public void setNeighborAlpha(String input) {
        float neighborAlpha = DEFAULT_NEIGHBOR_ALPHA;
        if (!TextUtils.isEmpty(input)) {
            try {
                neighborAlpha = Float.parseFloat(input);
            } catch (NumberFormatException e) {
            }
        }

        // The same work as setNeighborScale()
        if(this.mNeighborAlpha != neighborAlpha) {
            this.mNeighborAlpha = neighborAlpha;
            updateAdapterScaleAndAlpha(neighborAlpha, -1);
        }
    }

    @WXComponentProp(name = NEIGHBOR_SPACE)
    @SuppressWarnings("unused")
    public void setNeighborSpace(String input) {
        float neighborSpace = DEFAULT_NEIGHBOR_SPACE;
        if (!TextUtils.isEmpty(input)) {
            try {
                neighborSpace = Float.parseFloat(input);
            } catch (NumberFormatException e) {
            }
        }

        if(this.mNeighborSpace != neighborSpace) {
            this.mNeighborSpace = neighborSpace;
        }
    }

    @WXComponentProp(name = CURRENT_ITEM_SCALE)
    @SuppressWarnings("unused")
    public void setCurrentItemScale(String input) {
        float currentItemScale = DEFAULT_CURRENT_ITEM_SCALE;
        if (!TextUtils.isEmpty(input)) {
            try {
                currentItemScale = Float.parseFloat(input);
            } catch (NumberFormatException e) {
            }
        }

        if(this.mCurrentItemScale != currentItemScale) {
            this.mCurrentItemScale = currentItemScale;
            updateAdapterScaleAndAlpha(-1, -1);
        }
    }

    @Override
    protected boolean setProperty(String key, Object param) {
        String input;
        switch (key) {
            case NEIGHBOR_SCALE:
                input = WXUtils.getString(param, null);
                if (input != null) {
                    setNeighborScale(input);
                }
                return true;
            case NEIGHBOR_ALPHA:
                input = WXUtils.getString(param, null);
                if (input != null) {
                    setNeighborAlpha(input);
                }
                return true;
            case NEIGHBOR_SPACE:
                input = WXUtils.getString(param, null);
                if (input != null) {
                    setNeighborSpace(input);
                }
                return true;
            case CURRENT_ITEM_SCALE:
                input = WXUtils.getString(param, null);
                if (input != null) {
                    setCurrentItemScale(input);
                }
                return true;
        }
        return super.setProperty(key, param);
    }

    /**
     * we need add translation for left and right card view.
     * */
    private float calculateTranslation(@NonNull View hostPage) {
        if(!(hostPage instanceof ViewGroup)) {
            return 0;
        }
        View realView = ((ViewGroup)hostPage).getChildAt(0);
        float translation = (hostPage.getMeasuredWidth()-realView.getMeasuredWidth()*mNeighborScale)/4;
        translation += ((hostPage.getMeasuredWidth()-realView.getMeasuredWidth() * mCurrentItemScale)/2 - WXViewUtils.getRealPxByWidth(mNeighborSpace, getInstance().getInstanceViewPortWidth()))/2 ;
        return translation;
    }

    // Here is the trick.
     class ZoomTransformer implements ViewPager.PageTransformer {
        @Override
        public void transformPage(View page, float position) {
            int pagePosition = mAdapter.getPagePosition(page);
            int curPosition = mViewPager.getCurrentItem();

            int realCount = mAdapter.getRealCount();

            boolean ignore = false;
            if(curPosition != 0 && curPosition != realCount - 1 && Math.abs(pagePosition - curPosition) > 1)  {
                ignore = true;
            }
            if(curPosition == 0 && pagePosition < realCount - 1 && pagePosition > 1) {
                ignore = true;
            }
            if(curPosition == realCount - 1 && pagePosition < realCount - 2 && pagePosition > 0) {
                ignore = true;
            }
            // just transfer the neighbor(left & right) page.
            if(ignore) {
                return;
            }

            View realView = ((ViewGroup)page).getChildAt(0);
            if(realView == null){
                return;
            }
            float alpha, scale;

            if(position <= (-realCount + 1)) {
                position = position + realCount;
            }
            if(position >= realCount - 1) {
                position = position - realCount;
            }

            if (position >= -1 && position <= 1) {
                float factor = Math.abs(Math.abs(position) - 1);
                scale = mNeighborScale + factor * (mCurrentItemScale - mNeighborScale);
                alpha = (1- mNeighborAlpha) * factor + mNeighborAlpha;

                float translation = calculateTranslation(page);
                if(position > 0){
                    translation = (position*translation);
                    realView.setTranslationX(-translation);
                    page.setTranslationX(-translation);
                }else if(position == 0){
                    page.setTranslationX(0);
                    realView.setTranslationX(0);
                    updateAdapterScaleAndAlpha(mNeighborAlpha, mNeighborScale);
                }else{
                    if(realCount == 2 && Math.abs(position) == 1) {
                        return;
                    }
                    translation = (-position*translation);
                    realView.setTranslationX(translation);
                    page.setTranslationX(translation);
                }
                realView.setScaleX(scale);
                realView.setScaleY(scale);
                realView.setAlpha(alpha);
            }

        }
    }

}
