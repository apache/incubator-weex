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

package com.taobao.weex.ui.view.border;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.SparseArray;
import android.util.SparseIntArray;
import com.taobao.weex.dom.CSSShorthand;

class BorderUtil {

  static int fetchFromSparseArray(@Nullable SparseIntArray array, int position, int fallback) {
    return array == null ? fallback :
            array.get(position, array.get(CSSShorthand.EDGE.ALL.ordinal()));
  }

  static void updateSparseArray(@NonNull SparseIntArray array, int position, int value) {
    if (position == CSSShorthand.EDGE.ALL.ordinal()) {
      array.put(CSSShorthand.EDGE.ALL.ordinal(), value);
      array.put(CSSShorthand.EDGE.TOP.ordinal(), value);
      array.put(CSSShorthand.EDGE.LEFT.ordinal(), value);
      array.put(CSSShorthand.EDGE.RIGHT.ordinal(), value);
      array.put(CSSShorthand.EDGE.BOTTOM.ordinal(), value);
    } else {
      array.put(position, value);
    }
  }
}
