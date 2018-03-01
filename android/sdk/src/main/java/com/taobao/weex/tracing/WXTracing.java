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

import android.os.Looper;
import android.util.SparseArray;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.ITracingAdapter;
import com.taobao.weex.utils.WXLogUtils;

import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by moxun on 2017/6/6.
 */

public class WXTracing {
  private static final AtomicInteger sIdGenerator = new AtomicInteger(0);

  public static int nextId() {
    return sIdGenerator.getAndIncrement();
  }
  
  public static boolean isAvailable() {
    return WXEnvironment.isApkDebugable();
  }

  public static synchronized void submit(TraceEvent event) {
    ITracingAdapter tracingAdapter = WXSDKManager.getInstance().getTracingAdapter();
    if (tracingAdapter != null) {
      tracingAdapter.submitTracingEvent(event);
    }
  }

  public static class TraceEvent {
    public String fname;
    public String tname;
    public String ph;
    public int traceId;
    public long ts;
    public String iid;
    public String ref;
    public String parentRef;
    public String name;
    public String classname;
    public int parentId = -1;
    public double duration;

    /**
     * Internal use
     */
    public SparseArray<TraceEvent> subEvents;
    public String payload;
    public double parseJsonTime;
    public boolean isSegment;
    public Map<String, Object> extParams;
    public boolean firstScreenFinish;

    private boolean submitted;

    public TraceEvent() {
      ts = System.currentTimeMillis();
      traceId = nextId();
      tname = currentThreadName();
    }

    public void submit() {
      if (!submitted) {
        submitted = true;
        WXTracing.submit(this);
      } else {
        WXLogUtils.w("WXTracing", "Event " + traceId + " has been submitted.");
      }
    }
  }

  public static String currentThreadName() {
    Thread thread = Thread.currentThread();
    String name = thread.getName();

    if ("WeexJSBridgeThread".equals(name)) {
      return "JSThread";
    } else if ("WeeXDomThread".equals(name)) {
      return "DOMThread";
    } else if (Looper.getMainLooper() == Looper.myLooper()) {
      return "UIThread";
    }

    return name;
  }

  public static TraceEvent newEvent(String fname, String instanceId, int parentId) {
    WXTracing.TraceEvent traceEvent = new TraceEvent();
    traceEvent.fname = fname;
    traceEvent.iid = instanceId;
    traceEvent.traceId = WXTracing.nextId();
    traceEvent.parentId = parentId;
    return traceEvent;
  }

  public static class TraceInfo {
    public int rootEventId;
    public long domQueueTime;
    public long uiQueueTime;
    public long domThreadStart = -1;
    public long domThreadNanos;
    public long uiThreadStart = -1;
    public long uiThreadNanos;
  }
}
