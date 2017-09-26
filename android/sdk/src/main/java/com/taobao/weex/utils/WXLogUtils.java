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
package com.taobao.weex.utils;

import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.Log;

import com.taobao.weex.WXEnvironment;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.reflect.Method;
import java.util.HashMap;

public class WXLogUtils {

  public static final String WEEX_TAG = "weex";
  public static final String WEEX_PERF_TAG = "weex_perf";

  private static final String CLAZZ_NAME_DEBUG_TOOL = "com.taobao.weex.WXDebugTool";
  private static final String CLAZZ_NAME_LOG_UTIL = "com.taobao.weex.devtools.common.LogUtil";

  private static StringBuilder builder = new StringBuilder(50);
  private static HashMap<String, Class> clazzMaps = new HashMap<>(2);
  private static JsLogWatcher jsLogWatcher;

  static {
    clazzMaps.put(CLAZZ_NAME_DEBUG_TOOL, loadClass(CLAZZ_NAME_DEBUG_TOOL));
    clazzMaps.put(CLAZZ_NAME_LOG_UTIL, loadClass(CLAZZ_NAME_LOG_UTIL));
  }

  private static Class loadClass(String clazzName) {
    Class<?> clazz = null;
    try {
      clazz = Class.forName(clazzName);
      if (clazz != null) {
        clazzMaps.put(clazzName, clazz);
      }
    } catch (ClassNotFoundException e) {
      // ignore
    }
    return clazz;
  }

  public static void renderPerformanceLog(String type, long time) {
    if (WXEnvironment.isApkDebugable() || WXEnvironment.isPerf()) {
      builder.setLength(0);
      builder.append("[render time]").append(type).append(":").append(time);
      Log.d(WEEX_PERF_TAG, builder.substring(0));
      writeConsoleLog("debug", builder.substring(0));
    }
  }

  private static void log(String tag, String msg, LogLevel level){
    if (WXEnvironment.isApkDebugable() && msg != null && WXEnvironment.sLogLevel.compare(level) >= 0) {
      Log.println(level.getPriority(),tag, msg);
      writeConsoleLog(level.getName(), msg);
      sendLog(level, msg);
    }
  }

  public static void d(String msg) {
    d(WEEX_TAG,msg);
  }

  public static void i(String msg) {
    i(WEEX_TAG,msg);
  }

  public static void info(String msg) {
    i(WEEX_TAG,msg);
  }

  public static void v(String msg) {
    v(WEEX_TAG,msg);
  }

  public static void w(String msg) {
    w(WEEX_TAG,msg);
  }

  public static void e(String msg) {
    e(WEEX_TAG,msg);
  }

  public static void d(String tag, byte[] msg) {
    d(tag,new String(msg));
  }

  public static void wtf(String msg){
    wtf(WEEX_TAG, msg);
  }

  public static void d(String tag, String msg) {
    if (WXEnvironment.isApkDebugable() && !TextUtils.isEmpty(msg) && WXEnvironment.sLogLevel.compare(LogLevel.DEBUG) >= 0) {
      Log.d(tag, msg);

      if ("jsLog".equals(tag) && jsLogWatcher != null) {
        if (msg.endsWith("__DEBUG")) {
          jsLogWatcher.onJsLog(Log.DEBUG, msg.replace("__DEBUG", ""));
        } else if (msg.endsWith("__INFO")) {
          jsLogWatcher.onJsLog(Log.DEBUG, msg.replace("__INFO", ""));
        } else if (msg.endsWith("__WARN")) {
          jsLogWatcher.onJsLog(Log.DEBUG, msg.replace("__WARN", ""));
        } else if (msg.endsWith("__ERROR")) {
          jsLogWatcher.onJsLog(Log.DEBUG, msg.replace("__ERROR", ""));
        } else {
          jsLogWatcher.onJsLog(Log.DEBUG, msg);
        }
      }

      /** This log method will be invoked from jni code, so try to extract loglevel from message. **/
      writeConsoleLog("debug", tag + ":" + msg);
      if(msg.contains(" | __")){
        String[] msgs=msg.split(" | __");
        LogLevel level;
        if( msgs!=null && msgs.length==4 && !TextUtils.isEmpty(msgs[0]) && !TextUtils.isEmpty(msgs[2])){
          level=getLogLevel(msgs[2]);
          sendLog(level,msgs[0]);
          return;
        }
      }
      sendLog(LogLevel.DEBUG, tag + ":" + msg);
    }
  }

