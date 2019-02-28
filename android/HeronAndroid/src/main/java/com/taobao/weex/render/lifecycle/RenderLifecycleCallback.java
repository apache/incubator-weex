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
package com.taobao.weex.render.lifecycle;

import android.app.Activity;
import android.app.Application;
import android.content.ComponentCallbacks;
import android.content.res.Configuration;
import android.os.Bundle;

import com.taobao.weex.render.manager.RenderManager;

/**
 * Created by furture on 2018/8/31.
 */

public class RenderLifecycleCallback implements Application.ActivityLifecycleCallbacks, ComponentCallbacks {


    @Override
    public void onActivityCreated(Activity activity, Bundle savedInstanceState) {

    }

    @Override
    public void onActivityStarted(Activity activity) {

    }

    @Override
    public void onActivityResumed(Activity activity) {
        RenderManager.getInstance().onActivityResumed(activity);
    }

    @Override
    public void onActivityPaused(Activity activity) {
        RenderManager.getInstance().onActivityPaused(activity);
    }

    @Override
    public void onActivityStopped(Activity activity) {

    }

    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle outState) {

    }

    @Override
    public void onActivityDestroyed(Activity activity) {
        RenderManager.getInstance().onActivityDestroyed(activity);
    }

    @Override
    public void onConfigurationChanged(Configuration configuration) {}

    @Override
    public void onLowMemory() {
        RenderManager.getInstance().onLowMemory();
    }
}
