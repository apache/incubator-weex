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
package com.taobao.weex.ui.animation;

import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.util.Property;
import android.view.View;


class CameraDistanceProperty extends Property<View, Float> {

  private final static String TAG = "CameraDistance";
  private static CameraDistanceProperty instance;

  static Property<View, Float> getInstance(){
    return instance;
  }

  private CameraDistanceProperty() {
    super(Float.class, TAG);
  }

  @Override
  public Float get(View view) {
    if (VERSION.SDK_INT >= VERSION_CODES.JELLY_BEAN) {
      return view.getCameraDistance();
    }
    else{
      return Float.NaN;
    }
  }

  @Override
  public void set(View object, Float value) {
    object.setCameraDistance(value);
  }
}
