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
package org.apache.weex.utils;

import android.annotation.TargetApi;
import android.os.Build;

/**
 * Hepler class for systrace.
 *
 * Note that this will run only on JBMR2 or later.
 *
 * Trace will be enabled in debug production and be disabled in release
 * production, see build.gradle.
 * If you want to enable it in release, just set sEnabled to true.
 */
public class Trace {
  private static final String TAG = "Weex_Trace";
  private abstract static class AbstractTrace {
    abstract void beginSection(String sectionName);
    abstract void endSection();
  }

  private static final AbstractTrace sTrace;
  private static final boolean sEnabled;

  // Pick the correct trace class to handle tracing.
  static {
    // If you want to enable it in release, just set sEnabled to true.
    // If you turn sEnabled on, weex would trace logs on Java side
    // as well as on V8 side, please take care of turning sEnabled on
    // to avoid performance impact.
    // FIXME: weex sdk may use another build files to build and cause
    // compiling errors if these build files don't contain any
    // ENABLE_TRACE, just comment out the line below.
    //sEnabled = com.taobao.weappplus_sdk.BuildConfig.ENABLE_TRACE;
    sEnabled = false;

    if (sEnabled == true && OsVersion.isAtLeastJB_MR2()) {
      sTrace = new TraceJBMR2();
    } else {
      sTrace = new TraceDummy();
    }
  }

  public static final boolean getTraceEnabled() {
    return sEnabled;
  }

  /**
   * Writes a trace message to indicate that a given section of code has begun.
   * This call must be followed by a corresponding call to {@link #endSection()}
   * on the same thread.
   *
   * <p class="note"> If sectionName contains '|', '\n' and '\0', these characters
   * will be repaced with a space character.
   *
   * @param sectionName The name of code section to appear in the trace.
   */
  public static void beginSection(String sectionName) {
    android.util.Log.i(TAG, "beginSection() " + sectionName);
    sTrace.beginSection(sectionName);
  }

  /**
   * Writes a trace message to indicate that a given section of code has ended.
   * This call must be preceeded by a corresponding call to {@link #beginSection(String)}
   * on the same thread.
   */
  public static void endSection() {
    sTrace.endSection();
    android.util.Log.i(TAG, "endSection()");
  }

  @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
  private static final class TraceJBMR2 extends AbstractTrace {
    @Override
    void beginSection(String sectionName) {
      android.os.Trace.beginSection(sectionName);
    }

    @Override
    void endSection() {
      android.os.Trace.endSection();
    }
  }

  private static final class TraceDummy extends AbstractTrace {
    @Override
    void beginSection(String sectionName) {
    }

    @Override
    void endSection() {
    }
  }
}
