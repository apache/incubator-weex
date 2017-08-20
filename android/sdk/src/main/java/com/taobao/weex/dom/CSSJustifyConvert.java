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

import android.text.TextUtils;

import com.taobao.weex.dom.flex.CSSJustify;

class CSSJustifyConvert {

  public static CSSJustify convert(String s) {
    CSSJustify cssJustify = CSSJustify.FLEX_START;
    if (TextUtils.isEmpty(s) || s.equals("flex-start")) {
      cssJustify = CSSJustify.FLEX_START;
    } else if (s.equals("flex-end")) {
      cssJustify = CSSJustify.FLEX_END;
    } else if (s.equals("center")) {
      cssJustify = CSSJustify.CENTER;
    } else if (s.equals("space-between")) {
      cssJustify = CSSJustify.SPACE_BETWEEN;
    } else if (s.equals("space-around")) {
      cssJustify = CSSJustify.SPACE_AROUND;
    }
    return cssJustify;
  }
}
