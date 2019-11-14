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
package org.apache.weex.utils;

import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextDirectionHeuristic;
import android.text.TextDirectionHeuristics;
import android.text.TextPaint;

import java.lang.reflect.Constructor;

/**
 * Created by moxun on 2017/9/26.
 */

public class StaticLayoutProxy {
  private static Constructor<StaticLayout> layoutConstructor;
  public static StaticLayout create(CharSequence source, TextPaint paint,
                                    int width,
                                    Layout.Alignment align, float spacingmult, float spacingadd,
                                    boolean includepad, boolean forceRtl) {
    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN_MR2 && forceRtl) {
      TextDirectionHeuristic textDir = TextDirectionHeuristics.RTL;
      StaticLayout rtlLayout =  createInternal(source, paint, width, align, textDir, spacingmult, spacingadd, includepad);
      if (rtlLayout != null) {
        return rtlLayout;
      } else {
        return new StaticLayout(source, paint, width, align, spacingmult, spacingadd, includepad);
      }
    }
    return new StaticLayout(source, paint, width, align, spacingmult, spacingadd, includepad);
  }

  private static StaticLayout createInternal(CharSequence source, TextPaint paint,
                                             int width, Layout.Alignment align, TextDirectionHeuristic textDir,
                                             float spacingmult, float spacingadd,
                                             boolean includepad) {
    if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.JELLY_BEAN_MR2) {
      return null;
    } else {
      try {
        if (layoutConstructor == null) {
          Class<StaticLayout> clazz = StaticLayout.class;
          Constructor<StaticLayout> constructor = clazz.getConstructor(CharSequence.class, TextPaint.class,
                  int.class, Layout.Alignment.class, TextDirectionHeuristic.class,
                  float.class, float.class,
                  boolean.class);
          layoutConstructor = constructor;
        }
        if (layoutConstructor != null) {
          return layoutConstructor.newInstance(source, paint, width,
                  align, textDir, spacingmult, spacingadd, includepad);
        }

      } catch (Throwable e) {
        e.printStackTrace();
      }
    }
    return null;
  }
}
