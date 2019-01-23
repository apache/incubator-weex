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
package com.taobao.weex.ui;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.ui.action.BasicGraphicAction;
import com.taobao.weex.ui.action.GraphicActionBaseAddElement;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.ui.action.IExecutable;

import java.util.Map;
import java.util.Set;

/**
 * Created by luciolong on 2019/1/23.
 */
public interface IActionCreator {
    GraphicActionBaseAddElement createGraphicActionAddElement(@NonNull WXSDKInstance instance, String ref,
                                                        String componentType, String parentRef,
                                                        int index,
                                                        Map<String, String> style,
                                                        Map<String, String> attributes,
                                                        Set<String> events,
                                                        float[] margins,
                                                        float[] paddings,
                                                        float[] borders);

    BasicGraphicAction createGraphicActionLayout(WXSDKInstance instance,
                                                 String ref,
                                                 GraphicPosition layoutPosition,
                                                 GraphicSize layoutSize,
                                                 boolean isRTL);

    IExecutable createActionInvokeMethod(String pageId, String ref, String method, JSONArray args);

    BasicGraphicAction createGraphicActionAddEvent(WXSDKInstance instance, String ref, Object event);

    BasicGraphicAction createGraphicActionAnimation(@NonNull WXSDKInstance instance, @NonNull String ref, @Nullable String animation,
                           @Nullable final String callBack);

    BasicGraphicAction createGraphicActionCreateBody(@NonNull WXSDKInstance instance, String ref,
                                     String componentType,
                                     Map<String, String> style,
                                     Map<String, String> attributes,
                                     Set<String> events,
                                     float[] margins,
                                     float[] paddings,
                                     float[] borders);

    BasicGraphicAction createGraphicActionCreateFinish(@NonNull WXSDKInstance instance);

    BasicGraphicAction createGraphicActionMoveElement(WXSDKInstance instance, String ref, String parentRef, int index);

    BasicGraphicAction createGraphicActionRemoveElement(WXSDKInstance instance, String ref);

    BasicGraphicAction createGraphicActionRemoveEvent(WXSDKInstance instance, String ref, Object event);

    BasicGraphicAction createGraphicActionRenderSuccess(@NonNull WXSDKInstance instance);

    BasicGraphicAction createGraphicActionScrollToElement(WXSDKInstance instance, String ref, JSONObject options);

    BasicGraphicAction createGraphicActionUpdateAttr(WXSDKInstance instance, String ref,
                                     Map<String, String> attrs);

    BasicGraphicAction createGraphicActionUpdateStyle(WXSDKInstance instance, String ref,
                                      Map<String, Object> style,
                                      Map<String, String> paddings,
                                      Map<String, String> margins,
                                      Map<String, String> borders);
}
