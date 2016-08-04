/**
 * Copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * This source code is licensed under the BSD-style license found in the LICENSE file in the root
 * directory of this source tree. An additional grant of patent rights can be found in the PATENTS
 * file in the same directory.
 */
package com.taobao.weex.ui.view.border;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.Outline;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PathEffect;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.SparseIntArray;

import com.taobao.weex.dom.flex.FloatUtil;
import com.taobao.weex.dom.flex.Spacing;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXViewUtils;

import java.util.Locale;

/**
 * A subclass of {@link Drawable} used for background of {@link com.taobao.weex.ui.component.WXComponent}. It supports
 * drawing background color and borders (including rounded borders) by providing a react friendly
 * API (setter for each of those properties).
 *
 * The implementation tries to allocate as few objects as possible depending on which properties are
 * set. E.g. for views with rounded background/borders we allocate {@code mPathForBorderDrawn} and
 * {@code mTempRectForBorderRadius}. In case when view have a rectangular borders we allocate
 * {@code mBorderWidthResult} and similar. When only background color is set we won't allocate any
 * extra/unnecessary objects.
 */
public class BorderDrawable extends Drawable {

  public final static int BORDER_TOP_LEFT_RADIUS = 0;
  public final static int BORDER_TOP_RIGHT_RADIUS = 1;
  public final static int BORDER_BOTTOM_RIGHT_RADIUS = 2;
  public final static int BORDER_BOTTOM_LEFT_RADIUS = 3;
  public final static int BORDER_RADIUS_ALL = 8;
  private final static String TAG = "Border";
  private final static int DEFAULT_BORDER_COLOR = Color.BLACK;
  private final Paint mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);

  //TODO use layer to draw this. http://www.cnblogs.com/liangstudyhome/p/4143498.html
  private
  @Nullable
  Drawable mBackground;
  private
  @Nullable
  Spacing mBorderWidth;
  private
  @Nullable
  Spacing mBorderRadius;
  private
  @Nullable
  Spacing mOverlappingBorderRadius;
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
  private int mAlpha = 255;

  public BorderDrawable() {
    this(null);
  }

  public BorderDrawable(@Nullable Drawable background) {
    mBackground = background;
  }

  @Override
  public void draw(Canvas canvas) {
    canvas.save();
    updatePath();
    if (mPathForBorderOutline != null) {
      canvas.clipPath(mPathForBorderOutline);
      int useColor = WXViewUtils.multiplyColorAlpha(mColor, mAlpha);
      if ((useColor >>> 24) != 0) {
        mPaint.setStyle(Paint.Style.FILL);
        canvas.drawColor(useColor);
      } else {
        if (mBackground != null) {
          mBackground.draw(canvas);
        }
      }
    }
    mPaint.setStyle(Paint.Style.STROKE);
    mPaint.setStrokeJoin(Paint.Join.ROUND);
    drawBorders(canvas);
    canvas.restore();
    //TODO make the contentSize clip
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
   * @param cf
   */
  @Override
  public void setColorFilter(ColorFilter cf) {

  }

  @Override
  public int getOpacity() {
    return WXViewUtils.getOpacityFromColor(WXViewUtils.multiplyColorAlpha(mColor, mAlpha));
  }

  /* Android's elevation implementation requires this to be implemented to know where to draw the shadow. */
  @Override
  public void getOutline(@NonNull Outline outline) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      if (mPathForBorderOutline == null) {
        mNeedUpdatePath = true;
      }
      updatePath();
      outline.setConvexPath(mPathForBorderOutline);
    }
  }

  @Override
  protected void onBoundsChange(Rect bounds) {
    super.onBoundsChange(bounds);
    mNeedUpdatePath = true;
  }

  public void setBorderWidth(int position, float width) {
    if (mBorderWidth == null) {
      mBorderWidth = new Spacing();
      mBorderWidth.setDefault(Spacing.ALL, 0);
    }
    if (!FloatUtil.floatsEqual(mBorderWidth.getRaw(position), width)) {
      mBorderWidth.set(position, width);
      mNeedUpdatePath = true;
      invalidateSelf();
    }
  }

  public void setBorderRadius(int position, float radius) {
    if (mBorderRadius == null) {
      mBorderRadius = new Spacing();
      mBorderRadius.setDefault(Spacing.ALL, 0);
    }
    if (!FloatUtil.floatsEqual(mBorderRadius.getRaw(position), radius)) {
      mBorderRadius.set(position, radius);
      mNeedUpdatePath = true;
      invalidateSelf();
    }
  }

  public void setBorderColor(int position, int color) {
    if (mBorderColor == null) {
      mBorderColor = new SparseIntArray(5);
      mBorderColor.put(Spacing.ALL, DEFAULT_BORDER_COLOR);
    }
    if (mBorderColor.get(position) != color) {
      mBorderColor.put(position, color);
      invalidateSelf();
    }
  }

  public void setBorderStyle(int position, @NonNull String style) {
    if (mBorderStyle == null) {
      mBorderStyle = new SparseIntArray(4);
      mBorderStyle.put(Spacing.ALL, BorderStyle.SOLID.ordinal());
    }
    try {
      int borderStyle = BorderStyle.valueOf(style.toUpperCase(Locale.US)).ordinal();
      if (mBorderStyle.get(position) != borderStyle) {
        mBorderStyle.put(position, borderStyle);
        invalidateSelf();
      }
    } catch (IllegalArgumentException e) {
      WXLogUtils.e(TAG, WXLogUtils.getStackTrace(e));
    }
  }

  public int getColor() {
    return mColor;
  }

  public void setColor(int color) {
    mColor = color;
    invalidateSelf();
  }

  public
  @NonNull
  Path getContentClip(int viewTopPadding, int viewRightPadding, int viewBottomPadding, int viewLeftPadding) {
    RectF rectForBorderOutline = new RectF();
    Path contentClip = new Path();
    rectForBorderOutline.set(getBounds());
    rectForBorderOutline.top += viewTopPadding;
    rectForBorderOutline.right -= viewRightPadding;
    rectForBorderOutline.bottom -= viewBottomPadding;
    rectForBorderOutline.left += viewLeftPadding;
    if (mBorderRadius != null) {
      prepareBorderRadius();
      float topLeftRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_LEFT_RADIUS);
      float topRightRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_RIGHT_RADIUS);
      float bottomRightRadius = getBorderRadius(mOverlappingBorderRadius,
                                                BORDER_BOTTOM_RIGHT_RADIUS);
      float bottomLeftRadius = getBorderRadius(mOverlappingBorderRadius,
                                               BORDER_BOTTOM_LEFT_RADIUS);
      contentClip.addRoundRect(rectForBorderOutline,
                               new float[]{
                                   topLeftRadius - viewLeftPadding,
                                   topLeftRadius - viewTopPadding,
                                   topRightRadius - viewRightPadding,
                                   topRightRadius - viewTopPadding,
                                   bottomRightRadius - viewRightPadding,
                                   bottomRightRadius - viewBottomPadding,
                                   bottomLeftRadius - viewLeftPadding,
                                   bottomLeftRadius - viewBottomPadding
                               },
                               Path.Direction.CW);
    } else {
      contentClip.addRect(rectForBorderOutline, Path.Direction.CW);
    }
    return contentClip;
  }

  private void updatePath() {
    if (mNeedUpdatePath) {
      mNeedUpdatePath = false;
      if (mPathForBorderOutline == null) {
        mPathForBorderOutline = new Path();
      }
      mPathForBorderOutline.reset();
      RectF mRectForBorderOutline = new RectF();
      mRectForBorderOutline.set(getBounds());

      if (mBorderRadius != null) {
        prepareBorderRadius();
        float topLeftRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_LEFT_RADIUS);
        float topRightRadius = getBorderRadius(mOverlappingBorderRadius, BORDER_TOP_RIGHT_RADIUS);
        float bottomRightRadius = getBorderRadius(mOverlappingBorderRadius,
                                                  BORDER_BOTTOM_RIGHT_RADIUS);
        float bottomLeftRadius = getBorderRadius(mOverlappingBorderRadius,
                                                 BORDER_BOTTOM_LEFT_RADIUS);
        mPathForBorderOutline.addRoundRect(
            mRectForBorderOutline,
            new float[]{
                topLeftRadius, topLeftRadius,
                topRightRadius, topRightRadius,
                bottomRightRadius, bottomRightRadius,
                bottomLeftRadius, bottomLeftRadius
            },
            Path.Direction.CW);
      } else {
        mPathForBorderOutline.addRect(mRectForBorderOutline, Path.Direction.CW);
      }
    }
  }

  private void drawBorders(Canvas canvas) {
    if (mBorderWidth != null) {
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
      drawOneSide(canvas, new BorderEdge(topLeft, topRight, Spacing.TOP));
      drawOneSide(canvas, new BorderEdge(topRight, bottomRight, Spacing.RIGHT));
      drawOneSide(canvas, new BorderEdge(bottomRight, bottomLeft, Spacing.BOTTOM));
      drawOneSide(canvas, new BorderEdge(bottomLeft, topLeft, Spacing.LEFT));
    }
  }

  /**
   * Process overlapping curve according to https://www.w3.org/TR/css3-background/#corner-overlap .
   */
  private void prepareBorderRadius() {
    if (mBorderRadius != null) {
      Rect borderBox = getBounds();
      float top, right, bottom, left, factor;
      top = borderBox.width() / (getBorderRadius(mBorderRadius, BORDER_TOP_LEFT_RADIUS) +
                                 getBorderRadius(mBorderRadius, BORDER_TOP_RIGHT_RADIUS));
      right = borderBox.height() / (getBorderRadius(mBorderRadius, BORDER_TOP_RIGHT_RADIUS) +
                                    getBorderRadius(mBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS));

      bottom = borderBox.width() / (getBorderRadius(mBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS) +
                                    getBorderRadius(mBorderRadius, BORDER_BOTTOM_LEFT_RADIUS));
      left = borderBox.height() / (getBorderRadius(mBorderRadius, BORDER_BOTTOM_LEFT_RADIUS) +
                                   getBorderRadius(mBorderRadius, BORDER_TOP_LEFT_RADIUS));
      factor = Math.min(Math.min(top, right), Math.min(bottom, left));
      if (mOverlappingBorderRadius == null) {
        mOverlappingBorderRadius = new Spacing();
        mOverlappingBorderRadius.setDefault(Spacing.ALL, 0);
      }
      if (factor < 1) {
        mOverlappingBorderRadius.set(BORDER_TOP_LEFT_RADIUS, getBorderRadius(mBorderRadius, BORDER_TOP_LEFT_RADIUS) * factor);
        mOverlappingBorderRadius.set(BORDER_TOP_RIGHT_RADIUS, getBorderRadius(mBorderRadius, BORDER_TOP_RIGHT_RADIUS) * factor);
        mOverlappingBorderRadius.set(BORDER_BOTTOM_RIGHT_RADIUS, getBorderRadius(mBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS) * factor);
        mOverlappingBorderRadius.set(BORDER_BOTTOM_LEFT_RADIUS, getBorderRadius(mBorderRadius, BORDER_BOTTOM_LEFT_RADIUS) * factor);
      } else {
        mOverlappingBorderRadius.set(BORDER_TOP_LEFT_RADIUS, getBorderRadius(mBorderRadius, BORDER_TOP_LEFT_RADIUS));
        mOverlappingBorderRadius.set(BORDER_TOP_RIGHT_RADIUS, getBorderRadius(mBorderRadius, BORDER_TOP_RIGHT_RADIUS));
        mOverlappingBorderRadius.set(BORDER_BOTTOM_RIGHT_RADIUS, getBorderRadius(mBorderRadius, BORDER_BOTTOM_RIGHT_RADIUS));
        mOverlappingBorderRadius.set(BORDER_BOTTOM_LEFT_RADIUS, getBorderRadius(mBorderRadius, BORDER_BOTTOM_LEFT_RADIUS));
      }
    }
  }


  private float getBorderRadius(Spacing borderRadius, int position) {
    return borderRadius != null ? borderRadius.get(position) : 0;
  }

  private float getBorderWidth(int position) {
    return mBorderWidth != null ? mBorderWidth.get(position) : 0;
  }


  private void drawOneSide(Canvas canvas, @NonNull BorderEdge borderEdge) {
    preparePaint(borderEdge.getEdge());
    if (!FloatUtil.floatsEqual(0, getBorderWidth(borderEdge.getEdge()))) {
      Path path = borderEdge.createPath();
      canvas.drawPath(path, mPaint);
    }
  }


  private void preparePaint(int side) {
    float borderWidth = getBorderWidth(side);
    int color = getBorderColor(side);
    BorderStyle borderStyle = BorderStyle.values()[getBorderStyle(side)];
    PathEffect pathEffect = borderStyle.getPathEffect(borderWidth);
    mPaint.setColor(WXViewUtils.multiplyColorAlpha(color, mAlpha));
    mPaint.setStrokeWidth(borderWidth);
    mPaint.setPathEffect(pathEffect);
  }

  private int getBorderColor(int position) {
    return mBorderColor != null ?
           mBorderColor.get(position, mBorderColor.get(Spacing.ALL)) :
           DEFAULT_BORDER_COLOR;
  }

  private int getBorderStyle(int position) {
    return mBorderStyle != null ?
           mBorderStyle.get(position, mBorderStyle.get(Spacing.ALL)) :
           BorderStyle.SOLID.ordinal();
  }
}
