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
package org.apache.weex.ui.view.border;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.support.annotation.NonNull;
import org.apache.weex.base.FloatUtil;

abstract class BorderCorner {

  final static float SWEEP_ANGLE = 45;
  private float mCornerRadius = 0.0f;
  private float mPreBorderWidth = 0.0f;
  private float mPostBorderWidth = 0.0f;
  private RectF mBorderBox;
  protected float mAngleBisector;

  /**
   * Tell whether this corner has a rounded inner corner.
   * If a corner has a rounded inner corner, it has an outer corner as well.
   */
  private boolean hasInnerCorner = false;

  /**
   * Tell whether this corner has a rounded outer corner.
   */
  private boolean hasOuterCorner = false;

  private float mOvalLeft, mOvalTop, mOvalRight, mOvalBottom;

  private float mRoundCornerStartX, mRoundCornerStartY;
  private float mRoundCornerEndX, mRoundCornerEndY;

  BorderCorner() {
  }

  final void set(float cornerRadius, float preBorderWidth, float postBorderWidth,
                 @NonNull RectF borderBox, float angleBisector) {
    boolean dirty = !FloatUtil.floatsEqual(mCornerRadius, cornerRadius)
        || !FloatUtil.floatsEqual(mPreBorderWidth, preBorderWidth)
        || !FloatUtil.floatsEqual(mPostBorderWidth, postBorderWidth)
        || !FloatUtil.floatsEqual(mAngleBisector, angleBisector)
        || (null != mBorderBox && mBorderBox.equals(borderBox));

    if (dirty) {
      mCornerRadius = cornerRadius;
      mPreBorderWidth = preBorderWidth;
      mPostBorderWidth = postBorderWidth;
      mBorderBox = borderBox;
      mAngleBisector = angleBisector;

      hasOuterCorner = mCornerRadius > 0 && !FloatUtil.floatsEqual(0, mCornerRadius);

      hasInnerCorner = (hasOuterCorner
          && (getPreBorderWidth() >= 0)
          && (getPostBorderWidth() >= 0)
          && (getOuterCornerRadius() > getPreBorderWidth())
          && (getOuterCornerRadius() > getPostBorderWidth()));

      if (hasOuterCorner) {
        prepareOval();
      }
      prepareRoundCorner();
    }
  }

  /** Build oval data */
  abstract protected void prepareOval();

  /** Build corner data */
  abstract protected void prepareRoundCorner();

  public final void drawRoundedCorner(@NonNull Canvas canvas, @NonNull Paint paint, float startAngle) {
    if (this.hasOuterCorner()) {
      /*Due to the problem of hardware-acceleration, border-radius in some case will not
       be rendered if Path.addArc used instead and the following condition met.
       1. hardware-acceleration enabled
       2. System version is Android 4.1
       3. Screen width is 720px.
       http://dotwe.org/weex/421b9ad09fde51c0b49bb56b37fcf955
      */
      if (VERSION.SDK_INT >= VERSION_CODES.LOLLIPOP) {
        canvas.drawArc(mOvalLeft, mOvalTop, mOvalRight, mOvalBottom, startAngle, BorderCorner.SWEEP_ANGLE, false,
            paint);
      } else {
        canvas.drawArc(new RectF(mOvalLeft, mOvalTop, mOvalRight, mOvalBottom), startAngle, BorderCorner.SWEEP_ANGLE,
            false, paint);
      }
    } else {
      canvas.drawLine(getRoundCornerStartX(), getRoundCornerStartY(), getRoundCornerEndX(), getRoundCornerEndY(),
          paint);
    }
  }


  public final float getRoundCornerStartX() {
    return mRoundCornerStartX;
  }

  final void setRoundCornerStartX(float roundCornerStartX) {
    this.mRoundCornerStartX = roundCornerStartX;
  }

  public final float getRoundCornerStartY() {
    return mRoundCornerStartY;
  }

  final void setRoundCornerStartY(float roundCornerStartY) {
    this.mRoundCornerStartY = roundCornerStartY;
  }

  public final float getRoundCornerEndX() {
    return mRoundCornerEndX;
  }

  final void setRoundCornerEndX(float mRoundCornerEndX) {
    this.mRoundCornerEndX = mRoundCornerEndX;
  }

  public final float getRoundCornerEndY() {
    return mRoundCornerEndY;
  }

  final void setRoundCornerEndY(float mRoundCornerEndY) {
    this.mRoundCornerEndY = mRoundCornerEndY;
  }

  final void setOvalLeft(float mOvalLeft) {
    this.mOvalLeft = mOvalLeft;
  }

  final void setOvalTop(float mOvalTop) {
    this.mOvalTop = mOvalTop;
  }

  final void setOvalRight(float mOvalRight) {
    this.mOvalRight = mOvalRight;
  }

  final void setOvalBottom(float mOvalBottom) {
    this.mOvalBottom = mOvalBottom;
  }

  /**
   * Tell whether this corner has a rounded inner corner.
   * If a corner has a rounded inner corner, it has an outer corner as well.
   * @return true for a rounded inner corner, otherwise false.
   */
  boolean hasInnerCorner() {
    return hasInnerCorner;
  }

  /**
   * Tell whether this corner has a rounded outer corner.
   * @return true for a rounded outer corner, otherwise false.
   */
  boolean hasOuterCorner() {
    return hasOuterCorner;
  }

  protected final float getPreBorderWidth() {
    return mPreBorderWidth;
  }

  protected final float getPostBorderWidth() {
    return mPostBorderWidth;
  }

  protected final float getOuterCornerRadius() {
    return mCornerRadius;
  }

  protected final float getAngleBisectorDegree(){
    return mAngleBisector;
  }

  protected final RectF getBorderBox() {
    return mBorderBox;
  }

}
