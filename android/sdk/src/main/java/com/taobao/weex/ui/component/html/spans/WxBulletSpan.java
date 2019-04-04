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
import android.graphics.Path;
import android.graphics.Path.Direction;
import android.os.Parcel;
import android.text.Layout;
import android.text.ParcelableSpan;
import android.text.Spanned;
import android.text.style.LeadingMarginSpan;

/**
 * Created by Bruce Too On 2019/1/11. At 14:17
 * TODO different level with different bullet
 */
public class WxBulletSpan implements LeadingMarginSpan, ParcelableSpan {
  private final int mGapWidth;
  private final boolean mWantColor;
  private final int mColor;

  private static final int BULLET_RADIUS = 5;
  private static Path sBulletPath = null;
  public static final int STANDARD_GAP_WIDTH = 12;
  public final int mLevel;

  public WxBulletSpan() {
    mGapWidth = STANDARD_GAP_WIDTH;
    mWantColor = false;
    mColor = 0;
    mLevel = 0;
  }

  public WxBulletSpan(int gapWidth) {
    mGapWidth = gapWidth;
    mWantColor = false;
    mColor = 0;
    mLevel = 0;
  }

  public WxBulletSpan(int gapWidth, int color) {
    mGapWidth = gapWidth;
    mWantColor = true;
    mColor = color;
    mLevel = 0;
  }

  public WxBulletSpan(int gapWidth, int color,int level) {
    mGapWidth = gapWidth;
    mWantColor = true;
    mColor = color;
    mLevel = level;
  }

  public WxBulletSpan(Parcel src) {
    mGapWidth = src.readInt();
    mWantColor = src.readInt() != 0;
    mColor = src.readInt();
    mLevel = src.readInt();
  }

  public int getSpanTypeId() {
    return getSpanTypeIdInternal();
  }

  public int getSpanTypeIdInternal() {
    return 8;
  }

  public int describeContents() {
    return 0;
  }

  public void writeToParcel(Parcel dest, int flags) {
    writeToParcelInternal(dest, flags);
  }

  /** @hide */
  public void writeToParcelInternal(Parcel dest, int flags) {
    dest.writeInt(mGapWidth);
    dest.writeInt(mWantColor ? 1 : 0);
    dest.writeInt(mColor);
    dest.writeInt(mLevel);
  }

  public int getLeadingMargin(boolean first) {
    return 2 * BULLET_RADIUS + mGapWidth;
  }

  public void drawLeadingMargin(Canvas c, Paint p, int x, int dir,
                                int top, int baseline, int bottom,
                                CharSequence text, int start, int end,
                                boolean first, Layout l) {
    if (((Spanned) text).getSpanStart(this) == start) {
      Paint.Style style = p.getStyle();
      int oldcolor = 0;

      if (mWantColor) {
        oldcolor = p.getColor();
        p.setColor(mColor);
      }

      p.setStyle(Paint.Style.FILL);

      if (c.isHardwareAccelerated()) {
        if (sBulletPath == null) {
          sBulletPath = new Path();
          // Bullet is slightly better to avoid aliasing artifacts on mdpi devices.
          sBulletPath.addCircle(0.0f, 0.0f, 1.2f * BULLET_RADIUS, Direction.CW);
        }

        c.save();
        c.translate(x + dir * BULLET_RADIUS + 2, (top + bottom) / 2.0f);
        c.drawPath(sBulletPath, p);
        c.restore();
      } else {
        c.drawCircle(x + dir * BULLET_RADIUS + 2, (top + bottom) / 2.0f, BULLET_RADIUS, p);
      }

      if (mWantColor) {
        p.setColor(oldcolor);
      }

      p.setStyle(style);
    }
  }

  public void drawBulletWithLevel(Canvas c,int level){
    switch (level){
      case 0:

        break;
      case 1:

        break;
    }
  }
}
