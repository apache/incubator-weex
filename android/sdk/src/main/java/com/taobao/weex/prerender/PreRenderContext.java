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
package com.taobao.weex.prerender;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.taobao.weex.WeexFrameRateControl;
import com.taobao.weex.ui.component.node.WXComponentNode;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by luciolong on 18/07/2018.
 */
public class PreRenderContext implements WeexFrameRateControl.VSyncListener {
    public static final int INTERCEPT_RENDER_CLOSE = 0;
    public static final int INTERCEPT_RENDER_OPEN = 1;

    // need intercept real render
    public AtomicInteger interceptRenderState = new AtomicInteger(INTERCEPT_RENDER_CLOSE);

    private WeexFrameRateControl mFrameRateControl;

    @Nullable
    public WXComponentNode rootNode;

    @NonNull
    public Map<String, WXComponentNode> nodeMap = new HashMap<>();

    public int width = 0;
    public int height = 0;

    public AtomicBoolean isRenderSuccess = new AtomicBoolean(false);

    @Override
    public void OnVSync() {
        if (rootNode != null) {
            rootNode.getWxInstance().OnVSync();
        }
    }

    public void onPreRender() {
        if (mFrameRateControl == null) {
            mFrameRateControl = new WeexFrameRateControl(this);
        }
        mFrameRateControl.start();
    }

    public void onRealRender() {
        if (mFrameRateControl != null) {
            mFrameRateControl.stop();
            mFrameRateControl = null;
        }
    }
}
