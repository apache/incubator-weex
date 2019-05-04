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
import android.support.annotation.Nullable;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.ui.IActionCreator;

import java.util.Map;
import java.util.Set;

/**
 * Created by luciolong on 2019/1/23.
 */
public class GraphicActionCreator implements IActionCreator {
    @Override
    public GraphicActionBaseAddElement createGraphicActionAddElement(@NonNull WXSDKInstance instance,
                                                                     String ref,
                                                                     String componentType,
                                                                     String parentRef,
                                                                     int index,
                                                                     Map<String, String> style,
                                                                     Map<String, String> attributes,
                                                                     Set<String> events, float[] margins,
                                                                     float[] paddings,
                                                                     float[] borders) {
        return new GraphicActionAddElement(instance,
                ref, componentType, parentRef,
                index, style, attributes, events,
                margins, paddings, borders);
    }

    @Override
    public BasicGraphicAction createGraphicActionLayout(WXSDKInstance instance,
                                                        String ref,
                                                        GraphicPosition layoutPosition,
                                                        GraphicSize layoutSize,
                                                        boolean isRTL) {
        return new GraphicActionLayout(instance, ref, layoutPosition, layoutSize, isRTL);
    }

    @Override
    public IExecutable createActionInvokeMethod(String pageId, String ref, String method, JSONArray args) {
        return new ActionInvokeMethod(pageId, ref, method, args);
    }

    @Override
    public BasicGraphicAction createGraphicActionAddEvent(WXSDKInstance instance, String ref, Object event) {
        return new GraphicActionAddEvent(instance, ref,event);
    }

    @Override
    public BasicGraphicAction createGraphicActionAnimation(@NonNull WXSDKInstance instance,
                                                           @NonNull String ref,
                                                           @Nullable String animation,
                                                           @Nullable String callBack) {
        return new GraphicActionAnimation(instance, ref, animation, callBack);
    }

    @Override
    public BasicGraphicAction createGraphicActionCreateBody(@NonNull WXSDKInstance instance,
                                                            String ref,
                                                            String componentType,
                                                            Map<String, String> style,
                                                            Map<String, String> attributes,
                                                            Set<String> events,
                                                            float[] margins,
                                                            float[] paddings,
                                                            float[] borders) {
        return new GraphicActionCreateBody(instance, ref, componentType, style, attributes, events, margins, paddings, borders);
    }

    @Override
    public BasicGraphicAction createGraphicActionCreateFinish(@NonNull WXSDKInstance instance) {
        return new GraphicActionCreateFinish(instance);
    }

    @Override
    public BasicGraphicAction createGraphicActionMoveElement(WXSDKInstance instance, String ref, String parentRef, int index) {
        return new GraphicActionMoveElement(instance, ref, parentRef, index);
    }

    @Override
    public BasicGraphicAction createGraphicActionRemoveElement(WXSDKInstance instance, String ref) {
        return new GraphicActionRemoveElement(instance, ref);
    }

    @Override
    public BasicGraphicAction createGraphicActionRemoveEvent(WXSDKInstance instance, String ref, Object event) {
        return new GraphicActionRemoveEvent(instance, ref, event);
    }

    @Override
    public BasicGraphicAction createGraphicActionRenderSuccess(@NonNull WXSDKInstance instance) {
        return new GraphicActionRenderSuccess(instance);
    }

    @Override
    public BasicGraphicAction createGraphicActionScrollToElement(WXSDKInstance instance, String ref, JSONObject options) {
        return new GraphicActionScrollToElement(instance, ref, options);
    }

    @Override
    public BasicGraphicAction createGraphicActionUpdateAttr(WXSDKInstance instance, String ref, Map<String, String> attrs) {
        return new GraphicActionUpdateAttr(instance, ref, attrs);
    }

    @Override
    public BasicGraphicAction createGraphicActionUpdateStyle(WXSDKInstance instance,
                                                             String ref,
                                                             Map<String, Object> style,
                                                             Map<String, String> paddings,
                                                             Map<String, String> margins,
                                                             Map<String, String> borders) {
        return new GraphicActionUpdateStyle(instance, ref, style, paddings, margins, borders);
    }
}
