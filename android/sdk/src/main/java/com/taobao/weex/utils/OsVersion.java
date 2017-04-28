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
package com.taobao.weex.util;

/**
 * Android OS version utilities.
 */
public class OsVersion {
  private static boolean sIsAtLeastJB_MR2;


  static {
    final int v = getApiVersion();
    sIsAtLeastJB_MR2 = v >= android.os.Build.VERSION_CODES.JELLY_BEAN_MR2;
  }

  /**
   * @return True if the version of Android that we're running on is at
   * least Jelly Bean MR2(API level 18).
   */
  public static boolean isAtLeastJB_MR2() {
    return sIsAtLeastJB_MR2;
  }

  /**
   * @return The Android API version of the running OS.
   */
  public static int getApiVersion() {
    return android.os.Build.VERSION.SDK_INT;
  }
}
