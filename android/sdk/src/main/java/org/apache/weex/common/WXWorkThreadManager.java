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
package org.apache.weex.common;

import android.support.annotation.RestrictTo;
import android.support.annotation.RestrictTo.Scope;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Class for managing work thread.
 *
 * <p>
 *  This class is for internal purpose,
 *  please don't use it directly unless you know what you are doing.
 * </p>
 */
@RestrictTo(Scope.LIBRARY_GROUP)
public class WXWorkThreadManager {

  private ExecutorService singleThreadExecutor;

  public WXWorkThreadManager() {
    singleThreadExecutor = Executors.newSingleThreadExecutor();
  }

  public void post(Runnable task) {
    if (singleThreadExecutor != null)
      singleThreadExecutor.execute(task);
  }

  /**
   * Destroy current instance
   */
  public void destroy() {
    if (singleThreadExecutor != null)
      singleThreadExecutor.shutdown();
    singleThreadExecutor = null;
  }
}
