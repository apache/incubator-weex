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
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;
import android.support.annotation.NonNull;

import com.taobao.weex.dom.flex.Spacing;

/**
 * Edge for border. Every border has four edges, and each edge has a previous corner and a post
 * corner.
 */
class BorderEdge {

  private
  @NonNull
  final BorderCorner mPreCorner;
  private
  @NonNull
  final BorderCorner mPostCorner;

  private final int mEdge;
  private final float mBorderWidth;

  BorderEdge(@NonNull BorderCorner preCorner, @NonNull BorderCorner postCorner,
             @BorderWidthStyleColorType int edge, float
      borderWidth) {
    mPreCorner = preCorner;
    mPostCorner = postCorner;
    mEdge = edge;
    mBorderWidth = borderWidth;
  }

  /**
   * Draw the edge on the canvas with the specified paint.
   * @param canvas the canvas where the edge will be drawn.
   * @param paint the paint which is used to draw.
   */
  void drawEdge(@NonNull Canvas canvas, @NonNull Paint paint) {
    RectF oval;
    PointF lineStart = mPreCorner.getCornerEnd();
    paint.setStrokeWidth(mBorderWidth);

    drawRoundedCorner(canvas, paint, mPreCorner,
                      mPreCorner.getAngleBisectorDegree(),
                      mPreCorner.getSharpCornerStart(), lineStart);

    paint.setStrokeWidth(mBorderWidth);
    PointF lineEnd = mPostCorner.getCornerStart();
    canvas.drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, paint);

    drawRoundedCorner(canvas, paint, mPostCorner,
                      mPostCorner.getAngleBisectorDegree() - BorderCorner.SWEEP_ANGLE,
                      lineEnd, mPostCorner.getSharpCornerEnd());
  }

  /**
   * Draw the Rounded corner.
   * @param canvas the canvas where the edge will be drawn
   * @param paint the paint which is used to draw
   * @param borderCorner the corner to be drawn
   * @param startAngle the startAngle of the corner
   * @param startPoint the startPoint of the line
   * @param endPoint the endPoint of the line
   */
  private void drawRoundedCorner(@NonNull Canvas canvas, @NonNull Paint paint,
                                 @NonNull BorderCorner borderCorner, float startAngle,
                                 @NonNull PointF startPoint, @NonNull PointF endPoint) {
    if (borderCorner.hasOuterCorner()) {
      RectF oval;
      if (borderCorner.hasInnerCorner()) {
        oval = borderCorner.getOvalIfInnerCornerExist();
      } else {
        paint.setStrokeWidth(borderCorner.getOuterCornerRadius());
        oval = borderCorner.getOvalIfInnerCornerNotExist();
      }
      /*Due to the problem of hardware-acceleration, border-radius in some case will not
       be rendered if Path.addArc used instead and the following condition met.
       1. hardware-acceleration enabled
       2. System version is Android 4.1
       3. Screen width is 720px.
       http://dotwe.org/weex/421b9ad09fde51c0b49bb56b37fcf955
      */
      canvas.drawArc(oval, startAngle, BorderCorner.SWEEP_ANGLE, false, paint);
    } else {
      canvas.drawLine(startPoint.x, startPoint.y, endPoint.x, endPoint.y, paint);
    }
  }

  /**
   * The index of the edge
   * @return index of edge. May be one of
   * {@link Spacing#TOP},{@link Spacing#BOTTOM},{@link Spacing#RIGHT},{@link Spacing#LEFT}.
   */
  public @BorderWidthStyleColorType
  int getEdge() {
    return mEdge;
  }
}
