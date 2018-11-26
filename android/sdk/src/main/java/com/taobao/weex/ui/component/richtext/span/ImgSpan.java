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
package com.taobao.weex.ui.component.richtext.span;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.drawable.Drawable;
import android.text.style.ReplacementSpan;
import android.view.View;

import com.taobao.weex.adapter.IDrawableLoader;


public class ImgSpan extends ReplacementSpan implements IDrawableLoader.StaticTarget {

  private int width, height;
  private Drawable mDrawable;
  private View mView;

  public ImgSpan(int width, int height) {
    this.width = width;
    this.height = height;
  }

  /**
   * Mostly copied from
   *
   * {@link android.text.style.DynamicDrawableSpan#getSize(Paint, CharSequence, int, int, Paint.FontMetricsInt)},
   * but not use Drawable to calculate size;
   */
  @Override
  public int getSize(Paint paint, CharSequence text, int start, int end, Paint.FontMetricsInt fm) {
    if (fm != null) {
      fm.ascent = -height;
      fm.descent = 0;

      fm.top = fm.ascent;
      fm.bottom = 0;
    }
    return width;
  }

  /**
   * Mostly copied from
   * {@link android.text.style.DynamicDrawableSpan#draw(Canvas, CharSequence, int, int, float, int, int, int, Paint)},
   * except for vertical alignment.
   */
  @Override
  public void draw(Canvas canvas, CharSequence text, int start, int end, float x, int top, int y, int bottom, Paint paint) {
    if (mDrawable != null) {
      canvas.save();
      int transY = bottom - mDrawable.getBounds().bottom;
      transY -= paint.getFontMetricsInt().descent;
      canvas.translate(x, transY);
      mDrawable.draw(canvas);
      canvas.restore();
    }
  }

  @Override
  public void setDrawable(Drawable drawable, boolean resetBounds) {
    mDrawable = drawable;
    if(resetBounds) {
      mDrawable.setBounds(0, 0, width, height);
    }
    setCallback();
    mDrawable.invalidateSelf();
  }

  public void setView(View view) {
    mView = view;
    setCallback();
  }

  private void setCallback() {
    if (mDrawable != null && mView != null) {
      mDrawable.setCallback(mView);
    }
  }
}
