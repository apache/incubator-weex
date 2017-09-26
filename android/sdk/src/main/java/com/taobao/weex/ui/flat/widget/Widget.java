/**
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
package com.taobao.weex.ui.flat.widget;


import android.graphics.Canvas;
import android.graphics.Point;
import android.graphics.Rect;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import com.taobao.weex.ui.view.border.BorderDrawable;

@RestrictTo(Scope.LIBRARY)
public interface Widget {

  public static final String TAG = "Widget";

  void draw(@NonNull Canvas canvas);

  void onDraw(@NonNull Canvas canvas);

  void setBackgroundAndBorder(@NonNull BorderDrawable backgroundBorder);

  void setLayout(int width, int height, int left, int right, int top, int bottom, Point offset);

  void setContentBox(int leftOffset, int topOffset, int rightOffset, int bottomOffset);

  @NonNull Point getLocInFlatContainer();

  @Nullable BorderDrawable getBackgroundAndBorder();

  @NonNull Rect getBorderBox();
}
