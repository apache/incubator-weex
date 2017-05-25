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
package com.taobao.weex.ui.view.border;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.Outline;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.VisibleForTesting;
import android.util.SparseArray;
import android.util.SparseIntArray;

import com.taobao.weex.dom.flex.FloatUtil;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

/**
 * A subclass of
 * {@link Drawable} used for background of {@link com.taobao.weex.ui.component.WXComponent}.
 * It supports drawing background color and borders (including rounded borders) by providing a react
 * friendly API (setter for each of those properties). The implementation tries to allocate as few
 * objects as possible depending on which properties are set. E.g. for views with rounded
 * background/borders we allocate {@code mPathForBorderDrawn} and {@code mTempRectForBorderRadius}.
 * In case when view have a rectangular borders we allocate {@code mBorderWidthResult} and similar.
 * When only background color is set we won't allocate any extra/unnecessary objects.
 */
public class BorderDrawable extends Drawable {

  public static final int BORDER_TOP_LEFT_RADIUS = 0;
  public static final int BORDER_TOP_RIGHT_RADIUS = 1;
  public static final int BORDER_BOTTOM_RIGHT_RADIUS = 2;
  public static final int BORDER_BOTTOM_LEFT_RADIUS = 3;
  public static final int BORDER_RADIUS_ALL = 8;
  static final int DEFAULT_BORDER_COLOR = Color.BLACK;
  static final float DEFAULT_BORDER_WIDTH = 0;
  private static final float DEFAULT_BORDER_RADIUS = 0;
  private static final BorderStyle DEFAULT_BORDER_STYLE = BorderStyle.SOLID;
  private static final String TAG = "Border";
  private final Paint mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);

  private
  @Nullable
  SparseArray<Float> mBorderWidth;
  private
  @Nullable
  SparseArray<Float> mBorderRadius;
  private
  @Nullable
  SparseArray<Float> mOverlappingBorderRadius;
  private
  @Nullable
  SparseIntArray mBorderColor;
  private
  @Nullable
  SparseIntArray mBorderStyle;

  private
  @Nullable
  Path mPathForBorderOutline;
  private boolean mNeedUpdatePath = false;
  private int mColor = Color.TRANSPARENT;
  /**
   * set background-image linear-gradient
   */
  private Shader mShader=null;
  private int mAlpha = 255;

  public BorderDrawable() {
  }

  @Override
  public void draw(@NonNull Canvas canvas) {
    canvas.save();
    updateBorderOutline();
    //Shader uses alpha as well.
    mPaint.setAlpha(255);
    if (mPathForBorderOutline != null) {
      int useColor = WXViewUtils.multiplyColorAlpha(mColor, mAlpha);
      if (mShader != null) {
        mPaint.setShader(mShader);
        mPaint.setStyle(Paint.Style.FILL);
        canvas.drawPath(mPathForBorderOutline, mPaint);
        mPaint.setShader(null);
      } else if ((useColor >>> 24) != 0) {
        mPaint.setColor(useColor);
        mPaint.setStyle(Paint.Style.FILL);
        canvas.drawPath(mPathForBorderOutline, mPaint);
        mPaint.setShader(null);
      }
    }
    mPaint.setStyle(Paint.Style.STROKE);
    mPaint.setStrokeJoin(Paint.Join.ROUND);
    drawBorders(canvas);
    mPaint.setShader(null);
    canvas.restore();
  }

  @Override
  protected void onBoundsChange(Rect bounds) {
    super.onBoundsChange(bounds);
    mNeedUpdatePath = true;
  }

  @Override
  public void setAlpha(int alpha) {
    if (alpha != mAlpha) {
      mAlpha = alpha;
      invalidateSelf();
    }
  }

  @Override
  public int getAlpha() {
    return mAlpha;
  }

  /**
   * Do not support Color Filter
   */
  @Override
  public void setColorFilter(ColorFilter cf) {

  }

  @Override
  public int getOpacity() {
    return mShader!=null?PixelFormat.OPAQUE:
          WXViewUtils.getOpacityFromColor(WXViewUtils.multiplyColorAlpha(mColor, mAlpha));
  }

  /* Android's elevation implementation requires this to be implemented to know where to draw the
 shadow. */
  @Override
  public void getOutline(@NonNull Outline outline) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      if (mPathForBorderOutline == null) {
        mNeedUpdatePath = true;
      }
      updateBorderOutline();
      outline.setConvexPath(mPathForBorderOutline);
    }
  }

  public void setBorderWidth(@BorderWidthStyleColorType int position, float width) {
    if (mBorderWidth == null) {
      mBorderWidth = new SparseArray<>(5);
      mBorderWidth.put(Spacing.ALL, DEFAULT_BORDER_WIDTH);
    }
    if (!FloatUtil.floatsEqual(getBorderWidth(position), width)) {
      BorderUtil.updateSparseArray(mBorderWidth, position, width);
      mBorderWidth.put(position, width);
      mNeedUpdatePath = true;
      invalidateSelf();
    }
  }

  float getBorderWidth(@BorderWidthStyleColorType int position) {
    return BorderUtil.fetchFromSparseArray(mBorderWidth, position, DEFAULT_BORDER_WIDTH);
  }

  public void setBorderRadius(@BorderRadiusType int position, float radius) {
    if (mBorderRadius == null) {
      mBorderRadius = new SparseArray<>(5);
      mBorderRadius.put(Spacing.ALL, DEFAULT_BORDER_RADIUS);
    }
    if (!FloatUtil.floatsEqual(getBorderRadius(mBorderRadius, position), radius)) {
      BorderUtil.updateSparseArray(mBorderRadius, position, radius, true);
      mNeedUpdatePath = true;
      invalidateSelf();
    }
  }

  @VisibleForTesting
  float getBorderRadius(@BorderRadiusType int position) {
    return getBorderRadius(mOverlappingBorderRadius, position);
  }

  public
  @NonNull
  float[] getBorderRadius(RectF borderBox) {
    prepareBorderRadius(borderBox);
    float topLeftRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_LEFT_RADIUS);
    float topRightRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_RIGHT_RADIUS);
    float bottomRightRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS);
    float bottomLeftRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_BOTTOM_LEFT_RADIUS);
    return new float[]{topLeftRadius,topLeftRadius,
        topRightRadius,topRightRadius,
        bottomRightRadius, bottomRightRadius,
        bottomLeftRadius,bottomLeftRadius};
  }

  public void setBorderColor(@BorderWidthStyleColorType int position, int color) {
    if (mBorderColor == null) {
      mBorderColor = new SparseIntArray(5);
      mBorderColor.put(Spacing.ALL, DEFAULT_BORDER_COLOR);
    }
    if (getBorderColor(position) != color) {
      BorderUtil.updateSparseArray(mBorderColor, position, color);
      invalidateSelf();
    }
  }

  int getBorderColor(@BorderWidthStyleColorType int position) {
    return BorderUtil.fetchFromSparseArray(mBorderColor, position, DEFAULT_BORDER_COLOR);
  }

  public void setBorderStyle(@BorderWidthStyleColorType int position, @NonNull String style) {
    if (mBorderStyle == null) {
      mBorderStyle = new SparseIntArray(5);
      mBorderStyle.put(Spacing.ALL, DEFAULT_BORDER_STYLE.ordinal());
    }
    try {
      int borderStyle = BorderStyle.valueOf(style.toUpperCase(Locale.US)).ordinal();
      if (getBorderStyle(position) != borderStyle) {
        BorderUtil.updateSparseArray(mBorderStyle, position, borderStyle);
        invalidateSelf();
      }
    } catch (IllegalArgumentException e) {
      WXLogUtils.e(TAG, WXLogUtils.getStackTrace(e));
    }
  }

  int getBorderStyle(@BorderWidthStyleColorType int position) {
    return BorderUtil.fetchFromSparseArray(mBorderStyle, position, BorderStyle.SOLID.ordinal());
  }

  public int getColor() {
    return mColor;
  }

  public void setColor(int color) {
    mColor = color;
    invalidateSelf();
  }

  public void setImage(Shader shader) {
    mShader = shader;
    invalidateSelf();
  }

  public boolean hasImage(){
    return mShader!=null;
  }

  public boolean isRounded() {
    return mBorderRadius != null &&
           (!FloatUtil.floatsEqual(getBorderRadius(mBorderRadius, BORDER_TOP_LEFT_RADIUS), 0) ||
            !FloatUtil.floatsEqual(getBorderRadius(mBorderRadius, BORDER_TOP_RIGHT_RADIUS), 0) ||
            !FloatUtil.floatsEqual(getBorderRadius(mBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS), 0) ||
            !FloatUtil.floatsEqual(getBorderRadius(mBorderRadius, BORDER_BOTTOM_LEFT_RADIUS), 0));
  }

  public
  @NonNull
  Path getContentPath(@NonNull RectF borderBox) {
    Path contentClip = new Path();
    prepareBorderPath(0, 0, 0, 0, borderBox, contentClip);
    return contentClip;
  }

  private float getBorderRadius(@Nullable SparseArray<Float> borderRadius, @BorderRadiusType int position) {
    return BorderUtil.fetchFromSparseArray(borderRadius, position, DEFAULT_BORDER_RADIUS);
  }

  private void updateBorderOutline() {
    if (mNeedUpdatePath) {
      mNeedUpdatePath = false;
      if (mPathForBorderOutline == null) {
        mPathForBorderOutline = new Path();
      }
      mPathForBorderOutline.reset();
      prepareBorderPath(0, 0, 0, 0, new RectF(getBounds()), mPathForBorderOutline);
    }
  }

  private void prepareBorderPath(int topPadding,
                                 int rightPadding,
                                 int bottomPadding,
                                 int leftPadding,
                                 @NonNull RectF rectF,
                                 @NonNull Path path) {
    if (mBorderRadius != null) {
      prepareBorderRadius(rectF);
      float topLeftRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_LEFT_RADIUS);
      float topRightRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_RIGHT_RADIUS);
      float bottomRightRadius = getBorderRadius(mOverlappingBorderRadius,
                                                BORDER_BOTTOM_RIGHT_RADIUS);
      float bottomLeftRadius = getBorderRadius(mOverlappingBorderRadius,
                                               BORDER_BOTTOM_LEFT_RADIUS);
      path.addRoundRect(
          rectF,
          new float[]{
              topLeftRadius - leftPadding,
              topLeftRadius - topPadding,
              topRightRadius - rightPadding,
              topRightRadius - topPadding,
              bottomRightRadius - rightPadding,
              bottomRightRadius - bottomPadding,
              bottomLeftRadius - leftPadding,
              bottomLeftRadius - bottomPadding
          },
          Path.Direction.CW);
    } else {
      path.addRect(rectF, Path.Direction.CW);
    }
  }

  /**
   * Process overlapping curve according to https://www.w3.org/TR/css3-background/#corner-overlap .
   */
  private void prepareBorderRadius(@NonNull RectF borderBox) {
    if (mBorderRadius != null) {
      float factor = getScaleFactor(borderBox);
      if (mOverlappingBorderRadius == null) {
        mOverlappingBorderRadius = new SparseArray<>(5);
        mOverlappingBorderRadius.put(Spacing.ALL, 0f);
      }
      if (!Float.isNaN(factor) && factor < 1) {
        mOverlappingBorderRadius.put(BORDER_TOP_LEFT_RADIUS,
                                     getBorderRadius(mBorderRadius, BORDER_TOP_LEFT_RADIUS) *
                                     factor);
        mOverlappingBorderRadius.put(BORDER_TOP_RIGHT_RADIUS,
                                     getBorderRadius(mBorderRadius, BORDER_TOP_RIGHT_RADIUS) *
                                     factor);
        mOverlappingBorderRadius.put(BORDER_BOTTOM_RIGHT_RADIUS,
                                     getBorderRadius(mBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS) *
                                     factor);
        mOverlappingBorderRadius.put(BORDER_BOTTOM_LEFT_RADIUS,
                                     getBorderRadius(mBorderRadius, BORDER_BOTTOM_LEFT_RADIUS) *
                                     factor);
      } else {
        mOverlappingBorderRadius.put(BORDER_TOP_LEFT_RADIUS,
                                     getBorderRadius(mBorderRadius, BORDER_TOP_LEFT_RADIUS));
        mOverlappingBorderRadius.put(BORDER_TOP_RIGHT_RADIUS,
                                     getBorderRadius(mBorderRadius, BORDER_TOP_RIGHT_RADIUS));
        mOverlappingBorderRadius.put(BORDER_BOTTOM_RIGHT_RADIUS,
                                     getBorderRadius(mBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS));
        mOverlappingBorderRadius.put(BORDER_BOTTOM_LEFT_RADIUS,
                                     getBorderRadius(mBorderRadius, BORDER_BOTTOM_LEFT_RADIUS));
      }
    }
  }

  private float getScaleFactor(@NonNull RectF borderBox) {
    final float topRadius = getBorderRadius(mBorderRadius, BORDER_TOP_LEFT_RADIUS)
                            + getBorderRadius(mBorderRadius, BORDER_TOP_RIGHT_RADIUS);
    final float rightRadius = getBorderRadius(mBorderRadius, BORDER_TOP_RIGHT_RADIUS)
                              + getBorderRadius(mBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS);
    final float bottomRadius = getBorderRadius(mBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS)
                               + getBorderRadius(mBorderRadius, BORDER_BOTTOM_LEFT_RADIUS);
    final float leftRadius = getBorderRadius(mBorderRadius, BORDER_BOTTOM_LEFT_RADIUS)
                             + getBorderRadius(mBorderRadius, BORDER_TOP_LEFT_RADIUS);
    List<Float> factors = new ArrayList<>(4);
    updateFactor(factors, borderBox.width(), topRadius);
    updateFactor(factors, borderBox.height(), rightRadius);
    updateFactor(factors, borderBox.width(), bottomRadius);
    updateFactor(factors, borderBox.height(), leftRadius);
    float factor;
    if (factors.isEmpty()) {
      factor = Float.NaN;
    } else {
      factor = Collections.min(factors);
    }
    return factor;
  }

  private void updateFactor(@NonNull List<Float> list, float numerator, float denominator) {
    if (!FloatUtil.floatsEqual(denominator, 0)) {
      list.add(numerator / denominator);
    }
  }

  private void drawBorders(Canvas canvas) {
    RectF rectBounds = new RectF(getBounds());
    BorderCorner topLeft = new TopLeftCorner(
        getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_LEFT_RADIUS),
        getBorderWidth(Spacing.LEFT),
        getBorderWidth(Spacing.TOP),
        rectBounds);
    BorderCorner topRight = new TopRightCorner(
        getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_RIGHT_RADIUS),
        getBorderWidth(Spacing.TOP),
        getBorderWidth(Spacing.RIGHT),
        rectBounds);
    BorderCorner bottomRight = new BottomRightCorner(
        getBorderRadius(mOverlappingBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS),
        getBorderWidth(Spacing.RIGHT),
        getBorderWidth(Spacing.BOTTOM),
        rectBounds);
    BorderCorner bottomLeft = new BottomLeftCorner(
        getBorderRadius(mOverlappingBorderRadius, BORDER_BOTTOM_LEFT_RADIUS),
        getBorderWidth(Spacing.BOTTOM),
        getBorderWidth(Spacing.LEFT),
        rectBounds);
    drawOneSide(canvas, new BorderEdge(topLeft, topRight, Spacing.TOP,
                                       getBorderWidth(Spacing.TOP)));
    drawOneSide(canvas, new BorderEdge(topRight, bottomRight, Spacing.RIGHT,
                                       getBorderWidth(Spacing.RIGHT)));
    drawOneSide(canvas, new BorderEdge(bottomRight, bottomLeft, Spacing.BOTTOM,
                                       getBorderWidth(Spacing.BOTTOM)));
    drawOneSide(canvas, new BorderEdge(bottomLeft, topLeft, Spacing.LEFT,
                                       getBorderWidth(Spacing.LEFT)));
  }

  private void drawOneSide(Canvas canvas, @NonNull BorderEdge borderEdge) {
    if (!FloatUtil.floatsEqual(0, getBorderWidth(borderEdge.getEdge()))) {
      preparePaint(borderEdge.getEdge());
      borderEdge.drawEdge(canvas, mPaint);
    }
  }

  private void preparePaint(@BorderWidthStyleColorType int side) {
    float borderWidth = getBorderWidth(side);
    int color = WXViewUtils.multiplyColorAlpha(getBorderColor(side), mAlpha);
    BorderStyle borderStyle = BorderStyle.values()[getBorderStyle(side)];
    Shader shader = borderStyle.getLineShader(borderWidth, color, side);
    mPaint.setShader(shader);
    mPaint.setColor(color);
    mPaint.setStrokeCap(Paint.Cap.ROUND);
  }
}
