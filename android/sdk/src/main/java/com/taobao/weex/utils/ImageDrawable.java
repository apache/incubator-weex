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

package com.taobao.weex.utils;

import android.graphics.Bitmap;
import android.graphics.BitmapShader;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.PaintDrawable;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.widget.ImageView;

public class ImageDrawable extends PaintDrawable {

  public static Drawable createImageDrawable(@Nullable Drawable original,
                                             @NonNull ImageView.ScaleType scaleType,
                                             @Nullable float[] borderRadius,
                                             int vWidth,
                                             int vHeight,
                                             boolean gif) {
    Bitmap bm;
    if (!gif && vWidth > 0 && vHeight > 0) {
      if (original instanceof BitmapDrawable &&
          (bm = ((BitmapDrawable) original).getBitmap()) != null) {
        ImageDrawable imageDrawable;
        imageDrawable = new ImageDrawable();
        imageDrawable.bitmapWidth = bm.getWidth();
        imageDrawable.bitmapHeight = bm.getHeight();
        BitmapShader bitmapShader = new BitmapShader(bm, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);
        updateShaderAndSize(scaleType, vWidth, vHeight, imageDrawable, bitmapShader);
        imageDrawable.getPaint().setShader(bitmapShader);
        return imageDrawable;
      } else if (original instanceof ImageDrawable) {
        ImageDrawable imageDrawable = (ImageDrawable) original;
        if (imageDrawable.getPaint() != null &&
            imageDrawable.getPaint().getShader() instanceof BitmapShader) {
          BitmapShader bitmapShader = (BitmapShader) imageDrawable.getPaint().getShader();
          updateShaderAndSize(scaleType, vWidth, vHeight, imageDrawable, bitmapShader);
          return imageDrawable;
        }
      }
    }
    return original;
  }

  private static void updateShaderAndSize(@NonNull ImageView.ScaleType scaleType, int vWidth, int vHeight, ImageDrawable imageDrawable, BitmapShader bitmapShader) {
    Matrix matrix = createShaderMatrix(scaleType, vWidth, vHeight,
                                       imageDrawable.bitmapWidth,
                                       imageDrawable.bitmapHeight);
    imageDrawable.setIntrinsicWidth(vWidth);
    imageDrawable.setIntrinsicHeight(vHeight);
    bitmapShader.setLocalMatrix(matrix);
  }



  @NonNull
  private static Matrix createShaderMatrix(@NonNull ImageView.ScaleType scaleType, int vWidth,
                                           int vHeight, int bmWidth, int bmHeight) {
    //Refer ImageView#configureBounds()
    float scale, translateX = 0, translateY = 0;
    if (bmWidth * vHeight > bmHeight * vWidth) {
      scale = vHeight / (float) bmHeight;
      translateX = (vWidth - bmWidth * scale) * 0.5f;
    } else {
      scale = vWidth / (float) bmWidth;
      translateY = (vHeight - bmHeight * scale) * 0.5f;
    }

    Matrix mMatrix = new Matrix();
    if (scaleType == ImageView.ScaleType.FIT_XY) {
      mMatrix.setScale(vWidth / (float) bmWidth, vHeight / (float) bmHeight);
    } else if (scaleType == ImageView.ScaleType.FIT_CENTER) {
      RectF src = new RectF(0, 0, bmWidth, bmHeight);
      RectF dist = new RectF(0, 0, vWidth, vHeight);
      mMatrix.setRectToRect(src, dist, Matrix.ScaleToFit.CENTER);
    } else if (scaleType == ImageView.ScaleType.CENTER_CROP) {
      mMatrix.setScale(scale, scale);
      mMatrix.postTranslate(translateX + 0.5f, translateY + 0.5f);
    }
    return mMatrix;
  }

  private float[] radii;
  private int bitmapHeight;
  private int bitmapWidth;

  private ImageDrawable() {

  }

  @Override
  public void setCornerRadii(float[] radii) {
    this.radii = radii;
    super.setCornerRadii(radii);
  }

  public
  @Nullable
  float[] getCornerRadii() {
    return this.radii;
  }

  public int getBitmapHeight() {
    return bitmapHeight;
  }

  public int getBitmapWidth() {
    return bitmapWidth;
  }

}
