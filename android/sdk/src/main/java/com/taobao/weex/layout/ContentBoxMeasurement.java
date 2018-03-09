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
package com.taobao.weex.layout;

import com.taobao.weex.base.CalledByNative;
import com.taobao.weex.ui.component.WXComponent;

import java.io.Serializable;
import java.lang.ref.WeakReference;

public abstract class ContentBoxMeasurement implements Serializable {

  protected WeakReference<WXComponent> mComponent;

  public ContentBoxMeasurement() {

  }

  public ContentBoxMeasurement(WXComponent component) {
    this.mComponent = new WeakReference<>(component);
  }

  /** uiThread = false **/
  @CalledByNative
  public abstract MeasureSize measure(float width, float height, int widthMeasureMode, int heightMeasureMode);

  /** uiThread = false **/
  @CalledByNative
  public abstract void layoutBefore();

  /** uiThread = false **/
  @CalledByNative
  public abstract void layoutAfter(float computedWidth, float computedHeight);
}
