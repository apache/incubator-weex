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

import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.PathEffect;
import android.graphics.Shader;
import android.support.annotation.Nullable;
import org.apache.weex.dom.CSSShorthand;

enum BorderStyle {
  SOLID,
  DASHED,
  DOTTED;

  /**
   * Use {@link LinearGradient} to replace {@link PathEffect}
   * for implementing {@link #DASHED} or {@link #DASHED}
   * @param borderWidth width of the edge
   * @param borderColor color of the edge
   * @param edge the index of the ede. See {@link CSSShorthand}
   * @return An object of {@link LinearGradient} without color transitions for {@link #DOTTED}
   * or {@link #DASHED}, null otherwise
   */
  @Nullable
  Shader getLineShader(float borderWidth, int borderColor, CSSShorthand.EDGE edge) {
    switch (this) {
      case DOTTED:
        if (edge == CSSShorthand.EDGE.LEFT || edge == CSSShorthand.EDGE.RIGHT) {
          return new LinearGradient(0, 0, 0, borderWidth * 2, new int[]{borderColor, Color
                  .TRANSPARENT}, new float[]{0.5f, 0.5f}, Shader.TileMode.REPEAT);
        } else if (edge == CSSShorthand.EDGE.TOP || edge == CSSShorthand.EDGE.BOTTOM) {
          return new LinearGradient(0, 0, borderWidth * 2, 0, new int[]{borderColor, Color
                  .TRANSPARENT}, new float[]{0.5f, 0.5f}, Shader.TileMode.REPEAT);
        }
      case DASHED:
        if (edge == CSSShorthand.EDGE.LEFT || edge == CSSShorthand.EDGE.RIGHT) {
          return new LinearGradient(0, 0, 0, borderWidth * 6, new int[]{borderColor, Color
                  .TRANSPARENT}, new float[]{0.5f, 0.5f}, Shader.TileMode.REPEAT);
        } else if (edge == CSSShorthand.EDGE.TOP || edge == CSSShorthand.EDGE.BOTTOM) {
          return new LinearGradient(0, 0, borderWidth * 6, 0, new int[]{borderColor, Color
                  .TRANSPARENT}, new float[]{0.5f, 0.5f}, Shader.TileMode.REPEAT);
        }
      default:
        return null;
    }
  }
}
