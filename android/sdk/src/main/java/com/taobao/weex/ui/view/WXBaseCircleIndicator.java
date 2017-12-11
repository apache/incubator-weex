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


import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.FrameLayout;

import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.utils.WXViewUtils;


public class WXBaseCircleIndicator extends FrameLayout implements WXGestureObservable {

  private final Paint mPaintPage = new Paint();
  private final Paint mPaintFill = new Paint();
  private WXGesture wxGesture;
  private WXCircleViewPager mCircleViewPager;

  /**
   * Radius of the circle
   */
  private float radius;
  /**
   * Padding of the circle
   */
  private float circlePadding;
  /**
   * Fill color of unselected circle
   */
  private int pageColor = Color.LTGRAY;
  /**
   * Fill color of the selected circle
   */
  private int fillColor = Color.DKGRAY;
  private int realCurrentItem;

  private OnPageChangeListener mListener;


  public WXBaseCircleIndicator(Context context) {
    super(context);
    init();
  }

  private void init() {
    radius = WXViewUtils.dip2px(5);
    circlePadding = WXViewUtils.dip2px(5);
    pageColor = Color.LTGRAY;
    fillColor = Color.DKGRAY;

    mPaintFill.setStyle(Style.FILL);
    mPaintFill.setAntiAlias(true);
    mPaintPage.setAntiAlias(true);
    mPaintPage.setColor(pageColor);
    mPaintFill.setStyle(Style.FILL);
    mPaintFill.setColor(fillColor);
    this.setWillNotDraw(false);

  }

  /**
   * @param context
   * @param attrs
   */
  public WXBaseCircleIndicator(Context context, AttributeSet attrs) {
    super(context, attrs);
    init();
  }

  /**
   * @param viewPager the mCircleViewPager to set
   */
  public void setCircleViewPager(WXCircleViewPager viewPager) {
    mCircleViewPager = viewPager;
    if (mCircleViewPager != null) {
      if (mListener == null) {
        mListener = new ViewPager.SimpleOnPageChangeListener() {
          @Override
          public void onPageSelected(int position) {
            realCurrentItem = mCircleViewPager.getRealCurrentItem();
            invalidate();
          }
        };
      }
      this.mCircleViewPager.addOnPageChangeListener(mListener);
      this.realCurrentItem = mCircleViewPager.getRealCurrentItem();
      if (realCurrentItem < 0) {
        realCurrentItem = 0;
      }
    }
    requestLayout();
  }

  /**
   * @param radius the radius to set
   */
  public void setRadius(float radius) {
    this.radius = radius;
  }

  /**
   * @param fillColor the fillColor to set
   */
  public void setFillColor(int fillColor) {
    this.fillColor = fillColor;
    mPaintFill.setColor(fillColor);
  }

  public void setPageColor(int pageColor) {
    this.pageColor = pageColor;
    mPaintPage.setColor(pageColor);
  }

  /**
   * @return the realCurrentItem
   */
  public int getRealCurrentItem() {
    return realCurrentItem;
  }

  /**
   * @param realCurrentItem the realCurrentItem to set
   */
  public void setRealCurrentItem(int realCurrentItem) {
    this.realCurrentItem = realCurrentItem;
    invalidate();
  }

  @Override
  public void registerGestureListener(WXGesture wxGesture) {
    this.wxGesture = wxGesture;
  }

  @Override
  public boolean dispatchTouchEvent(MotionEvent event) {
    boolean result = super.dispatchTouchEvent(event);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, event);
    }
    return result;
  }

  @Override
  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);

    float dotWidth = (circlePadding + radius) * 2;

    float firstCenterX = getWidth() / 2 - (dotWidth * (getCount() - 1) / 2);
    float firstCenterY = getHeight() / 2 + getPaddingTop();

    for (int i = 0; i < getCount(); i++) {
      float dx = firstCenterX + dotWidth * i;
      float dy = firstCenterY;
      if (i != realCurrentItem) {
        canvas.drawCircle(dx, dy, radius, mPaintPage);
      } else {
        canvas.drawCircle(dx, dy, radius, mPaintFill);
      }
    }
  }

  @Override
  protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {

    int viewWidth;
    int viewHeight;

    int widthMode = MeasureSpec.getMode(widthMeasureSpec);
    int widthSize = MeasureSpec.getSize(widthMeasureSpec);
    int heightMode = MeasureSpec.getMode(heightMeasureSpec);
    int heightSize = MeasureSpec.getSize(heightMeasureSpec);

    if (widthMode == MeasureSpec.EXACTLY) {
      viewWidth = widthSize;
    } else {
      viewWidth = (int) (getPaddingLeft() + radius * 2 * getCount() + circlePadding * (getCount() - 1) + getPaddingRight()) + 1;
    }

    if (heightMode == MeasureSpec.EXACTLY) {
      viewHeight = heightSize;
    } else {
      viewHeight = (int) (getPaddingTop() + radius * 2 + getPaddingBottom()) + 1;
    }
    setMeasuredDimension(viewWidth, viewHeight);
  }

  /**
   * @return the count
   */
  public int getCount() {
    if (mCircleViewPager == null || mCircleViewPager.getAdapter() == null) {
      return 0;
    }
    return mCircleViewPager.getRealCount();
  }
}
