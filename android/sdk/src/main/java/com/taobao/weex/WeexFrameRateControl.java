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
package com.taobao.weex;

/**
 * Created by shiwentao on 2017/8/24.
 */

import android.annotation.TargetApi;
import android.os.Build;
import android.view.Choreographer;

public class WeexFrameRateControl {
    private VSyncListener mListener;
    private final Choreographer mChoreographer;
    private final Choreographer.FrameCallback mVSyncFrameCallback;

    public interface VSyncListener {
        void OnVSync();
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public WeexFrameRateControl(VSyncListener listener) {
        mListener = listener;
        mChoreographer = Choreographer.getInstance();
        mVSyncFrameCallback = new Choreographer.FrameCallback() {
            @Override
            public void doFrame(long frameTimeNanos) {
                mChoreographer.postFrameCallback(mVSyncFrameCallback);
                mListener.OnVSync();
            }
        };

    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public void start() {
        mChoreographer.postFrameCallback(mVSyncFrameCallback);
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public void stop() {
        mChoreographer.removeFrameCallback(mVSyncFrameCallback);
    }

}
