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

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import com.taobao.weex.base.CalledByNative;
import com.taobao.weex.common.Destroyable;
import com.taobao.weex.ui.component.WXComponent;

import java.io.Serializable;
import java.lang.ref.WeakReference;

public abstract class ContentBoxMeasurement implements Serializable,Destroyable {

  @Nullable
  protected WXComponent mComponent;

  protected float mMeasureWidth;

  protected float mMeasureHeight;

  public ContentBoxMeasurement() {
    mComponent = null;
  }

  public ContentBoxMeasurement(@NonNull WXComponent component) {
    this.mComponent = component;
  }

  /** uiThread = false **/
  @CalledByNative
  public final void measure(float width, float height, int widthMeasureMode, int heightMeasureMode) {
    measureInternal(width, height, widthMeasureMode, heightMeasureMode);
  }

  /** uiThread = false **/
  public abstract void measureInternal(float width, float height, int widthMeasureMode, int heightMeasureMode);

  /** uiThread = false **/
  @CalledByNative
  public abstract void layoutBefore();

  /** uiThread = false **/
  @CalledByNative
  public abstract void layoutAfter(float computedWidth, float computedHeight);

  @CalledByNative
  public float getWidth() {
    return mMeasureWidth;
  }

  @CalledByNative
  public float getHeight() {
    return mMeasureHeight;
  }

  @Override
  public void destroy() {
    mComponent = null;
  }
}
