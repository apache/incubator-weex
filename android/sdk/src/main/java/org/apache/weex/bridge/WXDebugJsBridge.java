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
package org.apache.weex.bridge;

import com.alibaba.fastjson.JSON;
import org.apache.weex.utils.WXWsonJSONSwitch;
import org.apache.weex.wson.WsonUtils;

/**
 * Created by darin on 27/03/2018.
 */

public class WXDebugJsBridge {

    public native void resetWXBridge(Object bridge, String className);

    public native void jsHandleSetJSVersion(String jsVersion);

    public native void jsHandleReportException(String instanceId, String func, String exceptionString);

    public native void jsHandleCallNative(String instanceId, byte[] tasks, String callback);

    public void jsHandleCallNativeModule(String instanceId, String module, String method, byte[] arguments, byte[] options) {
        jsHandleCallNativeModule(instanceId, module, method,
                WXWsonJSONSwitch.convertJSONToWsonIfUseWson(arguments), WXWsonJSONSwitch.convertJSONToWsonIfUseWson(options), true);
    }

    public native void jsHandleCallNativeModule(String instanceId, String module, String method, byte[] arguments, byte[] options, boolean h5);

    public void jsHandleCallNativeComponent(String instanceId, String componentRef, String method, byte[] arguments, byte[] options){
        jsHandleCallNativeComponent(instanceId, componentRef, method, WXWsonJSONSwitch.convertJSONToWsonIfUseWson(arguments), WXWsonJSONSwitch.convertJSONToWsonIfUseWson(options), true);
    }


    public native void jsHandleCallNativeComponent(String instanceId, String componentRef, String method, byte[] arguments, byte[] options, boolean from);

    public void jsHandleCallAddElement(String instanceId, String ref, String dom, String index){
        jsHandleCallAddElement(instanceId, ref, WsonUtils.toWson(JSON.parse(dom)), index, true);
    }

    public native void jsHandleCallAddElement(String instanceId, String ref, byte[] dom, String index, boolean h5);

    public native void jsHandleSetTimeout(String callbackId, String time);

    public native void jsHandleCallNativeLog(byte[] str_array);

    public void jsHandleCallCreateBody(String pageId, String domStr){
        jsHandleCallCreateBody(pageId, WsonUtils.toWson(JSON.parse(domStr)), true);
    }

    public native void jsHandleCallCreateBody(String pageId, byte[] domStr, boolean h5);

    public native void jsHandleCallUpdateFinish(String instanceId, byte[] tasks, String callback);

    public native void jsHandleCallCreateFinish(String pageId);

    public native void jsHandleCallRefreshFinish(String instanceId, byte[] tasks, String callback);

    public void jsHandleCallUpdateAttrs(String pageId, String ref, String data){
        jsHandleCallUpdateAttrs(pageId, ref, WsonUtils.toWson(JSON.parseObject(data)), true);
    }

    public native void jsHandleCallUpdateAttrs(String pageId, String ref, byte[] data, boolean h5);

    public void jsHandleCallUpdateStyle(String pageId, String ref, String data){
        byte[] data1 = WsonUtils.toWson(JSON.parseObject(data));
        jsHandleCallUpdateStyleNative(pageId, ref, data1, true);
    }

    public native void jsHandleCallUpdateStyleNative(String pageId, String ref, byte[] data, boolean h5);

    public native void jsHandleCallRemoveElement(String pageId, String ref);

    public native void jsHandleCallMoveElement(String pageId, String ref, String parentRef, String index_str);

    public native void jsHandleCallAddEvent(String pageId, String ref, String event);

    public native void jsHandleCallRemoveEvent(String pageId, String ref, String event);

    public native void jsHandleSetInterval(String instanceId, String callbackId, String time);

    public native void jsHandleClearInterval(String instanceId, String callbackId);

    public native void jsHandleCallGCanvasLinkNative(String contextId, int type, String val);
}
