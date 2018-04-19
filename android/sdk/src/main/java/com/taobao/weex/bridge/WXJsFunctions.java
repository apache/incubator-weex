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
package com.taobao.weex.bridge;

import com.taobao.weex.common.IWXJsFunctions;

/**
 * Created by darin on 27/03/2018.
 */

public class WXJsFunctions implements IWXJsFunctions {
    @Override
    public native void initWxBridge(Object bridge, String className);

    @Override
    public native void jsHandleSetJSVersion(String jsVersion);

    @Override
    public native void jsHandleReportException(String instanceId, String func, String exceptionString);

    @Override
    public native void jsHandleCallNative(String instanceId, byte[] tasks, String callback);

    @Override
    public native void jsHandleCallNativeModule(String instanceId, String module, String method, byte[] arguments, byte[] options);


    @Override
    public native void jsHandleCallNativeComponent(String instanceId, String componentRef, String method, byte[] arguments, byte[] options);


    @Override
    public native void jsHandleCallAddElement(String instanceId, String ref, String dom, String index, String callback);


    @Override
    public native void jsHandleSetTimeout(String callbackId, String time);


    @Override
    public native void jsHandleCallNativeLog(byte[] str_array);


    @Override
    public native void jsFunctionCallCreateBody(String pageId, String domStr);


    @Override
    public native void jsFunctionCallUpdateFinish(String instanceId, byte[] tasks, String callback);


    @Override
    public native void jsFunctionCallCreateFinish(String pageId);


    @Override
    public native void jsFunctionCallRefreshFinish(String instanceId, byte[] tasks, String callback);


    @Override
    public native void jsFunctionCallUpdateAttrs(String pageId, String ref, String data);


    @Override
    public native void jsFunctionCallUpdateStyle(String pageId, String ref, String data);


    @Override
    public native void jsFunctionCallRemoveElement(String pageId, String ref);


    @Override
    public native void jsFunctionCallMoveElement(String pageId, String ref, String parentRef, String index_str);


    @Override
    public native void jsFunctionCallAddEvent(String pageId, String ref, String event);


    @Override
    public native void jsFunctionCallRemoveEvent(String pageId, String ref, String event);


    @Override
    public native void jsHandleSetInterval(String instanceId, String callbackId, String time);


    @Override
    public native void jsHandleClearInterval(String instanceId, String callbackId);


    @Override
    public native void jsHandleCallGCanvasLinkNative(String contextId, int type, String val);
}
