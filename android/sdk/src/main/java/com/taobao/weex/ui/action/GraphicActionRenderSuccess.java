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

import android.support.annotation.NonNull;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.ui.component.WXComponent;

public class GraphicActionRenderSuccess extends BasicGraphicAction {

    public GraphicActionRenderSuccess(@NonNull WXSDKInstance instance) {
        super(instance, "");
    }

    @Override
    public void executeAction() {
        final WXSDKInstance instance = getWXSDKIntance();
        if (instance == null || instance.getContext() == null) {
            return;
        }
        WXComponent component = instance.getRootComponent();
        int layoutWidth = 0;
        int layoutHeight = 0;
        if (null != component) {
            layoutWidth = (int) component.getLayoutWidth();
            layoutHeight = (int) component.getLayoutHeight();
        }
        instance.onRenderSuccess(layoutWidth, layoutHeight);
    }

}