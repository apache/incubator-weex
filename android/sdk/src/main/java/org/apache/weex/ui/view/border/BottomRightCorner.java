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

import android.graphics.RectF;
import android.support.annotation.NonNull;

class BottomRightCorner extends BorderCorner {

  void set(float cornerRadius, float preBorderWidth, float postBorderWidth, @NonNull RectF borderBox) {
    set(cornerRadius, preBorderWidth, postBorderWidth, borderBox, 45);
  }

  @Override
  protected void prepareOval() {
    if (hasInnerCorner()) {
      setOvalLeft(getBorderBox().width() - (2 * getOuterCornerRadius() - getPreBorderWidth() / 2));
      setOvalTop(getBorderBox().height() - (2 * getOuterCornerRadius() - getPostBorderWidth() / 2));
      setOvalRight(getBorderBox().width() - (getPreBorderWidth() / 2));
      setOvalBottom(getBorderBox().height() - (getPostBorderWidth() / 2));
    } else {
      setOvalLeft(getBorderBox().width() - 1.5f * getOuterCornerRadius());
      setOvalTop(getBorderBox().height() - 1.5f * getOuterCornerRadius());
      setOvalRight(getBorderBox().width() - getOuterCornerRadius() / 2);
      setOvalBottom(getBorderBox().height() - getOuterCornerRadius() / 2);
    }
  }

  @Override
  protected void prepareRoundCorner() {
    if (hasOuterCorner()) {
      setRoundCornerStartX(getBorderBox().width() - getPreBorderWidth() / 2);
      setRoundCornerStartY(getBorderBox().height() - getOuterCornerRadius());

      setRoundCornerEndX(getBorderBox().width() - getOuterCornerRadius());
      setRoundCornerEndY(getBorderBox().height() - getPostBorderWidth() / 2);
    } else {
      final float x = getBorderBox().width() - getPreBorderWidth() / 2;
      final float y = getBorderBox().height() - getPostBorderWidth() / 2;

      setRoundCornerStartX(x);
      setRoundCornerStartY(y);

      setRoundCornerEndX(x);
      setRoundCornerEndY(y);
    }
  }
}
