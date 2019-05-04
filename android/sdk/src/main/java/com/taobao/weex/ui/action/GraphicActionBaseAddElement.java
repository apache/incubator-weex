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
package com.taobao.weex.ui.action;

import android.support.annotation.RestrictTo;
import android.support.annotation.WorkerThread;

import com.taobao.weex.WXSDKInstance;

/**
 * Created by luciolong on 2019/1/23.
 */
public abstract class GraphicActionBaseAddElement extends BasicGraphicAction {
    public GraphicActionBaseAddElement(WXSDKInstance instance, String ref) {
        super(instance, ref);
    }

    @RestrictTo(RestrictTo.Scope.LIBRARY)
    @WorkerThread
    public void setRTL(boolean isRTL) {
        // empty
    }

    @RestrictTo(RestrictTo.Scope.LIBRARY)
    @WorkerThread
    public void setSize(GraphicSize graphicSize) {
        // empty
    }

    @RestrictTo(RestrictTo.Scope.LIBRARY)
    @WorkerThread
    public void setPosition(GraphicPosition position) {
        // empty
    }

    @RestrictTo(RestrictTo.Scope.LIBRARY)
    @WorkerThread
    public void setIndex(int index) {
        // empty
    }
}
