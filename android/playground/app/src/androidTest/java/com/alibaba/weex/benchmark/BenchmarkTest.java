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

package com.alibaba.weex.benchmark;

import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.matcher.ViewMatchers.withClassName;
import static org.hamcrest.MatcherAssert.assertThat;

import android.support.test.InstrumentationRegistry;
import android.support.test.espresso.contrib.RecyclerViewActions;
import android.support.test.filters.SdkSuppress;
import android.support.test.runner.AndroidJUnit4;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.Direction;
import android.support.test.uiautomator.UiDevice;
import android.support.test.uiautomator.UiObject2;
import android.support.test.uiautomator.Until;
import android.text.TextUtils;
import android.util.Log;
import com.alibaba.weex.BenchmarkActivity;
import com.taobao.weex.ui.view.listview.WXRecyclerView;
import com.taobao.weex.utils.WXLogUtils;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import org.hamcrest.Matchers;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class BenchmarkTest {

  private static final String TAG = "benchmark";
  private static final int TIMES = 20;
  private static final int FLING_SPEED = 10000;
  private static final int SCROLL_SPEED = 5000;
  private static final int FRAMES = 120;
  private static final long WAIT_TIMEOUT = 10000;
  private static final float FPS = 30;
  private static final float FIRST_SCREEN_RENDER_TIME = 600F;
  private static List<Long> firstScreenRenderTime = new LinkedList<>();
  private static List<Long> flingFrameSeconds = new LinkedList<>();
  private static List<Long> scrollFrameSeconds = new LinkedList<>();
  private static final String DUMP_START = "QueueBufferDuration,\n";
  private static final String DUMP_END = "---PROFILEDATA---";
  private static final String DUMP_COMMAND = "dumpsys gfxinfo com.alibaba.weex framestats reset";

  @Rule
  public BenchmarkActivityTestRule mActivityRule = new BenchmarkActivityTestRule(
      BenchmarkActivity.class);
  @Rule
  public RepeatRule repeatRule = new RepeatRule();
  private UiDevice mUiDevice;

  @Before
  public void init() {
    mUiDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());
  }

  @Test
  public void testFirstScreenPerformance() {
    List<Long> localTotalTime = new ArrayList<>(TIMES);
    for (int i = 0; i < TIMES; i++) {
      long currentTime = calcTime();
      localTotalTime.add(currentTime);
      Log.d(TAG, "FIRST_SCREEN_RENDER_TIME (activity not kill) " + currentTime + "ms");
    }
    BoxPlot boxPlot = new BoxPlot(localTotalTime);
    Log.i(TAG, "Average firstScreenRenderTime (activity not kill) " + boxPlot.draw());
    assertThat(boxPlot.getAverage(), Matchers.lessThan(FIRST_SCREEN_RENDER_TIME));
  }

  @Repeat(TIMES)
  @Test
  public void testFirstFirstScreenPerformance() {
    long currentTime = calcTime();
    firstScreenRenderTime.add(currentTime);
    Log.d(TAG, "FIRST_SCREEN_RENDER_TIME (activity killed) " + currentTime + " ms");
  }

  @Repeat(TIMES)
  @Test
  @SdkSuppress(minSdkVersion = 23)
  public void testFlingFPS() {
    UiObject2 uiObject2 = loadPageForFPS();
    if (uiObject2 != null) {
      uiObject2.fling(Direction.DOWN, FLING_SPEED);
      uiObject2.fling(Direction.DOWN, FLING_SPEED);
      uiObject2.fling(Direction.DOWN, FLING_SPEED);
      uiObject2.fling(Direction.DOWN, FLING_SPEED);
      processGfxInfo(flingFrameSeconds);
    }
  }

  @Repeat(TIMES)
  @Test
  @SdkSuppress(minSdkVersion = 23)
  public void testScrollFPS() {
    UiObject2 uiObject2 = loadPageForFPS();
    if (uiObject2 != null) {
      uiObject2.scroll(Direction.DOWN, 6, SCROLL_SPEED);
      processGfxInfo(scrollFrameSeconds);
    }
  }

  @AfterClass
  public static void count() {
    BoxPlot boxPlot = new BoxPlot(firstScreenRenderTime);
    Log.i(TAG, "Average firstScreenRenderTime (activity killed) " + boxPlot.draw());
    assertThat(boxPlot.getAverage(), Matchers.lessThan(FIRST_SCREEN_RENDER_TIME));
    BoxPlot flingPlot = new BoxPlot(flingFrameSeconds);
    Log.i(TAG, "Average Fling FPS : " + flingPlot.draw());
    assertThat(1000 / flingPlot.getMedian(), Matchers.greaterThan(FPS));
    BoxPlot scrollPlot = new BoxPlot(scrollFrameSeconds);
    Log.i(TAG, "Average Scroll FPS : " + scrollPlot.draw());
    assertThat(1000 / scrollPlot.getMedian(), Matchers.greaterThan(FPS));
  }

  private UiObject2 loadPageForFPS() {
    BenchmarkActivity benchmarkActivity = mActivityRule.getActivity();
    benchmarkActivity.loadWeexPage();
    onView(withClassName(Matchers.is(WXRecyclerView.class.getName()))).perform(RecyclerViewActions.scrollToPosition(0));
    return mUiDevice.wait(Until.findObject(By.desc(BenchmarkActivity.ROOT)), WAIT_TIMEOUT);
  }

  private void processGfxInfo(List<Long> container) {
    try {
      String line;
      String[] columns;
      long timeStart, timeEnd, duration;
      String result = mUiDevice.executeShellCommand(DUMP_COMMAND);
      //histogramGfxInfo(result);
      result = result.substring(result.indexOf(DUMP_START), result.lastIndexOf(DUMP_END));
      result = result.substring(DUMP_START.length());
      BufferedReader bufferedReader = new BufferedReader(new StringReader(result));
      List<Long> list = createList(bufferedReader);
      //Collections.sort(list);
      //Log.d(TAG, list.toString());
      container.addAll(list);
      BoxPlot boxPlot = new BoxPlot(list);
      boxPlot.draw();
      Log.d(TAG, "FPS : " + boxPlot.getMedian() + " ms");
    } catch (IOException e) {
      WXLogUtils.e(TAG, WXLogUtils.getStackTrace(e));
    }
  }

  private List<Long> createList(BufferedReader bufferedReader) throws IOException {
    String line;
    String[] columns;
    long timeStart, timeEnd, duration;
    List<Long> list = new ArrayList<>(FRAMES);
    while (!TextUtils.isEmpty(line = bufferedReader.readLine())) {
      columns = line.split(",");
      if (Long.parseLong(columns[0]) == 0) {
        timeStart = Long.parseLong(columns[1]);
        timeEnd = Long.parseLong(columns[columns.length - 1]);
        duration = timeEnd - timeStart;
        if (duration > 0) {
          list.add(TimeUnit.MILLISECONDS.convert(duration, TimeUnit.NANOSECONDS));
        }
      }
    }
    return list;
  }

  private long calcTime() {
    BenchmarkActivity benchmarkActivity = mActivityRule.getActivity();
    benchmarkActivity.loadWeexPage("http://30.8.53.163:8080/complicated.js");
    onView(withClassName(Matchers.is(WXRecyclerView.class.getName()))).perform
        (RecyclerViewActions.scrollToPosition(0));
    return benchmarkActivity.getWXInstance().getWXPerformance().screenRenderTime;
  }

/*  private void histogramGfxInfo(String result) {
    try {
      String start = "HISTOGRAM: ";
      result = result.substring(result.indexOf(start));
      result = result.substring(start.length());
      BufferedReader bufferedReader = new BufferedReader(new StringReader(result));
      result = bufferedReader.readLine();
      List<Long> list = transformToLong(result.split("\\s"));
      Log.d(TAG, list.toString());
    } catch (IOException e) {
      WXLogUtils.e(TAG, WXLogUtils.getStackTrace(e));
    }
  }

  private List<Long> transformToLong(String[] string) {
    List<Long> array = new LinkedList<>();
    int count;
    long value;
    for (String item : string) {
      value = Long.parseLong(item.substring(0, item.indexOf("ms")));
      if (value > 0) {
        count = parseInt(item.substring(item.indexOf('=') + 1));
        for (int i = 0; i < count; i++) {
          array.add(value);
        }
      }
    }
    return array;
  }*/
}
