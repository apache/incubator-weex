/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.performance;


import android.os.Build;
import android.os.Handler;
import android.support.annotation.RequiresApi;
import android.view.Choreographer;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXPerformance;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * @author chenpeihan
 * @date 2017/12/12
 */

public class FpsCollector {

  private Map<String, IFPSCallBack> mListenerMap = new ConcurrentHashMap<>();
  private AtomicBoolean mHasInit = new AtomicBoolean(false);


  private static class SingleTonHolder {

    private static FpsCollector INSTANCE = new FpsCollector();
  }

  public static FpsCollector getInstance() {
    return SingleTonHolder.INSTANCE;
  }

  public void init() {
    if (!WXEnvironment.isApkDebugable()) {
      return;
    }
    if (mHasInit.compareAndSet(false, true)) {
      WXSDKManager.getInstance().postOnUiThread(new Runnable() {
        @Override
        public void run() {
          if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN && WXPerformance.TRACE_DATA) {
            Choreographer.getInstance().postFrameCallback(new OnFrameListener());
          }
        }
      }, 0);

    }
  }


  @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
  private class OnFrameListener implements Choreographer.FrameCallback {

    private int mFrameCount = 0;
    private long mTimeBegin = 0;

    @Override
    public void doFrame(long frameTimeNanos) {
      Choreographer.getInstance().postFrameCallback(this);
      if (mTimeBegin == 0) {
        mTimeBegin = System.currentTimeMillis();
        mFrameCount++;
        return;
      }
      long timeDiff = System.currentTimeMillis() - mTimeBegin;
      if (timeDiff < 1000) {
        mFrameCount++;
        return;
      }
      for (Map.Entry<String, IFPSCallBack> entry : mListenerMap.entrySet()) {
        entry.getValue().fps(mFrameCount);
      }
      WXAnalyzerDataTransfer.transferFps(mFrameCount);

      mTimeBegin = 0;
      mFrameCount = 0;
    }
  }

  public interface IFPSCallBack {

    void fps(int fps);
  }


  public void registerListener(String key, IFPSCallBack listener) {
    mListenerMap.put(key, listener);
  }

  public void unRegister(String key) {
    mListenerMap.remove(key);
  }
}
