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
import com.taobao.weex.dom.CSSShorthand.CSSProperty;
import java.util.Arrays;

public class CSSShorthand<T extends Enum<? extends CSSProperty>> implements Cloneable {

  interface CSSProperty{
  }

  public static enum EDGE implements CSSProperty{
    TOP, BOTTOM, LEFT, RIGHT, ALL;
  }

  public static enum CORNER implements CSSProperty{
    BORDER_TOP_LEFT, BORDER_TOP_RIGHT,
    BORDER_BOTTOM_RIGHT, BORDER_BOTTOM_LEFT, ALL;
  }

  public static enum TYPE {
    MARGIN, PADDING, BORDER;
  }

  private float[] values;

  public CSSShorthand(float []values){
    replace(values);
  }

  public CSSShorthand() {
    this(false);
  }

  CSSShorthand(boolean fillWithNaN) {
    values = new float[Math.max(EDGE.values().length,CORNER.values().length)];
    if (fillWithNaN) {
      Arrays.fill(values, Float.NaN);
    }
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public void set(@NonNull EDGE edge, float value){
    setInternal(edge, value);
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public void set(@NonNull CORNER edge, float value) {
    setInternal(edge, value);
  }

  /**
   * {@link EDGE#ALL} is not supported, 0 will be returned.
   * @throws IndexOutOfBoundsException
   * @param edge
   * @return
   */
  public float get(@NonNull EDGE edge){
    return getInternal(edge);
  }

  /**
   * {@link CORNER#ALL} is not supported, 0 will be returned.
   * @throws IndexOutOfBoundsException
   * @param edge
   * @return
   */
  public float get(@NonNull CORNER edge) {
    return getInternal(edge);
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public final void replace(float []values){
    this.values = values;
  }

  @Override
  public CSSShorthand clone() throws CloneNotSupportedException {
    return (CSSShorthand) super.clone();
  }

  private void setInternal(@NonNull Enum<? extends CSSProperty> edge, float value){
    if (edge == EDGE.ALL || edge == CORNER.ALL) {
      Arrays.fill(values, value);
    } else {
      values[edge.ordinal()] = value;
    }
  }

  private float getInternal(@NonNull Enum<? extends CSSProperty> edge){
    return (edge == EDGE.ALL || edge == CORNER.ALL) ? 0 : values[edge.ordinal()];
  }
}