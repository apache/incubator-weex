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
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.support.annotation.Keep;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.taobao.weex.ui.component.WXImage;
import com.taobao.weex.ui.view.gesture.WXGesture;
import com.taobao.weex.ui.view.gesture.WXGestureObservable;
import com.taobao.weex.utils.ImageDrawable;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.ref.WeakReference;
import java.util.Arrays;

public class WXImageView extends ImageView implements WXGestureObservable,
                                                      IRenderStatus<WXImage>,
                                                      IRenderResult<WXImage>, WXImage.Measurable {

  private WeakReference<WXImage> mWeakReference;
  private WXGesture wxGesture;
  private float[] borderRadius;
  private boolean gif;
  private boolean isBitmapReleased = false;
  private boolean enableBitmapAutoManage = true;


  public WXImageView(Context context) {
    super(context);
  }

  @Override
  public void setImageResource(int resId) {
    Drawable drawable = getResources().getDrawable(resId);
    setImageDrawable(drawable);
  }

  public void setImageDrawable(@Nullable Drawable drawable, boolean isGif) {
    this.gif = isGif;
    ViewGroup.LayoutParams layoutParams;
    if ((layoutParams = getLayoutParams()) != null) {
      Drawable wrapDrawable = ImageDrawable.createImageDrawable(drawable,
                                                                getScaleType(), borderRadius,
                                                                layoutParams.width - getPaddingLeft() - getPaddingRight(),
                                                                layoutParams.height - getPaddingTop() - getPaddingBottom(),
                                                                isGif);
      if (wrapDrawable instanceof ImageDrawable) {
        ImageDrawable imageDrawable = (ImageDrawable) wrapDrawable;
        if (!Arrays.equals(imageDrawable.getCornerRadii(), borderRadius)) {
          imageDrawable.setCornerRadii(borderRadius);
        }
      }
      super.setImageDrawable(wrapDrawable);
      if (mWeakReference != null) {
        WXImage component = mWeakReference.get();
        if (component != null) {
          component.readyToRender();
        }
      }
    }
  }

  @Override
  public void setImageDrawable(@Nullable Drawable drawable) {
    setImageDrawable(drawable, false);
  }

  @Override
  public void setImageBitmap(@Nullable Bitmap bm) {
    setImageDrawable(bm == null ? null : new BitmapDrawable(getResources(), bm));
  }

  @Override
  public void registerGestureListener(WXGesture wxGesture) {
    this.wxGesture = wxGesture;
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    boolean result = super.onTouchEvent(event);
    if (wxGesture != null) {
      result |= wxGesture.onTouch(this, event);
    }
    return result;
  }

  public void setBorderRadius(@NonNull float[] borderRadius) {
    this.borderRadius = borderRadius;
  }

  @Override
  protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
    super.onLayout(changed, left, top, right, bottom);
    if (changed) {
      setImageDrawable(getDrawable(), gif);
    }
  }

  @Override
  public void holdComponent(WXImage component) {
    mWeakReference = new WeakReference<>(component);
  }

  @Nullable
  @Override
  public WXImage getComponent() {
    return null != mWeakReference ? mWeakReference.get() : null;
  }

  @Override
  public int getNaturalWidth() {
    Drawable drawable = getDrawable();
    if (drawable != null) {
      if (drawable instanceof ImageDrawable) {
        return ((ImageDrawable) drawable).getBitmapWidth();
      } else if (drawable instanceof BitmapDrawable) {
        Bitmap bitmap = ((BitmapDrawable) drawable).getBitmap();
        if (bitmap != null) {
          return bitmap.getWidth();
        } else {
          WXLogUtils.w("WXImageView", "Bitmap on " + drawable.toString() + " is null");
        }
      } else {
        WXLogUtils.w("WXImageView", "Not supported drawable type: " + drawable.getClass().getSimpleName());
      }
    }
    return -1;
  }

  @Override
  public int getNaturalHeight() {
    Drawable drawable = getDrawable();
    if (drawable != null) {
      if (drawable instanceof ImageDrawable) {
        return ((ImageDrawable) drawable).getBitmapHeight();
      } else if (drawable instanceof BitmapDrawable) {
        Bitmap bitmap = ((BitmapDrawable) drawable).getBitmap();
        if (bitmap != null) {
          return bitmap.getHeight();
        } else {
          WXLogUtils.w("WXImageView", "Bitmap on " + drawable.toString() + " is null");
        }
      } else {
        WXLogUtils.w("WXImageView", "Not supported drawable type: " + drawable.getClass().getSimpleName());
      }
    }
    return -1;
  }

  private boolean mOutWindowVisibilityChangedReally;
  @Override
  public void dispatchWindowVisibilityChanged(int visibility) {
    mOutWindowVisibilityChangedReally = true;
    super.dispatchWindowVisibilityChanged(visibility);
    mOutWindowVisibilityChangedReally = false;
  }

  @Override
  protected void onWindowVisibilityChanged(int visibility) {
    super.onWindowVisibilityChanged(visibility);
    if(mOutWindowVisibilityChangedReally){
      if(visibility == View.VISIBLE){
         autoRecoverImage();
      }else{
         autoReleaseImage();
      }
    }
  }
  

  @Override
  protected void onAttachedToWindow() {
    super.onAttachedToWindow();
    autoRecoverImage();
  }

  @Override
  protected void onDetachedFromWindow() {
    super.onDetachedFromWindow();
    autoReleaseImage();

  }


  @Override
  public void onStartTemporaryDetach () {
    super.onStartTemporaryDetach();
    autoReleaseImage();

  }


  @Override
  public void onFinishTemporaryDetach () {
    super.onFinishTemporaryDetach();
    autoRecoverImage();
  }


  public void setEnableBitmapAutoManage(boolean enableBitmapAutoManage) {
     this.enableBitmapAutoManage = enableBitmapAutoManage;
  }

  public void autoReleaseImage(){
      if(enableBitmapAutoManage) {
        if (!isBitmapReleased) {
          isBitmapReleased = true;
          WXImage image = getComponent();
          if (image != null) {
            image.autoReleaseImage();
          }
        }
      }
  }

  public void autoRecoverImage(){
    if(enableBitmapAutoManage){
      if(isBitmapReleased){
        WXImage image = getComponent();
        if(image != null){
          image.autoRecoverImage();
        }
        isBitmapReleased = false;
      }
    }
  }




}
