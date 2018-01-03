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
package com.taobao.weex.common;

import android.os.Handler;
import android.os.Handler.Callback;
import android.os.HandlerThread;
import android.os.Message;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Thread used in weex
 */
public class WXThread extends HandlerThread {

  private Handler mHandler;

  static class SafeRunnable implements Runnable {

    static final String TAG = "SafeRunnable";

    final Runnable mTask;
    SafeRunnable(Runnable task){
      mTask = task;
    }

    @Override
    public void run() {
      try {
        if(mTask != null)
          mTask.run();
      }catch (Throwable e){
        //catch everything may throw from exection.
        if(WXEnvironment.isApkDebugable()){
          WXLogUtils.e(TAG,"SafeRunnable run throw expection:"+e.getMessage());
          throw e;
        }
        WXLogUtils.w(TAG, e);
      }
    }
  }

  static class SafeCallback implements Callback {
    static final String TAG = "SafeCallback";

    final Callback mCallback;
    SafeCallback(Callback cb){
      mCallback = cb;
    }

    @Override
    public boolean handleMessage(Message msg) {
      boolean result = false;
      try{
        if(mCallback != null){
          result = mCallback.handleMessage(msg);
        }
      }catch (Throwable e){
        //catch everything may throw from exection.
        if(WXEnvironment.isApkDebugable()){
          WXLogUtils.e(TAG,"SafeCallback handleMessage throw expection:"+e.getMessage());
          throw e;
        }
      }
      return result;
    }
  }

  /**
   * Secure Runnable to prevent throw during execution.
   * NOTE: DO NOT use this method to wrap runnable that may be removed by {@link Handler#removeCallbacks(Runnable)}
   * @param runnable
   * @return
   */
  public static Runnable secure(Runnable runnable){
    if(runnable == null || runnable instanceof SafeRunnable){
      return runnable;
    }
    return new SafeRunnable(runnable);
  }

  public static Callback secure(Callback callback){
    if(callback == null || callback instanceof SafeCallback){
      return callback;
    }

    return new SafeCallback(callback);
  }

  /**
   * @param name name of thread
   */
  public WXThread(String name) {
    super(name);
    start();
    mHandler = new Handler(getLooper());
  }


  public WXThread(String name, Callback callback) {
    super(name);
    start();
    mHandler = new Handler(getLooper(), secure(callback));
  }

  /**
   * @param name name of thread
   * @param priority The priority to run the thread at. The value supplied must be from
   *                 {@link android.os.Process} and not from java.lang.Thread.
   */
  public WXThread(String name, int priority, Callback callback) {
    super(name, priority);
    start();
    mHandler = new Handler(getLooper(), secure(callback));
  }

  /**
   * @param name name of thread
   * @param priority The priority to run the thread at. The value supplied must be from
   *                 {@link android.os.Process} and not from java.lang.Thread.
   */
  public WXThread(String name, int priority) {
    super(name, priority);
    start();
    mHandler = new Handler(getLooper());
  }

  public Handler getHandler() {
    return mHandler;
  }

  public boolean isWXThreadAlive() {
    return (mHandler != null && getLooper() != null && isAlive());
  }

  @Override
  public boolean quit() {
    if (mHandler != null) {
      mHandler.removeCallbacksAndMessages(null);
    }
    return super.quit();
  }

}
