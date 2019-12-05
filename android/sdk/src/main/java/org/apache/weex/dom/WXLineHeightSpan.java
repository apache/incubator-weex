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
package org.apache.weex.dom;

import android.graphics.Paint;
import android.text.style.LineHeightSpan;

import org.apache.weex.WXEnvironment;
import org.apache.weex.utils.WXLogUtils;

public class WXLineHeightSpan implements LineHeightSpan{

  private int lineHeight;
  public WXLineHeightSpan(int lineHeight){
    this.lineHeight=lineHeight;
  }

  @Override
  public void chooseHeight(CharSequence text, int start, int end, int spanstartv, int v, Paint.FontMetricsInt fm) {
    if(WXEnvironment.isApkDebugable()) {
      WXLogUtils.d("LineHeight", text + " ; start " + start + "; end " + end + "; spanstartv "
              + spanstartv + "; v " + v + "; fm " + fm);
    }
    int halfLeading=(lineHeight-(fm.descent-fm.ascent))/2;
    fm.top-=halfLeading;
    fm.bottom+=halfLeading;
    fm.ascent-=halfLeading;
    fm.descent+=halfLeading;
  }
}
