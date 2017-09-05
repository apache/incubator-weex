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
package com.taobao.weex.tracing;

import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Created by moxun on 2017/6/2.
 */

public class Stopwatch {
  private static final ThreadLocal<Stopwatch> sThreadLocal = new ThreadLocal<>();
  private long startNanos;
  private List<ProcessEvent> splits = new ArrayList<>();
  private long startMillis;

  private static void prepare() {
    if (sThreadLocal.get() == null) {
      sThreadLocal.set(new Stopwatch());
    }
  }

  public static void tick() {
    if (WXTracing.isAvailable()) {
      try {
        prepare();
        if (sThreadLocal.get().startNanos != 0L) {
          WXLogUtils.w("Stopwatch", "Stopwatch is not reset");
        }
        sThreadLocal.get().startNanos = System.nanoTime();
        sThreadLocal.get().startMillis = System.currentTimeMillis();
      } catch (Throwable t) {
        t.printStackTrace();
      }
    }
  }

  public static void split(String fname) {
    if (WXTracing.isAvailable()) {
      try {
        ProcessEvent event = new ProcessEvent();
        long start = sThreadLocal.get().startMillis;
        double millis = tackAndTick();
        event.fname = fname;
        event.duration = millis;
        event.startMillis = start;
        sThreadLocal.get().splits.add(event);
      } catch (Throwable throwable) {
        throwable.printStackTrace();
      }
    }
  }

  public static List<ProcessEvent> getProcessEvents() {
    if (WXTracing.isAvailable()) {
      tack();
      List<ProcessEvent> existedEvents = sThreadLocal.get().splits;
      sThreadLocal.get().splits = new ArrayList<>();
      return existedEvents;
    } else {
      return Collections.emptyList();
    }
  }

  public static double tack() {
    if (WXTracing.isAvailable()) {
      try {
        long startNanos = sThreadLocal.get().startNanos;
        if (startNanos == 0L) {
          WXLogUtils.w("Stopwatch", "Should call Stopwatch.tick() before Stopwatch.tack() called");
        }
        long nanos = System.nanoTime() - startNanos;
        sThreadLocal.get().startNanos = 0L;
        return nanosToMillis(nanos);
      } catch (Throwable throwable) {
        throwable.printStackTrace();
      }
    }
    return -1;
  }

  public static long lastTickStamp() {
    if (WXTracing.isAvailable()) {
      try {
        return sThreadLocal.get().startMillis;
      } catch (Throwable t) {
        t.printStackTrace();
      }
    }
    return -1;
  }

  public static double tackAndTick() {
    double ms = tack();
    tick();
    return ms;
  }

  public static double nanosToMillis(long nanos) {
    return nanos / 1000000.0;
  }

  public static double millisUntilNow(long startNanos) {
    return nanosToMillis(System.nanoTime() - startNanos);
  }

  public static class ProcessEvent {
    public String fname;
    public double duration;
    public long startMillis;
  }
}
