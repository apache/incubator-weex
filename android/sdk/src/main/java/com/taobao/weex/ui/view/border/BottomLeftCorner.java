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

class BottomLeftCorner extends BorderCorner {

  BottomLeftCorner(float cornerRadius, float preBorderWidth, float postBorderWidth, @NonNull RectF borderBox) {
    super(cornerRadius, preBorderWidth, postBorderWidth, borderBox, 135);
  }

  @NonNull
  @Override
  protected RectF getOvalIfInnerCornerExist() {
    return new RectF(getPostBorderWidth() / 2,
                     getBorderBox().height() - (2 * getOuterCornerRadius() - getPreBorderWidth() / 2),
                     2 * getOuterCornerRadius() - getPostBorderWidth() / 2,
                     getBorderBox().height() - getPreBorderWidth() / 2);
  }

  @NonNull
  @Override
  protected RectF getOvalIfInnerCornerNotExist() {
    return new RectF(getOuterCornerRadius()/2,
                     getBorderBox().height()-1.5f*getOuterCornerRadius(),
                     1.5f*getOuterCornerRadius(),
                     getBorderBox().height()-getOuterCornerRadius()/2);
  }

  @NonNull
  @Override
  protected PointF getRoundCornerStart() {
    return new PointF(getOuterCornerRadius(),
                      getBorderBox().height() - getPreBorderWidth() / 2);
  }

  @NonNull
  @Override
  protected PointF getSharpCornerVertex() {
    return new PointF(getPostBorderWidth() / 2,
                      getBorderBox().height() - getPreBorderWidth() / 2);
  }

  @NonNull
  @Override
  protected PointF getSharpCornerStart() {
    PointF pointF=getSharpCornerVertex();
    pointF.y=getBorderBox().height();
    return pointF;
  }

  @NonNull
  @Override
  protected PointF getSharpCornerEnd() {
    PointF pointF=getSharpCornerVertex();
    pointF.x=0;
    return pointF;
  }

  @NonNull
  @Override
  protected PointF getRoundCornerEnd() {
    return new PointF(getPostBorderWidth() / 2,
                      getBorderBox().height() - getOuterCornerRadius());
  }
}
