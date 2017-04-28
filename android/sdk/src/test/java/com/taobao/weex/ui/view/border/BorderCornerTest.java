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

import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.RectF;
import android.support.annotation.Nullable;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import static org.hamcrest.CoreMatchers.is;
import static org.junit.Assert.*;

@RunWith(RobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class BorderCornerTest {

  private final static String TOP_LEFT = "TopLeft";
  private final static String TOP_RIGHT = "TopRight";
  private final static String BOTTOM_RIGHT = "BottomRight";
  private final static String BOTTOM_LEFT = "BottomLeft";

  private List<BorderCorner> borderCorners = new ArrayList<>();
  private RectF borderBox = new RectF(0, 0, 400, 400);
  private List<Float> radiusList = Arrays.asList(-10f, 0f, 10f, 25f, 50f, 75f, 150f);
  private List<Float> preWidthList = Arrays.asList(-10f, 0f, 10f, 50f, 100f);
  private List<Float> postWidthList = Arrays.asList(-10f, 0f, 10f, 50f, 100f);
  private List<String> cornerList = Arrays.asList(TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT);

  private
  @Nullable
  BorderCorner createBorderCorner(String corner, float
      radius, float preWidth, float postWidth, RectF borderBox) {
    switch (corner) {
      case TOP_LEFT:
        return new TopLeftCorner(radius, preWidth, postWidth, borderBox);
      case TOP_RIGHT:
        return new TopRightCorner(radius, preWidth, postWidth, borderBox);
      case BOTTOM_RIGHT:
        return new BottomRightCorner(radius, preWidth, postWidth, borderBox);
      case BOTTOM_LEFT:
        return new BottomLeftCorner(radius, preWidth, postWidth, borderBox);
      default:
        return null;
    }
  }

  @Before
  public void setUp() throws Exception {

  }

  @After
  public void tearDown() throws Exception {
    borderCorners.clear();
  }

  @Test
  public void testHasOuterCorner() throws Exception {
    for (float preWidth : preWidthList) {
      for (float postWidth : postWidthList) {
        for (String corner : cornerList) {
          assertThat(createBorderCorner(corner, 0f, preWidth, postWidth, borderBox).hasOuterCorner(), is(false));
          assertThat(createBorderCorner(corner, 50f, preWidth, postWidth, borderBox)
                         .hasOuterCorner(), is(true));
          assertThat(createBorderCorner(corner, -10f, preWidth, postWidth, borderBox)
                         .hasOuterCorner(), is(false));
        }
      }
    }
  }

  @Test
  public void testHasInnerCorner() throws Exception {
    for (String corner : cornerList) {
      for (float preWidth : preWidthList) {
        for (float postWidth : postWidthList) {
          assertThat(createBorderCorner(corner, -10, preWidth, postWidth, borderBox).hasInnerCorner()
              , is(false));
          assertThat(createBorderCorner(corner, 0, preWidth, postWidth, borderBox)
                         .hasInnerCorner()
              , is(false));
        }
      }
      assertThat(createBorderCorner(corner, 25, -10, -10, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, -10, 0, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, -10, 10, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, -10, 50, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, 0, -10, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, 0, 0, borderBox).hasInnerCorner()
          , is(true));
      assertThat(createBorderCorner(corner, 25, 0, 10, borderBox).hasInnerCorner()
          , is(true));
      assertThat(createBorderCorner(corner, 25, 0, 50, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, 10, -10, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, 10, 0, borderBox).hasInnerCorner()
          , is(true));
      assertThat(createBorderCorner(corner, 25, 10, 10, borderBox).hasInnerCorner()
          , is(true));
      assertThat(createBorderCorner(corner, 25, 10, 50, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, 50, -10, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, 50, 0, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, 50, 10, borderBox).hasInnerCorner()
          , is(false));
      assertThat(createBorderCorner(corner, 25, 50, 50, borderBox).hasInnerCorner()
          , is(false));
    }
  }

  @Test
  public void testGetCornerStart() throws Exception {
    PointF topLeft = createBorderCorner(TOP_LEFT, 25, 10, 50, borderBox)
        .getCornerStart();
    assertThat(topLeft.x, is(5f));
    assertThat(topLeft.y, is(25f));
    PointF topRight = createBorderCorner(TOP_RIGHT, 25, 10, 50, borderBox)
        .getCornerStart();
    assertThat(topRight.x, is(375f));
    assertThat(topRight.y, is(5f));
    PointF bottomRight = createBorderCorner(BOTTOM_RIGHT, 25, 10, 50, borderBox)
        .getCornerStart();
    assertThat(bottomRight.x, is(395f));
    assertThat(bottomRight.y, is(375f));
    PointF bottomLeft = createBorderCorner(BOTTOM_LEFT, 25, 10, 50, borderBox)
        .getCornerStart();
    assertThat(bottomLeft.x, is(25f));
    assertThat(bottomLeft.y, is(395f));
  }

  @Test
  public void testGetCornerEnd() throws Exception {
    PointF topLeft = createBorderCorner(TOP_LEFT, 0, 10, 50, borderBox)
        .getCornerEnd();
    assertThat(topLeft.x, is(5f));
    assertThat(topLeft.y, is(25f));
    PointF topRight = createBorderCorner(TOP_RIGHT, 0, 10, 50, borderBox)
        .getCornerEnd();
    assertThat(topRight.x, is(375f));
    assertThat(topRight.y, is(5f));
    PointF bottomRight = createBorderCorner(BOTTOM_RIGHT, 0, 10, 50, borderBox)
        .getCornerEnd();
    assertThat(bottomRight.x, is(395f));
    assertThat(bottomRight.y, is(375f));
    PointF bottomLeft = createBorderCorner(BOTTOM_LEFT, 0, 10, 50, borderBox)
        .getCornerEnd();
    assertThat(bottomLeft.x, is(25f));
    assertThat(bottomLeft.y, is(395f));
  }

  @Test
  public void testGetRoundCornerStart() throws Exception {
    PointF topLeft = createBorderCorner(TOP_LEFT, 25, 10, 50, borderBox)
        .getRoundCornerStart();
    assertThat(topLeft.x, is(5f));
    assertThat(topLeft.y, is(25f));
    PointF topRight = createBorderCorner(TOP_RIGHT, 25, 10, 50, borderBox)
        .getRoundCornerStart();
    assertThat(topRight.x, is(375f));
    assertThat(topRight.y, is(5f));
    PointF bottomRight = createBorderCorner(BOTTOM_RIGHT, 25, 10, 50, borderBox)
        .getRoundCornerStart();
    assertThat(bottomRight.x, is(395f));
    assertThat(bottomRight.y, is(375f));
    PointF bottomLeft = createBorderCorner(BOTTOM_LEFT, 25, 10, 50, borderBox)
        .getRoundCornerStart();
    assertThat(bottomLeft.x, is(25f));
    assertThat(bottomLeft.y, is(395f));
  }

  @Test
  public void testGetRoundCornerEnd() throws Exception {
    PointF topLeft = createBorderCorner(TOP_LEFT, 25, 10, 50, borderBox)
        .getRoundCornerEnd();
    assertThat(topLeft.x, is(25f));
    assertThat(topLeft.y, is(25f));
    PointF topRight = createBorderCorner(TOP_RIGHT, 25, 10, 50, borderBox)
        .getRoundCornerEnd();
    assertThat(topRight.x, is(375f));
    assertThat(topRight.y, is(25f));
    PointF bottomRight = createBorderCorner(BOTTOM_RIGHT, 25, 10, 50, borderBox)
        .getRoundCornerEnd();
    assertThat(bottomRight.x, is(375f));
    assertThat(bottomRight.y, is(375f));
    PointF bottomLeft = createBorderCorner(BOTTOM_LEFT, 25, 10, 50, borderBox)
        .getRoundCornerEnd();
    assertThat(bottomLeft.x, is(25f));
    assertThat(bottomLeft.y, is(375f));
  }

  @Test
  public void testGetSharpCornerVertex() throws Exception {
    PointF topLeft = createBorderCorner(TOP_LEFT, 0, 10, 50, borderBox)
        .getSharpCornerVertex();
    assertThat(topLeft.x, is(5f));
    assertThat(topLeft.y, is(25f));
    PointF topRight = createBorderCorner(TOP_RIGHT, 0, 10, 50, borderBox)
        .getSharpCornerVertex();
    assertThat(topRight.x, is(375f));
    assertThat(topRight.y, is(5f));
    PointF bottomRight = createBorderCorner(BOTTOM_RIGHT, 0, 10, 50, borderBox)
        .getSharpCornerVertex();
    assertThat(bottomRight.x, is(395f));
    assertThat(bottomRight.y, is(375f));
    PointF bottomLeft = createBorderCorner(BOTTOM_LEFT, 0, 10, 50, borderBox)
        .getSharpCornerVertex();
    assertThat(bottomLeft.x, is(25f));
    assertThat(bottomLeft.y, is(395f));
  }

  @Test
  public void testGetSharpCornerStart() throws Exception {
    PointF topLeft = createBorderCorner(TOP_LEFT, 0, 10, 50, borderBox)
        .getSharpCornerStart();
    assertThat(topLeft.x, is(0f));
    assertThat(topLeft.y, is(25f));
    PointF topRight = createBorderCorner(TOP_RIGHT, 0, 10, 50, borderBox)
        .getSharpCornerStart();
    assertThat(topRight.x, is(375f));
    assertThat(topRight.y, is(0f));
    PointF bottomRight = createBorderCorner(BOTTOM_RIGHT, 0, 10, 50, borderBox)
        .getSharpCornerStart();
    assertThat(bottomRight.x, is(400f));
    assertThat(bottomRight.y, is(375f));
    PointF bottomLeft = createBorderCorner(BOTTOM_LEFT, 0, 10, 50, borderBox)
        .getSharpCornerStart();
    assertThat(bottomLeft.x, is(25f));
    assertThat(bottomLeft.y, is(400f));
  }

  @Test
  public void testGetSharpCornerEnd() throws Exception {
    PointF topLeft = createBorderCorner(TOP_LEFT, 0, 10, 50, borderBox)
        .getSharpCornerEnd();
    assertThat(topLeft.x, is(5f));
    assertThat(topLeft.y, is(0f));
    PointF topRight = createBorderCorner(TOP_RIGHT, 0, 10, 50, borderBox)
        .getSharpCornerEnd();
    assertThat(topRight.x, is(400f));
    assertThat(topRight.y, is(5f));
    PointF bottomRight = createBorderCorner(BOTTOM_RIGHT, 0, 10, 50, borderBox)
        .getSharpCornerEnd();
    assertThat(bottomRight.x, is(395f));
    assertThat(bottomRight.y, is(400f));
    PointF bottomLeft = createBorderCorner(BOTTOM_LEFT, 0, 10, 50, borderBox)
        .getSharpCornerEnd();
    assertThat(bottomLeft.x, is(0f));
    assertThat(bottomLeft.y, is(395f));
  }

  @Test
  public void testGetOvalIfInnerCornerExist() throws Exception {
    RectF topLeft = createBorderCorner(TOP_LEFT, 100, 10, 50, borderBox)
        .getOvalIfInnerCornerExist();
    assertThat(topLeft, is(new RectF(5, 25, 195, 175)));
    RectF topRight = createBorderCorner(TOP_RIGHT, 100, 10, 50, borderBox)
        .getOvalIfInnerCornerExist();
    assertThat(topRight, is(new RectF(225, 5, 375, 195)));
    RectF bottomRight = createBorderCorner(BOTTOM_RIGHT, 100, 10, 50, borderBox)
        .getOvalIfInnerCornerExist();
    assertThat(bottomRight, is(new RectF(205, 225, 395, 375)));
    RectF bottomLeft = createBorderCorner(BOTTOM_LEFT, 100, 10, 50, borderBox)
        .getOvalIfInnerCornerExist();
    assertThat(bottomLeft, is(new RectF(25, 205, 175, 395)));
  }

  @Test
  public void testGetOvalIfInnerCornerNotExist() throws Exception {
    RectF topLeft = createBorderCorner(TOP_LEFT, 50, 10, 50, borderBox).getOvalIfInnerCornerNotExist();
    assertThat(topLeft, is(new RectF(25, 25, 75, 75)));
    RectF topRight = createBorderCorner(TOP_RIGHT, 50, 10, 50, borderBox).getOvalIfInnerCornerNotExist();
    assertThat(topRight, is(new RectF(325, 25, 375, 75)));
    RectF bottomRight = createBorderCorner(BOTTOM_RIGHT, 50, 10, 50, borderBox).getOvalIfInnerCornerNotExist();
    assertThat(bottomRight, is(new RectF(325, 325, 375, 375)));
    RectF bottomLeft = createBorderCorner(BOTTOM_LEFT, 50, 10, 50, borderBox)
        .getOvalIfInnerCornerNotExist();
    assertThat(bottomLeft, is(new RectF(25, 325, 75, 375)));
  }

}
