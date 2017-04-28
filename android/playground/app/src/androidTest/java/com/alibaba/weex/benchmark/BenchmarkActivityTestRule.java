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

package com.alibaba.weex.benchmark;

import android.support.test.espresso.Espresso;
import android.support.test.espresso.idling.CountingIdlingResource;
import android.support.test.rule.ActivityTestRule;

import com.alibaba.weex.BenchmarkActivity;

class BenchmarkActivityTestRule extends ActivityTestRule<BenchmarkActivity> {

  BenchmarkActivityTestRule(Class<BenchmarkActivity> activityClass) {
    this(activityClass,true);
  }

  BenchmarkActivityTestRule(Class<BenchmarkActivity> activityClass,boolean launchActivity) {
    super(activityClass,false,launchActivity);
  }

  @Override
  protected void beforeActivityLaunched() {
    if(BenchmarkActivity.countingIdlingResource==null) {
      BenchmarkActivity.countingIdlingResource = new CountingIdlingResource
          ("TC_Monitor_List_With_Append_Tree");
    }
    Espresso.registerIdlingResources(BenchmarkActivity.countingIdlingResource);
  }

  @Override
  protected void afterActivityFinished() {
    super.afterActivityLaunched();
    Espresso.unregisterIdlingResources(BenchmarkActivity.countingIdlingResource);
  }

}
