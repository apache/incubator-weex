/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.performance;

import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXConfigAdapter;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import org.json.JSONObject;

/**
 * @author zhongcang
 * @date 2019/5/30
 */
public class WhiteScreenUtils {

    public static boolean doWhiteScreenCheck() {
        IWXConfigAdapter configAdapter = WXSDKManager.getInstance().getWxConfigAdapter();
        if (null == configAdapter) {
            return false;
        }
        double randomValue = Math.random() * 100;
        double max = 100;
        try {
            String configValue = configAdapter.getConfig("wxapm", "new_ws_sampling", "100");
            max = Double.valueOf(configValue);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return randomValue < max;
    }

    public static boolean isWhiteScreen(WXSDKInstance instance) {
        if (null == instance) {
            return false;
        }
        View v = instance.getContainerView();
        if (!(v instanceof ViewGroup)) {
            return false;
        }
        ViewGroup group = (ViewGroup)v;
        if (group.getChildCount() == 0) {
            return true;
        }
        return !hasLeafView(group);
    }

    private static boolean hasLeafView(View v) {
        if (!(v instanceof ViewGroup)) {
            return true;
        }

        ViewGroup group = (ViewGroup)v;
        for (int i = 0; i < group.getChildCount(); i++) {
            View child = group.getChildAt(i);
            boolean res = hasLeafView(child);
            if (res) {
                return true;
            }
        }
        return false;
    }

    /**
     * get instance viewTree && component tree msg
     */
    public static String getViewMsg(WXSDKInstance instance) {
        if (null == instance) {
            return "nullInstance";
        }
        View v = instance.getContainerView();
        StringBuilder builder = new StringBuilder();
        WXComponent component = instance.getRootComponent();
        if (null != component) {
            builder.append("componentMsg:").append(getComponentTreeMsg(component)).append("-----");
        }
        if (null != v) {
            builder.append("viewTreeMsg:").append(geViewDetailTreeMsg(v));
        }
        return builder.toString();
    }

    private static String getComponentTreeMsg(WXComponent component) {
        if (null == component) {
            return "nullComponent";
        }

        if (!(component instanceof WXVContainer)) {
            return component.getRef();
        }
        WXVContainer container = (WXVContainer)component;
        StringBuilder builder = new StringBuilder();
        builder.append(" _start_ ");
        for (int i = 0; i < container.getChildCount(); i++) {
            builder.append(getComponentTreeMsg(container.getChild(i))).append(",");
        }
        builder.append(" _end_ ");
        return builder.toString();
    }

    private static JSONObject geViewDetailTreeMsg(View view) {
        if (null == view) {
            return null;
        }
        JSONObject node = new JSONObject();
        try {
            node.put("width", view.getWidth());
            node.put("height", view.getHeight());
            int[] location = new int[2];
            location[0] = -1;
            location[1] = -1;
            view.getLocationOnScreen(location);
            node.put("x", location[0]);
            node.put("y", location[1]);

            if (view instanceof ViewGroup) {
                node.put("type", "ViewGroup");
                ViewGroup group = (ViewGroup)view;
                for (int i = 0; i < group.getChildCount(); i++) {
                    node.put("child", geViewDetailTreeMsg(group.getChildAt(i)));
                }
            } else {
                node.put("type", view.getClass().getSimpleName());
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
        return node;
    }
}
