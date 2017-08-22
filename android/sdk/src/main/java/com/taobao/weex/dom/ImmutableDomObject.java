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
package com.taobao.weex.dom;

import android.support.annotation.NonNull;
import com.taobao.weex.dom.flex.Spacing;

/**
 * Created by sospartan on 25/10/2016.
 */

public interface ImmutableDomObject {
  String getRef();
  @NonNull Spacing getMargin();
  float getLayoutWidth();
  float getLayoutHeight();

  /**
   * Use {@link #getCSSLayoutLeft()} ()} instead
   */
  @Deprecated
  float getLayoutX();

  /**
   * Use {@link #getCSSLayoutTop()} instead
   */
  @Deprecated
  float getLayoutY();

  public float getCSSLayoutTop();

  public float getCSSLayoutBottom();

  public float getCSSLayoutLeft();

  public float getCSSLayoutRight();
  boolean isFixed();
  @NonNull WXStyle getStyles();
  @NonNull WXEvent getEvents();
  @NonNull WXAttr getAttrs();
  @NonNull Spacing getPadding();
  @NonNull Spacing getBorder();
  Object getExtra();
  String getType();

  public final ImmutableDomObject DESTROYED = WXDomObject.DESTROYED;
}
