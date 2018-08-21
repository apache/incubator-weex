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

import android.graphics.RectF;
import android.support.annotation.NonNull;

class TopLeftCorner extends BorderCorner {

  void set(float cornerRadius, float preBorderWidth, float postBorderWidth, @NonNull RectF borderBox) {
    set(cornerRadius, preBorderWidth, postBorderWidth, borderBox, 225);
  }

  @Override
  protected void prepareOval() {
    if (hasInnerCorner()) {
      setOvalLeft(getPreBorderWidth() / 2);
      setOvalTop(getPostBorderWidth() / 2);
      setOvalRight(2 * getOuterCornerRadius() - getPreBorderWidth() / 2);
      setOvalBottom(2 * getOuterCornerRadius() - getPostBorderWidth() / 2);
    } else {
      setOvalLeft(getOuterCornerRadius() / 2);
      setOvalTop(getOuterCornerRadius() / 2);
      setOvalRight(getOuterCornerRadius() * 1.5f);
      setOvalBottom(getOuterCornerRadius() * 1.5f);
    }
  }

  @Override
  protected void prepareRoundCorner() {
    if (hasOuterCorner()) {
      setRoundCornerStartX(getPreBorderWidth() / 2);
      setRoundCornerStartY(getOuterCornerRadius());

      setRoundCornerEndX(getOuterCornerRadius());
      setRoundCornerEndY(getPostBorderWidth() / 2);
    } else {
      final float x = getPreBorderWidth() / 2;
      final float y = getPostBorderWidth() / 2;

      setRoundCornerStartX(x);
      setRoundCornerStartY(y);

      setRoundCornerEndX(x);
      setRoundCornerEndY(y);
    }
  }
}
