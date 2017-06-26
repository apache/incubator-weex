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

import android.text.TextUtils;
import android.view.View;

import com.facebook.yoga.YogaAlign;
import com.facebook.yoga.YogaFlexDirection;
import com.facebook.yoga.YogaJustify;
import com.facebook.yoga.YogaMeasureMode;
import com.facebook.yoga.YogaPositionType;
import com.facebook.yoga.YogaWrap;


/**
 * Created by sospartan on 21/06/2017.
 */

public class LayoutUtility {
  public static final float UNDEFINED = Float.NaN;
  private static final float EPSILON = .00001f;

  public static boolean isUndefined(float value) {
    return Float.compare(value, UNDEFINED) == 0;
  }

  public static YogaAlign alignItems(String s) {
    YogaAlign align = YogaAlign.STRETCH;
    if (TextUtils.isEmpty(s)) {
      align = YogaAlign.STRETCH;
    } else if (s.equals("stretch")) {
      align = YogaAlign.STRETCH;
    } else if (s.equals("flex-end")) {
      align = YogaAlign.FLEX_END;
    } else if (s.equals("auto")) {
      align = YogaAlign.AUTO;
    } else if (s.equals("center")) {
      align = YogaAlign.CENTER;
    }

    return align;
  }

  public static YogaAlign alignSelf(String s) {
    YogaAlign align = YogaAlign.AUTO;
    if (TextUtils.isEmpty(s)) {
      align = YogaAlign.AUTO;
    } else if (s.equals("flex-start")) {
      align = YogaAlign.FLEX_START;
    } else if (s.equals("flex-end")) {
      align = YogaAlign.FLEX_END;
    } else if (s.equals("stretch")) {
      align = YogaAlign.STRETCH;
    } else if (s.equals("center")) {
      align = YogaAlign.CENTER;
    }

    return align;
  }

  public static YogaFlexDirection flexDirection(String s) {
    YogaFlexDirection cssFlexDirection = YogaFlexDirection.ROW;
    if (TextUtils.isEmpty(s)) {
      cssFlexDirection = YogaFlexDirection.ROW;
    } else if (s.equals("column")) {
      cssFlexDirection = YogaFlexDirection.COLUMN;
    } else if (s.equals("column-reverse")) {
      cssFlexDirection = YogaFlexDirection.COLUMN_REVERSE;
    } else if (s.equals("row")) {
      cssFlexDirection = YogaFlexDirection.ROW;
    } else if (s.equals("row-reverse")) {
      cssFlexDirection = YogaFlexDirection.ROW_REVERSE;
    }
    return cssFlexDirection;
  }

  public static YogaJustify justify(String s) {
    YogaJustify cssJustify = YogaJustify.FLEX_START;
    if (TextUtils.isEmpty(s) || s.equals("flex-start")) {
      cssJustify = YogaJustify.FLEX_START;
    } else if (s.equals("flex-end")) {
      cssJustify = YogaJustify.FLEX_END;
    } else if (s.equals("center")) {
      cssJustify = YogaJustify.CENTER;
    } else if (s.equals("space-between")) {
      cssJustify = YogaJustify.SPACE_BETWEEN;
    } else if (s.equals("space-around")) {
      cssJustify = YogaJustify.SPACE_AROUND;
    }
    return cssJustify;
  }

  public static YogaWrap wrap(String s) {
    YogaWrap cssWrap = YogaWrap.NO_WRAP;
    if (TextUtils.isEmpty(s)) {
      return cssWrap;
    } else if (s.equals("wrap")) {
      return YogaWrap.WRAP;
    }
    return cssWrap;
  }

  public static YogaPositionType positionType(String s) {
    YogaPositionType position = YogaPositionType.RELATIVE;
    if (TextUtils.isEmpty(s) || s.equals("relative") || s.equals("sticky")) {
      position = YogaPositionType.RELATIVE;
    } else if (s.equals("absolute") || s.equals("fixed")) {
      position = YogaPositionType.ABSOLUTE;
    }
    return position;
  }


  public static int viewMeasureSpec(YogaMeasureMode mode) {
    if (mode == YogaMeasureMode.AT_MOST) {
      return View.MeasureSpec.AT_MOST;
    } else if (mode == YogaMeasureMode.EXACTLY) {
      return View.MeasureSpec.EXACTLY;
    } else {
      return View.MeasureSpec.UNSPECIFIED;
    }
  }

  public static boolean floatsEqual(float f1, float f2) {
    if (Float.isNaN(f1) || Float.isNaN(f2)) {
      return Float.isNaN(f1) && Float.isNaN(f2);
    }
    return Math.abs(f2 - f1) < EPSILON;
  }
}
