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
import com.taobao.weex.utils.WXLogUtils;

class BottomLeftCorner extends BorderCorner {

  void set(float cornerRadius, float preBorderWidth, float postBorderWidth, @NonNull RectF borderBox) {
    set(cornerRadius, preBorderWidth, postBorderWidth, borderBox, 135);
  }

  @NonNull
  @Override
  protected RectF getOvalIfInnerCornerExist() {
    WXLogUtils.w("BottomLeftCorner", "++++++++++> getOvalIfInnerCornerExist() " + mOval);
    final float left = getPostBorderWidth() / 2;
    final float top = getBorderBox().height() - (2 * getOuterCornerRadius() - getPreBorderWidth() / 2);
    final float right = 2 * getOuterCornerRadius() - getPostBorderWidth() / 2;
    final float bottom = getBorderBox().height() - getPreBorderWidth() / 2;
    if (null == mOval) {
      mOval = new RectF(left, top, right, bottom);
    } else {
      mOval.set(left, top, right, bottom);
    }
    return mOval;
  }

  @NonNull
  @Override
  protected RectF getOvalIfInnerCornerNotExist() {
    WXLogUtils.w("BottomLeftCorner", "++++++++++> getOvalIfInnerCornerNotExist() " + mOval);
    final float left = getOuterCornerRadius() / 2;
    final float top = getBorderBox().height() - 1.5f * getOuterCornerRadius();
    final float right = 1.5f * getOuterCornerRadius();
    final float bottom = getBorderBox().height() - getOuterCornerRadius() / 2;
    if (null == mOval) {
      mOval = new RectF(left, top, right, bottom);
    } else {
      mOval.set(left, top, right, bottom);
    }
    return mOval;
  }

  @NonNull
  @Override
  protected PointF getRoundCornerStart() {
    final float x = getOuterCornerRadius();
    final float y = getBorderBox().height() - getPreBorderWidth() / 2;
    if (null == mRoundCornerStart) {
      mRoundCornerStart = new PointF(x, y);
    } else {
      mRoundCornerStart.set(x, y);
    }
    return mRoundCornerStart;
  }

  @NonNull
  @Override
  protected PointF getSharpCornerVertex() {
    final float x = getPostBorderWidth() / 2;
    final float y = getBorderBox().height() - getPreBorderWidth() / 2;
    if (null == mSharpCornerVertex) {
      mSharpCornerVertex = new PointF(x, y);
    } else {
      mSharpCornerVertex.set(x, y);
    }
    return mSharpCornerVertex;
  }

  @NonNull
  @Override
  protected PointF getSharpCornerStart() {
    PointF pointF = getSharpCornerVertex();
    pointF.y = getBorderBox().height();
    return pointF;
  }

  @NonNull
  @Override
  protected PointF getSharpCornerEnd() {
    PointF pointF = getSharpCornerVertex();
    pointF.x = 0;
    return pointF;
  }

  @NonNull
  @Override
  protected PointF getRoundCornerEnd() {
    final float x = getPostBorderWidth() / 2;
    final float y = getBorderBox().height() - getOuterCornerRadius();
    if (null == mRoundCornerEnd) {
      mRoundCornerEnd = new PointF(x, y);
    } else {
      mRoundCornerEnd.set(x, y);
    }
    return mRoundCornerEnd;
  }
}
