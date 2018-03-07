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
package com.taobao.weex.dom;

import android.support.annotation.NonNull;
import android.support.annotation.RestrictTo;

import java.util.Arrays;

public class CSSShorthand implements Cloneable {

  public static enum EDGE {
    TOP, RIGHT, BOTTOM, LEFT, ALL;
  }

  public static enum TYPE {
    MARGIN, PADDING, BORDER;
  }

  private final float[] values = new float[EDGE.values().length];

  public CSSShorthand() {
    this(false);
  }

  CSSShorthand(boolean fillWithNaN) {
    if (fillWithNaN) {
      Arrays.fill(values, Float.NaN);
    }
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public void set(@NonNull EDGE edge, float value) {
    if (edge == EDGE.ALL) {
      for (EDGE side : EDGE.values()) {
        values[side.ordinal()] = value;
      }
    } else {
      values[edge.ordinal()] = value;
    }
  }

  public float get(@NonNull EDGE edge) {
    return values[edge.ordinal()];
  }

  @Override
  public CSSShorthand clone() throws CloneNotSupportedException {
    return (CSSShorthand) super.clone();
  }
}