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
package com.taobao.weex.dom;

import android.os.SystemClock;

import com.taobao.weex.dom.action.AbstractAddElementAction;
import com.taobao.weex.dom.action.TraceableAction;
import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.tracing.WXTracing;
import com.taobao.weex.ui.IWXRenderTask;

/**
 * Created by sospartan on 23/02/2017.
 */

class RenderActionTask implements IWXRenderTask {
  private final RenderAction mRenderTask;
  private final RenderActionContext mContext;
  private final long mStartMillis = SystemClock.uptimeMillis();

  public RenderActionTask(RenderAction action, RenderActionContext context){
    mRenderTask = action;
    mContext = context;
  }

  @Override
  public void execute() {
    if (WXTracing.isAvailable() && mRenderTask instanceof TraceableAction) {
      ((TraceableAction) mRenderTask).mUIQueueTime = SystemClock.uptimeMillis() - mStartMillis;
    }
    long start = System.currentTimeMillis();
    long uiNanos = System.nanoTime();
    mRenderTask.executeRender(mContext);

    if (WXTracing.isAvailable()) {
      uiNanos = System.nanoTime() - uiNanos;
      if (mRenderTask instanceof TraceableAction) {
        if (!(mRenderTask instanceof AbstractAddElementAction)) {
          WXTracing.TraceEvent uiExecuteEvent = WXTracing.newEvent("UIExecute", mContext.getInstance().getInstanceId(), ((TraceableAction) mRenderTask).mTracingEventId);
          uiExecuteEvent.duration = Stopwatch.nanosToMillis(uiNanos);
          uiExecuteEvent.ts = start;
          uiExecuteEvent.submit();
        }
        ((TraceableAction) mRenderTask).onFinishUIExecute();
      }
    }
  }
}
