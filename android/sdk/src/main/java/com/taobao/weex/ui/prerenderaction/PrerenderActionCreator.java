package com.taobao.weex.ui.prerenderaction;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.ui.IActionCreator;
import com.taobao.weex.ui.action.BasicGraphicAction;
import com.taobao.weex.ui.action.GraphicActionBaseAddElement;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.ui.action.IExecutable;

import java.util.Map;
import java.util.Set;

/**
 * Description
 * Created by luciolong on 2019/1/23.
 */
public class PrerenderActionCreator implements IActionCreator {
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
        return new PrerenderGraphicActionAddElement(instance,
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
        return new PrerenderGraphicActionLayout(instance, ref, layoutPosition, layoutSize, isRTL);
    }

    @Override
    public IExecutable createActionInvokeMethod(String pageId, String ref, String method, JSONArray args) {
        return new PrerenderActionInvokeMethod(pageId, ref, method, args);
    }

    @Override
    public BasicGraphicAction createGraphicActionAddEvent(WXSDKInstance instance, String ref, Object event) {
        return new PrerenderGraphicActionAddEvent(instance, ref,event);
    }

    @Override
    public BasicGraphicAction createGraphicActionAnimation(@NonNull WXSDKInstance instance,
                                                           @NonNull String ref,
                                                           @Nullable String animation,
                                                           @Nullable String callBack) {
        return new PrerenderGraphicActionAnimation(instance, ref, animation, callBack);
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
        return new PrerenderGraphicActionCreateBody(instance, ref, componentType, style, attributes, events, margins, paddings, borders);
    }

    @Override
    public BasicGraphicAction createGraphicActionCreateFinish(@NonNull WXSDKInstance instance) {
        return new PrerenderGraphicActionCreateFinish(instance);
    }

    @Override
    public BasicGraphicAction createGraphicActionMoveElement(WXSDKInstance instance, String ref, String parentRef, int index) {
        return new PrerenderGraphicActionMoveElement(instance, ref, parentRef, index);
    }

    @Override
    public BasicGraphicAction createGraphicActionRemoveElement(WXSDKInstance instance, String ref) {
        return new PrerenderGraphicActionRemoveElement(instance, ref);
    }

    @Override
    public BasicGraphicAction createGraphicActionRemoveEvent(WXSDKInstance instance, String ref, Object event) {
        return new PrerenderGraphicActionRemoveEvent(instance, ref, event);
    }

    @Override
    public BasicGraphicAction createGraphicActionRenderSuccess(@NonNull WXSDKInstance instance) {
        return new PrerenderGraphicActionRenderSuccess(instance);
    }

    @Override
    public BasicGraphicAction createGraphicActionScrollToElement(WXSDKInstance instance, String ref, JSONObject options) {
        return new PrerenderGraphicActionScrollToElement(instance, ref, options);
    }

    @Override
    public BasicGraphicAction createGraphicActionUpdateAttr(WXSDKInstance instance, String ref, Map<String, String> attrs) {
        return new PrerenderGraphicActionUpdateAttr(instance, ref, attrs);
    }

    @Override
    public BasicGraphicAction createGraphicActionUpdateStyle(WXSDKInstance instance,
                                                             String ref,
                                                             Map<String, Object> style,
                                                             Map<String, String> paddings,
                                                             Map<String, String> margins,
                                                             Map<String, String> borders) {
        return new PrerenderGraphicActionUpdateStyle(instance, ref, style, paddings, margins, borders);
    }
}
