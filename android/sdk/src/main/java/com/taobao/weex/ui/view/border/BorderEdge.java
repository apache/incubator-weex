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
import android.support.annotation.NonNull;
import com.taobao.weex.dom.CSSShorthand;

/**
 * Edge for border. Every border has four edges, and each edge has a previous corner and a post
 * corner.
 */
class BorderEdge {

  private
  @NonNull
  BorderCorner mPreCorner;
  private
  @NonNull
  BorderCorner mPostCorner;

  private final CSSShorthand.EDGE mEdge;
  private float mBorderWidth;

  BorderEdge(CSSShorthand.EDGE edge) {
    mEdge = edge;
  }

  BorderEdge set(@NonNull BorderCorner preCorner, @NonNull BorderCorner postCorner,
                 float borderWidth) {
    mPreCorner = preCorner;
    mPostCorner = postCorner;
    mBorderWidth = borderWidth;
    return this;
  }

  /**
   * Draw the edge on the canvas with the specified paint.
   * @param canvas the canvas where the edge will be drawn.
   * @param paint the paint which is used to draw.
   */
  void drawEdge(@NonNull Canvas canvas, @NonNull Paint paint) {
    PointF lineStart = mPreCorner.getCornerEnd();
    paint.setStrokeWidth(mBorderWidth);

    mPreCorner.drawRoundedCorner(canvas, paint,
            mPreCorner.getAngleBisectorDegree(),
            mPreCorner.getSharpCornerStart(), lineStart);

    paint.setStrokeWidth(mBorderWidth);
    PointF lineEnd = mPostCorner.getCornerStart();
    canvas.drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, paint);

    mPostCorner.drawRoundedCorner(canvas, paint,
            mPostCorner.getAngleBisectorDegree() - BorderCorner.SWEEP_ANGLE,
            lineEnd, mPostCorner.getSharpCornerEnd());
  }

  /**
   * The index of the edge
   * @return index of edge. May be one of
   */
  public CSSShorthand.EDGE getEdge() {
    return mEdge;
  }
}
