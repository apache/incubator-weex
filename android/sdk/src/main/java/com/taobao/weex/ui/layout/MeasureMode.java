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
package com.taobao.weex.ui.layout;

import com.taobao.weex.base.CalledByNative;

public class MeasureMode {

  private static int EXACTLY = 0;

  private static int ATMOST = 1;

  private int MEASURE_MODE = ATMOST;

  public boolean isEXACTLY() {
    return MEASURE_MODE == EXACTLY ? true : false;
  }

  public boolean isATMOST() {
    return MEASURE_MODE == ATMOST ? true : false;
  }

  public void EXACTLY() {
    MEASURE_MODE = EXACTLY;
  }

  public void ATMOST() {
    MEASURE_MODE = ATMOST;
  }

  @CalledByNative
  public int getEXACTLY() {
    return EXACTLY;
  }

  @CalledByNative
  public int getATMOST() {
    return ATMOST;
  }

  @CalledByNative
  public static MeasureMode GET() {
    return new MeasureMode();
  }
}
