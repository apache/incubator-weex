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
package com.taobao.weex.dom.action;

import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.tracing.WXTracing;

/**
 * Created by moxun on 2017/6/30.
 */

public class TraceableAction {
  public int mTracingEventId;
  public long mStartMillis;
  public long mDomQueueTime;
  public long mUIQueueTime;
  public long mParseJsonNanos = -1;
  protected WXTracing.TraceEvent mBeginEvent;

  {
    if (WXTracing.isAvailable()) {
      mTracingEventId = WXTracing.nextId();
      mStartMillis = System.currentTimeMillis();
    }
  }

  protected WXTracing.TraceEvent submitPerformance(String fname, String ph, String instanceId, double duration, long ts, boolean... isSegment) {
    if (WXTracing.isAvailable()) {
      WXTracing.TraceEvent event = WXTracing.newEvent(fname, instanceId, mTracingEventId);
      event.ts = ts;
      event.ph = ph;
      event.duration = duration;

      if (isSegment != null && isSegment.length == 1) {
        event.isSegment = isSegment[0];
      }

      event.submit();
      return event;
    }
    return new WXTracing.TraceEvent();
  }

  public void onStartDomExecute(String instanceId, String fname, String ref, String type, String payload) {
    if (WXTracing.isAvailable()) {
      mBeginEvent = WXTracing.newEvent(fname, instanceId, -1);
      mBeginEvent.traceId = mTracingEventId;
      mBeginEvent.ts = mStartMillis;
      mBeginEvent.ph = "B";
      mBeginEvent.ref = ref;
      mBeginEvent.name = type;
      mBeginEvent.payload = payload;
      mBeginEvent.parseJsonTime = Stopwatch.nanosToMillis(mParseJsonNanos);
      mBeginEvent.submit();
    }
  }

  public void onFinishUIExecute() {
    if (WXTracing.isAvailable() && mBeginEvent != null) {
      WXTracing.TraceEvent endEvent = WXTracing.newEvent(getClass().getSimpleName(), mBeginEvent.iid, -1);
      endEvent.traceId = mBeginEvent.traceId;
      endEvent.ph = "E";
      endEvent.submit();
    }
  }
}
