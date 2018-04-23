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
package com.taobao.weex.common;

/**
 * Created by darin on 27/03/2018.
 */

public interface IWXJsFunctions {

    void initWxBridge(Object bridge, String className);

    void jsHandleSetJSVersion(String jsVersion);

    void jsHandleReportException(String instanceId, String func, String exceptionString);

    void jsHandleCallNative(String instanceId, byte[] tasks, String callback);

    void jsHandleCallNativeModule(String instanceId, String module, String method, byte[]
            arguments, byte[] options);

    void jsHandleCallNativeComponent(String instanceId, String componentRef, String method,
                                     byte[] arguments, byte[] options);

    void jsHandleCallAddElement(String instanceId, String ref, String dom, String index);

    void jsHandleSetTimeout(String callbackId, String time);

    void jsHandleCallNativeLog(byte[] str_array);

    void jsFunctionCallCreateBody(String pageId, String domStr);

    void jsFunctionCallUpdateFinish(String instanceId, byte[] tasks, String callback);

    void jsFunctionCallCreateFinish(String pageId);

    void jsFunctionCallRefreshFinish(String instanceId, byte[] tasks, String callback);

    void jsFunctionCallUpdateAttrs(String pageId, String ref, String data);

    void jsFunctionCallUpdateStyle(String pageId, String ref, String data);

    void jsFunctionCallRemoveElement(String pageId, String ref);

    void jsFunctionCallMoveElement(String pageId, String ref, String parentRef, String index_str);

    void jsFunctionCallAddEvent(String pageId, String ref, String event);

    void jsFunctionCallRemoveEvent(String pageId, String ref, String event);

    void jsHandleSetInterval(String instanceId, String callbackId, String time);

    void jsHandleClearInterval(String instanceId, String callbackId);

    void jsHandleCallGCanvasLinkNative(String contextId, int type, String val);
}
