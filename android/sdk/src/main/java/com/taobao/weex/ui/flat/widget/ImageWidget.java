/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.ui.flat.widget;


import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import android.widget.ImageView.ScaleType;
import com.taobao.weex.adapter.IDrawableLoader.AnimatedTarget;
import com.taobao.weex.adapter.IDrawableLoader.StaticTarget;
import com.taobao.weex.ui.component.WXImage;
import com.taobao.weex.ui.flat.FlatGUIIContext;
import com.taobao.weex.utils.ImageDrawable;
import java.util.Arrays;

@RestrictTo(Scope.LIBRARY)
public class ImageWidget extends BaseWidget implements StaticTarget, AnimatedTarget {

  //TODO WXImage.src.readyToRender
  //TODO blur
  private Drawable mImageDrawable;
  private ScaleType mScaleType = WXImage.DEFAULT_SCALE_TYPE;
  private float[] borderRadius;

  public ImageWidget(@NonNull FlatGUIIContext context) {
    super(context);
  }

  @Override
  public void onDraw(@NonNull Canvas canvas) {
    if (mImageDrawable != null) {
      mImageDrawable.draw(canvas);
    }
  }

  @Override
  public void setAnimatedDrawable(@Nullable Drawable drawable) {
    mImageDrawable = drawable;
    initDrawable();
  }

  @Override
  public void setDrawable(@Nullable Drawable drawable, boolean resetBounds) {
    setDrawable(drawable);
  }

  public void setDrawable(@Nullable Drawable imageDrawable) {
    //As there is a translate operation in BaseWidget,
    //width and height of the widget instead of borderBox should be passed to ImageDrawable.
    mImageDrawable = ImageDrawable
        .createImageDrawable(imageDrawable, mScaleType, null,
            getBorderBox().width() - getLeftOffset() - getRightOffset(),
            getBorderBox().height() - getTopOffset() - getBottomOffset(), false);
    if (mImageDrawable instanceof ImageDrawable) {
      if (!Arrays.equals(((ImageDrawable) mImageDrawable).getCornerRadii(), borderRadius)) {
        ((ImageDrawable) mImageDrawable).setCornerRadii(borderRadius);
      }
    }
    initDrawable();
  }

  public void setScaleType(ScaleType scaleType) {
    this.mScaleType = scaleType;
    setDrawable(mImageDrawable);
  }

  public void setBorderRadius(float[] borderRadius) {
    this.borderRadius = borderRadius;
  }

  public @Nullable Drawable getDrawable(){
    return mImageDrawable;
  }

  private void initDrawable(){
    if(mImageDrawable!=null) {
      mImageDrawable.setBounds(0, 0, getBorderBox().width(), getBorderBox().height());
      setCallback(mImageDrawable);
      invalidate();
    }
  }
}
