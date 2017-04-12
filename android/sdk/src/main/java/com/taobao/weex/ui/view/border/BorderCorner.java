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

import android.graphics.PointF;
import android.graphics.RectF;
import android.support.annotation.NonNull;

import com.taobao.weex.dom.flex.FloatUtil;

abstract class BorderCorner {

  final static float SWEEP_ANGLE = 45;
  private final float mCornerRadius;
  private final float mPreBorderWidth;
  private final float mPostBorderWidth;
  private final RectF mBorderBox;

  BorderCorner(float cornerRadius, float preBorderWidth, float postBorderWidth, @NonNull RectF
      borderBox) {
    mCornerRadius = cornerRadius;
    mPreBorderWidth = preBorderWidth;
    mPostBorderWidth = postBorderWidth;
    mBorderBox = borderBox;
  }

  /**
   * Tell whether this corner has a rounded inner corner.
   * If a corner has a rounded inner corner, it has an outer corner as well.
   * @return true for a rounded inner corner, otherwise false.
   */
  boolean hasInnerCorner() {
    return hasOuterCorner()
           && (getPreBorderWidth() >= 0)
           && (getPostBorderWidth() >= 0)
           && (getOuterCornerRadius() > getPreBorderWidth())
           && (getOuterCornerRadius() > getPostBorderWidth());
  }

  /**
   * Tell whether this corner has a rounded outer corner.
   * @return true for a rounded outer corner, otherwise false.
   */
  boolean hasOuterCorner() {
    return getOuterCornerRadius() > 0 && !FloatUtil.floatsEqual(0, getOuterCornerRadius());
  }

  protected float getPreBorderWidth() {
    return mPreBorderWidth;
  }

  protected float getPostBorderWidth() {
    return mPostBorderWidth;
  }

  protected float getOuterCornerRadius() {
    return mCornerRadius;
  }

  /**
   * Get the staring point of the corner.
   * @return the starting point of the corner.
   */
  @NonNull
  PointF getCornerStart() {
    PointF lineStart;
    if (hasOuterCorner()) {
      lineStart = getRoundCornerStart();
    } else {
      lineStart = getSharpCornerVertex();
    }
    return lineStart;
  }

  @NonNull
  abstract protected PointF getRoundCornerStart();

  @NonNull
  abstract protected PointF getSharpCornerVertex();

  /**
   * Get the ending point of the corner.
   * @return the ending point of the corner.
   */
  @NonNull
  PointF getCornerEnd() {
    PointF lineEnd;
    if (hasOuterCorner()) {
      lineEnd = getRoundCornerEnd();
    } else {
      lineEnd = getSharpCornerVertex();
    }
    return lineEnd;
  }

  @NonNull
  abstract protected PointF getRoundCornerEnd();

  abstract protected float getAngleBisectorDegree();

  @NonNull
  abstract protected PointF getSharpCornerStart();

  @NonNull
  abstract protected PointF getSharpCornerEnd();

  @NonNull
  abstract protected RectF getOvalIfInnerCornerExist();

  @NonNull
  abstract protected RectF getOvalIfInnerCornerNotExist();

  protected RectF getBorderBox() {
    return mBorderBox;
  }
}
