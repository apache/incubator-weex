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
package org.apache.weex.performance;

import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.adapter.IWXConfigAdapter;
import org.apache.weex.utils.WXViewUtils;
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

        if (!WXViewUtils.isViewVisible(v) || !checkParentVisible(v.getParent())){
            return false;
        }
        if (isInWhiteList(instance)){
            return false;
        }
        return !hasLeafViewOrSizeIgnore(v,3);
    }

    private static boolean isInWhiteList(WXSDKInstance instance){
        IWXConfigAdapter configAdapter = WXSDKManager.getInstance().getWxConfigAdapter();
        if (null == configAdapter){
            return false;
        }
        String whiteList = configAdapter.getConfig("wxapm","ws_white_list",null);
        if (TextUtils.isEmpty(whiteList)){
            return false;
        }
        try {
            String[] urlList = whiteList.split(";");
            for (String whiteUrl : urlList){
                if (instance.getBundleUrl() != null && instance.getBundleUrl().contains(whiteUrl)){
                    return true;
                }
            }
        }catch (Exception e){
            e.printStackTrace();
        }

        return false;
    }

    private static boolean checkParentVisible(ViewParent parent){
        //root view getParent is null
        if (!(parent instanceof View)){
            return true;
        }
        View vp = (View)parent;
        boolean visible = vp.getVisibility() == View.VISIBLE && vp.getAlpha()>0;
        if (!visible){
            return false;
        }
        return checkParentVisible(vp.getParent());
    }

    private static boolean hasLeafViewOrSizeIgnore(View v,int checkDeep) {

        if (!(v instanceof ViewGroup)) {
            return true;
        }

        if (checkDeep > 0){
            if ( v.getHeight() < 10 || v.getWidth() < 10) {
                return true;
            }
            checkDeep--;
        }

        ViewGroup group = (ViewGroup)v;
        for (int i = 0; i < group.getChildCount(); i++) {
            View child = group.getChildAt(i);
            boolean res = hasLeafViewOrSizeIgnore(child,checkDeep);
            if (res) {
                return true;
            }
        }
        return false;
    }

    /**
     * get instance viewTree && component tree msg
     */
    public static String takeViewTreeSnapShot(WXSDKInstance instance) {
        if (null == instance) {
            return "nullInstance";
        }
        View v = instance.getContainerView();
        JSONObject root = geViewDetailTreeMsg(v);
        if (null != root) {
            return root.toString();
        }
        return "";
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
                node.put("type", view.getClass().getSimpleName());
                ViewGroup group = (ViewGroup)view;
                for (int i = 0; i < group.getChildCount(); i++) {
                    node.put("child_"+i, geViewDetailTreeMsg(group.getChildAt(i)));
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
