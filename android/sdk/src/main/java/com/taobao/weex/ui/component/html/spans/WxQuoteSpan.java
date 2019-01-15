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
package com.taobao.weex.ui.component.html.spans;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.text.Layout;
import android.text.style.QuoteSpan;

/**
 * Created by Bruce Too On 2019/1/11. At 14:11
 */
public class WxQuoteSpan extends QuoteSpan {
  private static final int STRIPE_WIDTH = 8;
  private static final int GAP_WIDTH = 14;
  @Override
  public int getColor() {
    return 0xffDEDEDE;
  }

  @Override
  public int getLeadingMargin(boolean first) {
    return STRIPE_WIDTH + GAP_WIDTH;
  }

  public void drawLeadingMargin(Canvas c, Paint p, int x, int dir,
                                int top, int baseline, int bottom,
                                CharSequence text, int start, int end,
                                boolean first, Layout layout) {
    Paint.Style style = p.getStyle();
    int color = p.getColor();

    p.setStyle(Paint.Style.FILL);
    p.setColor(getColor());

    c.drawRect(x, top, x + dir * STRIPE_WIDTH, bottom, p);

    p.setStyle(style);
    p.setColor(color);
  }
}
