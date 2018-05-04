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

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.Rect;

import com.taobao.weex.utils.WXResourceUtils;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import static org.hamcrest.CoreMatchers.is;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThat;

@RunWith(RobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class BorderDrawableTest {

  @Before
  public void setUp() throws Exception {

  }

  @Test
  public void testGetOpacity() throws Exception {
    BorderDrawable opaque = new BorderDrawable();
    opaque.setColor(Color.GREEN);
    assertThat(opaque.getOpacity(), is(PixelFormat.OPAQUE));

    BorderDrawable transparent = new BorderDrawable();
    transparent.setColor(WXResourceUtils.getColor("#00ff0000"));
    assertThat(transparent.getOpacity(), is(PixelFormat.TRANSPARENT));

    BorderDrawable half = new BorderDrawable();
    half.setColor(WXResourceUtils.getColor("#aaff0000"));
    assertThat(half.getOpacity(), is(PixelFormat.TRANSLUCENT));

    BorderDrawable changeAlpha = new BorderDrawable();
    changeAlpha.setColor(Color.RED);
    changeAlpha.setAlpha(15);
    assertThat(changeAlpha.getOpacity(), is(PixelFormat.TRANSLUCENT));
  }

  @Test
  public void testBorderWidth() throws Exception {
    BorderDrawable none = new BorderDrawable();
    assertThat(none.getBorderWidth(Spacing.ALL), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(none.getBorderWidth(Spacing.LEFT), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(none.getBorderWidth(Spacing.RIGHT), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(none.getBorderWidth(Spacing.TOP), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(none.getBorderWidth(Spacing.BOTTOM), is(BorderDrawable.DEFAULT_BORDER_WIDTH));

    BorderDrawable full = new BorderDrawable();
    full.setBorderWidth(Spacing.ALL, 12);
    assertThat(full.getBorderWidth(Spacing.ALL), is(12f));
    assertThat(full.getBorderWidth(Spacing.LEFT), is(12f));
    assertThat(full.getBorderWidth(Spacing.RIGHT), is(12f));
    assertThat(full.getBorderWidth(Spacing.TOP), is(12f));
    assertThat(full.getBorderWidth(Spacing.BOTTOM), is(12f));

    BorderDrawable noneAndPart = new BorderDrawable();
    noneAndPart.setBorderWidth(Spacing.LEFT, 5);
    noneAndPart.setBorderWidth(Spacing.TOP, 12);
    assertThat(noneAndPart.getBorderWidth(Spacing.LEFT), is(5f));
    assertThat(noneAndPart.getBorderWidth(Spacing.RIGHT), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(noneAndPart.getBorderWidth(Spacing.TOP), is(12f));
    assertThat(noneAndPart.getBorderWidth(Spacing.BOTTOM), is(BorderDrawable.DEFAULT_BORDER_WIDTH));

    BorderDrawable fullAndPart = new BorderDrawable();
    fullAndPart.setBorderWidth(Spacing.ALL, 5);
    fullAndPart.setBorderWidth(Spacing.LEFT, 12);
    fullAndPart.setBorderWidth(Spacing.TOP, 19);
    assertThat(fullAndPart.getBorderWidth(Spacing.LEFT), is(12f));
    assertThat(fullAndPart.getBorderWidth(Spacing.RIGHT), is(5f));
    assertThat(fullAndPart.getBorderWidth(Spacing.TOP), is(19f));
    assertThat(fullAndPart.getBorderWidth(Spacing.BOTTOM), is(5f));

    BorderDrawable partAndFull = new BorderDrawable();
    partAndFull.setBorderWidth(Spacing.LEFT, 12);
    partAndFull.setBorderWidth(Spacing.ALL, 5);
    assertThat(partAndFull.getBorderWidth(Spacing.ALL), is(5f));
    assertThat(partAndFull.getBorderWidth(Spacing.LEFT), is(5f));
    assertThat(partAndFull.getBorderWidth(Spacing.RIGHT), is(5f));
    assertThat(partAndFull.getBorderWidth(Spacing.TOP), is(5f));
    assertThat(partAndFull.getBorderWidth(Spacing.BOTTOM), is(5f));
  }

  @Test
  public void testBorderRadius() throws Exception {
    BorderDrawable none = new BorderDrawable();
    none.draw(new Canvas());
    assertThat(none.getBorderRadius(BorderDrawable.BORDER_RADIUS_ALL), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(none.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(none.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(none.getBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(none.getBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS), is(BorderDrawable.DEFAULT_BORDER_WIDTH));

    BorderDrawable full = new BorderDrawable();
    full.setBorderRadius(BorderDrawable.BORDER_RADIUS_ALL, 12);
    full.setBounds(new Rect(0, 0, 400, 400));
    full.draw(new Canvas());
    assertThat(full.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS), is(12f));
    assertThat(full.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS), is(12f));
    assertThat(full.getBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS), is(12f));
    assertThat(full.getBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS), is(12f));

    BorderDrawable noneAndPart = new BorderDrawable();
    noneAndPart.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 5);
    noneAndPart.setBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS, 12);
    noneAndPart.setBounds(new Rect(0, 0, 400, 400));
    noneAndPart.draw(new Canvas());
    assertThat(noneAndPart.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS), is(5f));
    assertThat(noneAndPart.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS), is(BorderDrawable.DEFAULT_BORDER_WIDTH));
    assertThat(noneAndPart.getBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS), is(12f));
    assertThat(noneAndPart.getBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS), is(BorderDrawable.DEFAULT_BORDER_WIDTH));

    BorderDrawable fullAndPart = new BorderDrawable();
    fullAndPart.setBorderRadius(BorderDrawable.BORDER_RADIUS_ALL, 5);
    fullAndPart.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 12);
    fullAndPart.setBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS, 19);
    fullAndPart.setBounds(new Rect(0, 0, 400, 400));
    fullAndPart.draw(new Canvas());
    assertThat(fullAndPart.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS), is(12f));
    assertThat(fullAndPart.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS), is(5f));
    assertThat(fullAndPart.getBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS), is(19f));
    assertThat(fullAndPart.getBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS), is(5f));

    BorderDrawable partAndFull = new BorderDrawable();
    partAndFull.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 12);
    partAndFull.setBorderRadius(BorderDrawable.BORDER_RADIUS_ALL, 5);
    partAndFull.setBounds(new Rect(0, 0, 400, 400));
    partAndFull.draw(new Canvas());
    assertThat(partAndFull.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS), is(5f));
    assertThat(partAndFull.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS), is(5f));
    assertThat(partAndFull.getBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS), is(5f));
    assertThat(partAndFull.getBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS), is(5f));

    BorderDrawable noOverLapping = new BorderDrawable();
    noOverLapping.setBounds(new Rect(0, 0, 400, 400));
    noOverLapping.setBorderRadius(BorderDrawable.BORDER_RADIUS_ALL, 0);
    noOverLapping.draw(new Canvas());
    assertThat(noOverLapping.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS), is(0f));
    assertThat(noOverLapping.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS), is(0f));
    assertThat(noOverLapping.getBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS), is(0f));
    assertThat(noOverLapping.getBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS), is(0f));

    BorderDrawable overlappingOneLine = new BorderDrawable();
    overlappingOneLine.setBounds(new Rect(0, 0, 400, 400));
    overlappingOneLine.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 300);
    overlappingOneLine.setBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS, 200);
    overlappingOneLine.draw(new Canvas());
    assertThat(overlappingOneLine.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS), is(240f));
    assertThat(overlappingOneLine.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS), is(160f));
    assertThat(overlappingOneLine.getBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS), is
        (0f));
    assertThat(overlappingOneLine.getBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS), is
        (0f));

    BorderDrawable overlappingAdjoinLine = new BorderDrawable();
    overlappingAdjoinLine.setBounds(new Rect(0, 0, 400, 400));
    overlappingAdjoinLine.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 300);
    overlappingAdjoinLine.setBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS, 200);
    overlappingAdjoinLine.setBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS, 300);
    overlappingAdjoinLine.setBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS, 50);
    overlappingAdjoinLine.draw(new Canvas());
    assertThat(overlappingAdjoinLine.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS), is
        (240f));
    assertThat(overlappingAdjoinLine.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS), is
        (160f));
    assertThat(overlappingAdjoinLine.getBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS), is
        (240f));
    assertThat(overlappingAdjoinLine.getBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS),
               is(40f));

    BorderDrawable overlappingNonadjoinLine = new BorderDrawable();
    overlappingNonadjoinLine.setBounds(new Rect(0, 0, 400, 800));
    overlappingNonadjoinLine.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 300);
    overlappingNonadjoinLine.setBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS, 200);
    overlappingNonadjoinLine.setBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS, 300);
    overlappingNonadjoinLine.setBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS, 200);
    overlappingNonadjoinLine.draw(new Canvas());
    assertThat(overlappingNonadjoinLine.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS), is
        (240f));
    assertThat(overlappingNonadjoinLine.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS), is
        (160f));
    assertThat(overlappingNonadjoinLine.getBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS), is
        (240f));
    assertThat(overlappingNonadjoinLine.getBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS),
               is(160f));

    BorderDrawable overlappingThreeLines = new BorderDrawable();
    overlappingThreeLines.setBounds(new Rect(0, 0, 400, 500));
    overlappingThreeLines.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 300);
    overlappingThreeLines.setBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS, 200);
    overlappingThreeLines.setBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS, 600);
    overlappingThreeLines.setBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS, 100);
    overlappingThreeLines.draw(new Canvas());
    assertEquals(300f * 4 / 7,
                 overlappingThreeLines.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS),
                 .00001f);
    assertEquals(200f * 4 / 7, overlappingThreeLines.getBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS),
                 .00001f);
    assertEquals(600f * 4 / 7, overlappingThreeLines.getBorderRadius(BorderDrawable
                                                                         .BORDER_BOTTOM_RIGHT_RADIUS),
                 .00001f);
    assertEquals(100f * 4 / 7, overlappingThreeLines.getBorderRadius(BorderDrawable
                                                                         .BORDER_BOTTOM_LEFT_RADIUS),
                 .00001f);

    BorderDrawable overlappingFourLines = new BorderDrawable();
    overlappingFourLines.setBounds(new Rect(0, 0, 400, 500));
    overlappingFourLines.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 500);
    overlappingFourLines.setBorderRadius(BorderDrawable.BORDER_TOP_RIGHT_RADIUS, 600);
    overlappingFourLines.setBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS, 700);
    overlappingFourLines.setBorderRadius(BorderDrawable.BORDER_BOTTOM_LEFT_RADIUS, 800);
    overlappingFourLines.draw(new Canvas());
    assertEquals(500f * 4 / 15,
                 overlappingFourLines.getBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS),
                 .0001f);
    assertEquals(600f * 4 / 15, overlappingFourLines.getBorderRadius(BorderDrawable
                                                                         .BORDER_TOP_RIGHT_RADIUS),
                 .0001f);
    assertEquals(700f * 4 / 15, overlappingFourLines.getBorderRadius(BorderDrawable
                                                                         .BORDER_BOTTOM_RIGHT_RADIUS),
                 .0001f);
    assertEquals(800f * 4 / 15, overlappingFourLines.getBorderRadius(BorderDrawable
                                                                         .BORDER_BOTTOM_LEFT_RADIUS),
                 .0001f);
  }

  @Test
  public void testBorderColor() throws Exception {
    BorderDrawable none = new BorderDrawable();
    assertThat(none.getBorderColor(Spacing.ALL), is(BorderDrawable.DEFAULT_BORDER_COLOR));
    assertThat(none.getBorderColor(Spacing.LEFT), is(BorderDrawable.DEFAULT_BORDER_COLOR));
    assertThat(none.getBorderColor(Spacing.RIGHT), is(BorderDrawable.DEFAULT_BORDER_COLOR));
    assertThat(none.getBorderColor(Spacing.TOP), is(BorderDrawable.DEFAULT_BORDER_COLOR));
    assertThat(none.getBorderColor(Spacing.BOTTOM), is(BorderDrawable.DEFAULT_BORDER_COLOR));

    BorderDrawable full = new BorderDrawable();
    full.setBorderColor(Spacing.ALL, Color.RED);
    assertThat(full.getBorderColor(Spacing.ALL), is(Color.RED));
    assertThat(full.getBorderColor(Spacing.LEFT), is(Color.RED));
    assertThat(full.getBorderColor(Spacing.RIGHT), is(Color.RED));
    assertThat(full.getBorderColor(Spacing.TOP), is(Color.RED));
    assertThat(full.getBorderColor(Spacing.BOTTOM), is(Color.RED));

    BorderDrawable noneAndPart = new BorderDrawable();
    noneAndPart.setBorderColor(Spacing.LEFT, Color.BLUE);
    noneAndPart.setBorderColor(Spacing.TOP, Color.GREEN);
    assertThat(noneAndPart.getBorderColor(Spacing.LEFT), is(Color.BLUE));
    assertThat(noneAndPart.getBorderColor(Spacing.RIGHT), is(BorderDrawable.DEFAULT_BORDER_COLOR));
    assertThat(noneAndPart.getBorderColor(Spacing.TOP), is(Color.GREEN));
    assertThat(noneAndPart.getBorderColor(Spacing.BOTTOM), is(BorderDrawable.DEFAULT_BORDER_COLOR));

    BorderDrawable fullAndPart = new BorderDrawable();
    fullAndPart.setBorderColor(Spacing.ALL, Color.BLUE);
    fullAndPart.setBorderColor(Spacing.LEFT, Color.GREEN);
    fullAndPart.setBorderColor(Spacing.TOP, Color.RED);
    assertThat(fullAndPart.getBorderColor(Spacing.LEFT), is(Color.GREEN));
    assertThat(fullAndPart.getBorderColor(Spacing.RIGHT), is(Color.BLUE));
    assertThat(fullAndPart.getBorderColor(Spacing.TOP), is(Color.RED));
    assertThat(fullAndPart.getBorderColor(Spacing.BOTTOM), is(Color.BLUE));

    BorderDrawable partAndFull = new BorderDrawable();
    partAndFull.setBorderColor(Spacing.LEFT, Color.RED);
    partAndFull.setBorderColor(Spacing.ALL, Color.BLUE);
    assertThat(partAndFull.getBorderColor(Spacing.ALL), is(Color.BLUE));
    assertThat(partAndFull.getBorderColor(Spacing.LEFT), is(Color.BLUE));
    assertThat(partAndFull.getBorderColor(Spacing.RIGHT), is(Color.BLUE));
    assertThat(partAndFull.getBorderColor(Spacing.TOP), is(Color.BLUE));
    assertThat(partAndFull.getBorderColor(Spacing.BOTTOM), is(Color.BLUE));

  }

  @Test
  public void testBorderStyle() throws Exception {
    BorderDrawable none = new BorderDrawable();
    assertThat(none.getBorderStyle(Spacing.ALL), is(BorderStyle.SOLID.ordinal()));
    assertThat(none.getBorderStyle(Spacing.LEFT), is(BorderStyle.SOLID.ordinal()));
    assertThat(none.getBorderStyle(Spacing.RIGHT), is(BorderStyle.SOLID.ordinal()));
    assertThat(none.getBorderStyle(Spacing.TOP), is(BorderStyle.SOLID.ordinal()));
    assertThat(none.getBorderStyle(Spacing.BOTTOM), is(BorderStyle.SOLID.ordinal()));

    BorderDrawable full = new BorderDrawable();
    full.setBorderStyle(Spacing.ALL, BorderStyle.DOTTED.name());
    assertThat(full.getBorderStyle(Spacing.ALL), is(BorderStyle.DOTTED.ordinal()));
    assertThat(full.getBorderStyle(Spacing.LEFT), is(BorderStyle.DOTTED.ordinal()));
    assertThat(full.getBorderStyle(Spacing.RIGHT), is(BorderStyle.DOTTED.ordinal()));
    assertThat(full.getBorderStyle(Spacing.TOP), is(BorderStyle.DOTTED.ordinal()));
    assertThat(full.getBorderStyle(Spacing.BOTTOM), is(BorderStyle.DOTTED.ordinal()));

    BorderDrawable noneAndPart = new BorderDrawable();
    noneAndPart.setBorderStyle(Spacing.LEFT, BorderStyle.DOTTED.name());
    noneAndPart.setBorderStyle(Spacing.TOP, BorderStyle.DASHED.name());
    assertThat(noneAndPart.getBorderStyle(Spacing.LEFT), is(BorderStyle.DOTTED.ordinal()));
    assertThat(noneAndPart.getBorderStyle(Spacing.RIGHT), is(BorderStyle.SOLID.ordinal()));
    assertThat(noneAndPart.getBorderStyle(Spacing.TOP), is(BorderStyle.DASHED.ordinal()));
    assertThat(noneAndPart.getBorderStyle(Spacing.BOTTOM), is(BorderStyle.SOLID.ordinal()));

    BorderDrawable fullAndPart = new BorderDrawable();
    fullAndPart.setBorderStyle(Spacing.ALL, BorderStyle.DASHED.name());
    fullAndPart.setBorderStyle(Spacing.LEFT, BorderStyle.DOTTED.name());
    fullAndPart.setBorderStyle(Spacing.TOP, BorderStyle.SOLID.name());
    assertThat(fullAndPart.getBorderStyle(Spacing.LEFT), is(BorderStyle.DOTTED.ordinal()));
    assertThat(fullAndPart.getBorderStyle(Spacing.RIGHT), is(BorderStyle.DASHED.ordinal()));
    assertThat(fullAndPart.getBorderStyle(Spacing.TOP), is(BorderStyle.SOLID.ordinal()));
    assertThat(fullAndPart.getBorderStyle(Spacing.BOTTOM), is(BorderStyle.DASHED.ordinal()));

    BorderDrawable partAndFull = new BorderDrawable();
    partAndFull.setBorderStyle(Spacing.LEFT, BorderStyle.DASHED.name());
    partAndFull.setBorderStyle(Spacing.ALL, BorderStyle.DOTTED.name());
    assertThat(partAndFull.getBorderStyle(Spacing.ALL), is(BorderStyle.DOTTED.ordinal()));
    assertThat(partAndFull.getBorderStyle(Spacing.LEFT), is(BorderStyle.DOTTED.ordinal()));
    assertThat(partAndFull.getBorderStyle(Spacing.RIGHT), is(BorderStyle.DOTTED.ordinal()));
    assertThat(partAndFull.getBorderStyle(Spacing.TOP), is(BorderStyle.DOTTED.ordinal()));
    assertThat(partAndFull.getBorderStyle(Spacing.BOTTOM), is(BorderStyle.DOTTED.ordinal()));
  }

  @Test
  public void testIsRounded(){
    BorderDrawable none = new BorderDrawable();
    assertThat(none.isRounded(), is(false));

    BorderDrawable full = new BorderDrawable();
    full.setBorderRadius(BorderDrawable.BORDER_RADIUS_ALL, 12);
    assertThat(full.isRounded(), is(true));

    BorderDrawable noneAndPart = new BorderDrawable();
    noneAndPart.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 5);
    noneAndPart.setBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS, 12);
    assertThat(noneAndPart.isRounded(), is(true));

    BorderDrawable fullAndPart = new BorderDrawable();
    fullAndPart.setBorderRadius(BorderDrawable.BORDER_RADIUS_ALL, 0);
    fullAndPart.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 12);
    fullAndPart.setBorderRadius(BorderDrawable.BORDER_BOTTOM_RIGHT_RADIUS, 19);
    assertThat(fullAndPart.isRounded(), is(true));

    BorderDrawable partAndFull = new BorderDrawable();
    partAndFull.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 12);
    partAndFull.setBorderRadius(BorderDrawable.BORDER_RADIUS_ALL, 0);
    assertThat(partAndFull.isRounded(), is(true));

    BorderDrawable zeroAll = new BorderDrawable();
    zeroAll.setBorderRadius(BorderDrawable.BORDER_RADIUS_ALL, 0);
    assertThat(zeroAll.isRounded(), is(false));

    BorderDrawable zeroPart = new BorderDrawable();
    zeroPart.setBorderRadius(BorderDrawable.BORDER_TOP_LEFT_RADIUS, 0);
    assertThat(zeroPart.isRounded(), is(false));
  }
}
