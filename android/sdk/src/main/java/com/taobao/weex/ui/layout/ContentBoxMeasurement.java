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
package com.taobao.weex.ui.layout;

import android.icu.util.Measure;
import android.support.annotation.NonNull;
import android.text.Layout;
import android.util.Log;
import android.util.Pair;

import com.taobao.weex.base.CalledByNative;

public class ContentBoxMeasurement {

  @CalledByNative
  private Pair<Float, Float> measure(float width, float height, MeasureMode widthMeasureMode, MeasureMode heightMeasureMode) {
    Log.i("ContentBoxMeasurement", "measure");
    return new Pair<Float, Float>(0.0f, 0.0f);
  }

  @CalledByNative
  private void layoutBefore() {
    Log.i("ContentBoxMeasurement", "layoutBefore");
  }

  @CalledByNative
  private void layoutAfter(Layout layout) {
    Log.i("ContentBoxMeasurement", "layoutBefore");
  }

  @CalledByNative
  private static ContentBoxMeasurement Generate() {
    return new ContentBoxMeasurement();
  }
}
