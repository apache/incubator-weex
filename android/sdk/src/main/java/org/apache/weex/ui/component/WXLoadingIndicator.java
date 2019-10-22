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
package org.apache.weex.ui.component;

import android.content.Context;
import android.graphics.Color;

import android.support.annotation.NonNull;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.annotation.Component;
import org.apache.weex.common.Constants;
import org.apache.weex.ui.action.BasicComponentData;
import org.apache.weex.ui.view.refresh.circlebar.CircleProgressBar;
import org.apache.weex.utils.WXResourceUtils;
import org.apache.weex.utils.WXUtils;

@Component(lazyload = false)

public class WXLoadingIndicator extends WXComponent<CircleProgressBar> {


    public WXLoadingIndicator(WXSDKInstance instance, WXVContainer parent, boolean isLazy, BasicComponentData basicComponentData) {
        super(instance, parent, isLazy, basicComponentData);
    }

    @Override
    protected CircleProgressBar initComponentHostView(@NonNull Context context) {
        return new CircleProgressBar(context);
    }

    @Override
    protected boolean setProperty(String key, Object param) {
        switch (key) {
            case Constants.Name.COLOR:
                String color = WXUtils.getString(param,null);
                if (color != null)
                    setColor(color);
                return true;
            case Constants.Name.ANIMATING:
                Boolean result = WXUtils.getBoolean(param, null);
                if (result != null) {
                    setAnimating(result);
                }
                return true;
        }
        return super.setProperty(key, param);
    }

    @WXComponentProp(name = Constants.Name.COLOR)
    public void setColor(String color) {
        if (color != null && !color.equals("")) {
            int parseColor = WXResourceUtils.getColor(color, Color.RED);
            getHostView().setColorSchemeColors(parseColor);
        }
    }

    @WXComponentProp(name = Constants.Name.ANIMATING)
    public void setAnimating(boolean animating) {
        if (animating) {
            getHostView().start();
        } else {
            getHostView().stop();
        }
    }
}