  private static LogLevel getLogLevel(String level) {
    switch (level.trim()){
      case "__ERROR":
        return LogLevel.ERROR;
      case "__WARN":
        return LogLevel.WARN;
      case "__INFO":
        return LogLevel.INFO;
      case "__LOG":
        return LogLevel.INFO;
      case "__DEBUG":
        return LogLevel.DEBUG;
    }
    return LogLevel.DEBUG;
  }

  public static void i(String tag, String msg) {
    log(tag, msg,LogLevel.INFO);
  }

  public static void v(String tag, String msg) {
    log(tag, msg,LogLevel.VERBOSE);
  }

  public static void w(String tag, String msg) {
    log(tag, msg,LogLevel.WARN);
  }

  public static void e(String tag, String msg) {
    log(tag, msg,LogLevel.ERROR);
  }

  public static void wtf(String tag, String msg){
    log(tag, msg, LogLevel.WTF);
  }

  /**
   * 'p' for 'Performance', use {@link #WEEX_PERF_TAG}
   * @param msg
   */
  public static void p(String msg) {
    d(WEEX_PERF_TAG,msg);
  }

  public static void d(String prefix, Throwable e) {
    if (WXEnvironment.isApkDebugable()) {
      d(prefix + getStackTrace(e));
    }
  }

  public static void i(String prefix, Throwable e) {
    if (WXEnvironment.isApkDebugable()) {
      info(prefix + getStackTrace(e));
    }
  }

  public static void v(String prefix, Throwable e) {
    if (WXEnvironment.isApkDebugable()) {
      v(prefix + getStackTrace(e));
    }
  }

  public static void w(String prefix, Throwable e) {
    if (WXEnvironment.isApkDebugable()) {
      w(prefix + getStackTrace(e));
    }
  }

  public static void e(String prefix, Throwable e) {
    if (WXEnvironment.isApkDebugable()) {
      e(prefix + getStackTrace(e));
    }
  }

  public static void wtf(String prefix, Throwable e){
    if (WXEnvironment.isApkDebugable()) {
      wtf(prefix + getStackTrace(e));
    }
  }

  /**
   * 'p' for 'Performance', use {@link #WEEX_PERF_TAG}
   */
  public static void p(String prefix, Throwable e) {
    if (WXEnvironment.isApkDebugable()) {
      p(prefix + getStackTrace(e));
    }
  }

  public static void eTag(String tag, Throwable e) {
    if (WXEnvironment.isApkDebugable()) {
      e(tag, getStackTrace(e));
    }
  }

  public static String getStackTrace(@Nullable Throwable e) {
    if (e == null) {
      return "";
    }
    StringWriter sw = null;
    PrintWriter pw = null;
    try {
      sw = new StringWriter();
      pw = new PrintWriter(sw);
      e.printStackTrace(pw);
      pw.flush();
      sw.flush();
    } finally {
      if (sw != null) {
        try {
          sw.close();
        } catch (IOException e1) {
          e1.printStackTrace();
        }
      }
      if (pw != null) {
        pw.close();
      }
    }
    return sw.toString();
  }



  private static void writeConsoleLog(String level, String message) {
    if (WXEnvironment.isApkDebugable()) {
      try {
        Class<?> clazz = clazzMaps.get(CLAZZ_NAME_LOG_UTIL);
        if (clazz != null) {
          Method m = clazz.getMethod("log", String.class, String.class);
          m.invoke(clazz, level, message);
        }
      } catch (Exception e) {
        Log.d(WEEX_TAG, "LogUtil not found!");
      }
    }
  }

  private static void sendLog(LogLevel level, String msg) {
    if(WXEnvironment.isApkDebugable()){
      try {
        Class<?> clazz = clazzMaps.get(CLAZZ_NAME_DEBUG_TOOL);
        if (clazz != null) {
          Method m = clazz.getMethod("sendLog", LogLevel.class,String.class);
          m.invoke(clazz, level,msg);
        }
      } catch (Exception e) {
        Log.d(WEEX_TAG, "WXDebugTool not found!");
      }
    }
  }

  public static void setJsLogWatcher(JsLogWatcher watcher) {
    jsLogWatcher = watcher;
  }

  public interface JsLogWatcher {
    void onJsLog(int level, String log);
  }
}
