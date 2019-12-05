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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

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
import android.util.SparseIntArray;
import org.apache.weex.dom.CSSShorthand;
import org.apache.weex.dom.CSSShorthand.CORNER;
import org.apache.weex.dom.CSSShorthand.EDGE;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXViewUtils;
import org.apache.weex.ui.component.WXComponent;

import static org.apache.weex.dom.CSSShorthand.CORNER.ALL;
import static org.apache.weex.dom.CSSShorthand.CORNER.BORDER_BOTTOM_LEFT;
import static org.apache.weex.dom.CSSShorthand.CORNER.BORDER_BOTTOM_RIGHT;
import static org.apache.weex.dom.CSSShorthand.CORNER.BORDER_TOP_LEFT;
import static org.apache.weex.dom.CSSShorthand.CORNER.BORDER_TOP_RIGHT;

/**
 * A subclass of
 * {@link Drawable} used for background of {@link WXComponent}.
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
  public static final int BORDER_RADIUS_ALL = 5;
  static final int DEFAULT_BORDER_COLOR = Color.BLACK;
  static final float DEFAULT_BORDER_WIDTH = 0;
  private static final BorderStyle DEFAULT_BORDER_STYLE = BorderStyle.SOLID;
  private static final String TAG = "Border";
  private final Paint mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
  private static BorderStyle[] sBorderStyle = BorderStyle.values();

  private
  @Nullable
  CSSShorthand<EDGE> mBorderWidth;
  private
  @Nullable
  CSSShorthand<CORNER> mBorderRadius;
  private
  @Nullable
  CSSShorthand<CORNER> mOverlappingBorderRadius;
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
  private Shader mShader = null;
  private int mAlpha = 255;

  private TopLeftCorner mTopLeftCorner;
  private TopRightCorner mTopRightCorner;
  private BottomRightCorner mBottomRightCorner;
  private BottomLeftCorner mBottomLeftCorner;

  private final BorderEdge mBorderEdge = new BorderEdge();

  private RectF mRectBounds;

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
    return mShader != null ? PixelFormat.OPAQUE :
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

  public void setBorderWidth(CSSShorthand.EDGE edge, float width) {
    if (mBorderWidth == null) {
      mBorderWidth = new CSSShorthand<>();
    }
    if (mBorderWidth.get(edge) != width) {
      mBorderWidth.set(edge, width);
      mNeedUpdatePath = true;
      invalidateSelf();
    }
  }

  float getBorderWidth(CSSShorthand.EDGE edge) {
    return mBorderWidth.get(edge);
  }

  public void setBorderRadius(CORNER position, float radius) {
    if (mBorderRadius == null) {
      mBorderRadius = new CSSShorthand<>();
    }
    if (mBorderRadius.get(position) != radius ||
        (position == ALL &&
            (radius != mBorderRadius.get(BORDER_TOP_LEFT) ||
            radius != mBorderRadius.get(BORDER_TOP_RIGHT) ||
            radius != mBorderRadius.get(BORDER_BOTTOM_RIGHT) ||
            radius != mBorderRadius.get(BORDER_BOTTOM_LEFT)))) {
      mBorderRadius.set(position, radius);
      mNeedUpdatePath = true;
      invalidateSelf();
    }
  }

  public
  @NonNull
  float[] getBorderRadius(RectF borderBox) {
    prepareBorderRadius(borderBox);
    if (mOverlappingBorderRadius == null) {
      mOverlappingBorderRadius = new CSSShorthand<>();
    }
    float topLeftRadius = mOverlappingBorderRadius.get(BORDER_TOP_LEFT);
    float topRightRadius = mOverlappingBorderRadius.get(BORDER_TOP_RIGHT);
    float bottomRightRadius = mOverlappingBorderRadius.get(BORDER_BOTTOM_RIGHT);
    float bottomLeftRadius = mOverlappingBorderRadius.get(BORDER_BOTTOM_LEFT);
    return new float[]{topLeftRadius, topLeftRadius,
            topRightRadius, topRightRadius,
            bottomRightRadius, bottomRightRadius,
            bottomLeftRadius, bottomLeftRadius};
  }

  public
  @NonNull
  float[] getBorderInnerRadius(RectF borderBox) {
    prepareBorderRadius(borderBox);
    if (mOverlappingBorderRadius == null) {
      mOverlappingBorderRadius = new CSSShorthand<>();
    }
    float topLeftRadius = mOverlappingBorderRadius.get(BORDER_TOP_LEFT);
    float topRightRadius = mOverlappingBorderRadius.get(BORDER_TOP_RIGHT);
    float bottomRightRadius = mOverlappingBorderRadius.get(BORDER_BOTTOM_RIGHT);
    float bottomLeftRadius = mOverlappingBorderRadius.get(BORDER_BOTTOM_LEFT);
    if (null != mBorderWidth) {
      topLeftRadius = Math.max(topLeftRadius - mBorderWidth.get(EDGE.TOP), 0);
      topRightRadius = Math.max(topRightRadius - mBorderWidth.get(EDGE.TOP), 0);
      bottomRightRadius = Math.max(bottomRightRadius - mBorderWidth.get(EDGE.BOTTOM), 0);
      bottomLeftRadius = Math.max(bottomLeftRadius - mBorderWidth.get(EDGE.BOTTOM), 0);
    }
    return new float[] {topLeftRadius, topLeftRadius,
        topRightRadius, topRightRadius,
        bottomRightRadius, bottomRightRadius,
        bottomLeftRadius, bottomLeftRadius};
  }

  public void setBorderColor(CSSShorthand.EDGE edge, int color) {
    if (mBorderColor == null) {
      mBorderColor = new SparseIntArray(5);
      mBorderColor.put(CSSShorthand.EDGE.ALL.ordinal(), DEFAULT_BORDER_COLOR);
    }
    if (getBorderColor(edge) != color) {
      BorderUtil.updateSparseArray(mBorderColor, edge.ordinal(), color);
      invalidateSelf();
    }
  }

  int getBorderColor(CSSShorthand.EDGE edge) {
    return BorderUtil.fetchFromSparseArray(mBorderColor, edge.ordinal(), DEFAULT_BORDER_COLOR);
  }

  public void setBorderStyle(CSSShorthand.EDGE edge, @NonNull String style) {
    if (mBorderStyle == null) {
      mBorderStyle = new SparseIntArray(5);
      mBorderStyle.put(CSSShorthand.EDGE.ALL.ordinal(), DEFAULT_BORDER_STYLE.ordinal());
    }
    try {
      int borderStyle = BorderStyle.valueOf(style.toUpperCase(Locale.US)).ordinal();
      if (getBorderStyle(edge) != borderStyle) {
        BorderUtil.updateSparseArray(mBorderStyle, edge.ordinal(), borderStyle);
        invalidateSelf();
      }
    } catch (IllegalArgumentException e) {
      WXLogUtils.e(TAG, WXLogUtils.getStackTrace(e));
    }
  }

  int getBorderStyle(CSSShorthand.EDGE edge) {
    return BorderUtil.fetchFromSparseArray(mBorderStyle, edge.ordinal(), BorderStyle.SOLID.ordinal());
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

  public boolean hasImage() {
    return mShader != null;
  }

  public boolean isRounded() {
    return mBorderRadius != null &&
            (mBorderRadius.get(BORDER_TOP_LEFT) != 0 ||
                mBorderRadius.get(BORDER_TOP_RIGHT) != 0 ||
                mBorderRadius.get(BORDER_BOTTOM_RIGHT) != 0 ||
                mBorderRadius.get(BORDER_BOTTOM_LEFT) != 0);
  }

  public
  @NonNull
  Path getContentPath(@NonNull RectF borderBox) {
    Path contentClip = new Path();
    prepareBorderPath(0, 0, 0, 0, borderBox, contentClip);
    return contentClip;
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
      if (mOverlappingBorderRadius == null) {
        mOverlappingBorderRadius = new CSSShorthand<>();
      }
      float topLeftRadius = mOverlappingBorderRadius.get(BORDER_TOP_LEFT);
      float topRightRadius = mOverlappingBorderRadius.get(BORDER_TOP_RIGHT);
      float bottomRightRadius = mOverlappingBorderRadius.get(BORDER_BOTTOM_RIGHT);
      float bottomLeftRadius = mOverlappingBorderRadius.get(BORDER_BOTTOM_LEFT);
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
        mOverlappingBorderRadius = new CSSShorthand<>();
      }
      if (!Float.isNaN(factor) && factor < 1) {
        mOverlappingBorderRadius.set(BORDER_TOP_LEFT,
            mBorderRadius.get(BORDER_TOP_LEFT) * factor);
        mOverlappingBorderRadius.set(BORDER_TOP_RIGHT,
            mBorderRadius.get(BORDER_TOP_RIGHT) * factor);
        mOverlappingBorderRadius.set(BORDER_BOTTOM_RIGHT,
            mBorderRadius.get(BORDER_BOTTOM_RIGHT) * factor);
        mOverlappingBorderRadius.set(BORDER_BOTTOM_LEFT,
            mBorderRadius.get(BORDER_BOTTOM_LEFT) * factor);
      } else {
        mOverlappingBorderRadius.set(BORDER_TOP_LEFT,
            mBorderRadius.get(BORDER_TOP_LEFT));
        mOverlappingBorderRadius.set(BORDER_TOP_RIGHT,
            mBorderRadius.get(BORDER_TOP_RIGHT));
        mOverlappingBorderRadius.set(BORDER_BOTTOM_RIGHT,
            mBorderRadius.get(BORDER_BOTTOM_RIGHT));
        mOverlappingBorderRadius.set(BORDER_BOTTOM_LEFT,
            mBorderRadius.get(BORDER_BOTTOM_LEFT));
      }
    }
  }

  private float getScaleFactor(@NonNull RectF borderBox) {
    final float topRadius = mBorderRadius.get(BORDER_TOP_LEFT)
            + mBorderRadius.get(BORDER_TOP_RIGHT);
    final float rightRadius = mBorderRadius.get(BORDER_TOP_RIGHT)
            + mBorderRadius.get(BORDER_BOTTOM_RIGHT);
    final float bottomRadius = mBorderRadius.get(BORDER_BOTTOM_RIGHT)
            + mBorderRadius.get(BORDER_BOTTOM_LEFT);
    final float leftRadius = mBorderRadius.get(BORDER_BOTTOM_LEFT)
            + mBorderRadius.get(BORDER_TOP_LEFT);
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
    if (denominator != 0) {
      list.add(numerator / denominator);
    }
  }

  private void drawBorders(Canvas canvas) {
    if (mRectBounds == null) {
      mRectBounds = new RectF(getBounds());
    } else {
      mRectBounds.set(getBounds());
    }

    if (mBorderWidth == null)
      return;

    final float leftBorderWidth = mBorderWidth.get(EDGE.LEFT);
    final float topBorderWidth = mBorderWidth.get(EDGE.TOP);
    final float bottomBorderWidth = mBorderWidth.get(EDGE.BOTTOM);
    final float rightBorderWidth = mBorderWidth.get(EDGE.RIGHT);

    if (mTopLeftCorner == null) {
      mTopLeftCorner = new TopLeftCorner();
    }
    mTopLeftCorner.set(getBorderRadius(BORDER_TOP_LEFT), leftBorderWidth, topBorderWidth, mRectBounds);
    if (mTopRightCorner == null) {
      mTopRightCorner = new TopRightCorner();
    }
    mTopRightCorner.set(getBorderRadius(BORDER_TOP_RIGHT), topBorderWidth, rightBorderWidth, mRectBounds);
    if (mBottomRightCorner == null) {
      mBottomRightCorner = new BottomRightCorner();
    }
    mBottomRightCorner.set(getBorderRadius(BORDER_BOTTOM_RIGHT), rightBorderWidth, bottomBorderWidth, mRectBounds);
    if (mBottomLeftCorner == null) {
      mBottomLeftCorner = new BottomLeftCorner();
    }
    mBottomLeftCorner.set(getBorderRadius(BORDER_BOTTOM_LEFT), bottomBorderWidth, leftBorderWidth, mRectBounds);

    drawOneSide(canvas, mBorderEdge.set(mTopLeftCorner, mTopRightCorner, topBorderWidth, EDGE.TOP));
    drawOneSide(canvas, mBorderEdge.set(mTopRightCorner, mBottomRightCorner, rightBorderWidth, EDGE.RIGHT));
    drawOneSide(canvas, mBorderEdge.set(mBottomRightCorner, mBottomLeftCorner, bottomBorderWidth, EDGE.BOTTOM));
    drawOneSide(canvas, mBorderEdge.set(mBottomLeftCorner, mTopLeftCorner, leftBorderWidth, EDGE.LEFT));
  }

  private float getBorderRadius(CORNER position) {
    if (null != mOverlappingBorderRadius) {
      return mOverlappingBorderRadius.get(position);
    } else {
      return 0.0f;
    }
  }

  private void drawOneSide(Canvas canvas, @NonNull BorderEdge borderEdge) {
    if (0 != borderEdge.getBorderWidth()) {
      preparePaint(borderEdge.getEdge());
      borderEdge.drawEdge(canvas, mPaint);
    }
  }

  private void preparePaint(CSSShorthand.EDGE edge) {
    final float borderWidth = mBorderWidth.get(edge);
    final int color = WXViewUtils.multiplyColorAlpha(getBorderColor(edge), mAlpha);
    final BorderStyle borderStyle = sBorderStyle[getBorderStyle(edge)];
    final Shader shader = borderStyle.getLineShader(borderWidth, color, edge);
    mPaint.setShader(shader);
    mPaint.setColor(color);
    mPaint.setStrokeCap(Paint.Cap.ROUND);
  }
}
