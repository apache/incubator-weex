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
package com.taobao.weex.render.component;

import android.content.Context;
import android.support.annotation.NonNull;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.Constants;
import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXViewUtils;

import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Created by furture on 2018/8/7.
 */

public class WXRenderFrameComponent extends WXVContainer<ViewGroup> {

    public static final String RENDER_FRAME_COMPONENT = "segment";

    public RenderView mRenderView;



    public static class Creator implements ComponentCreator {
        public WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
            return new WXRenderFrameComponent(instance, parent, basicComponentData);
        }
    }
    public WXRenderFrameComponent(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
       this(instance, parent, basicComponentData);
    }

    public WXRenderFrameComponent(WXSDKInstance instance, WXVContainer parent, boolean lazy, BasicComponentData basicComponentData) {
        this(instance, parent, basicComponentData);
    }

    public WXRenderFrameComponent(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
        super(instance, parent, basicComponentData);
    }


    @Override
    protected FrameLayout initComponentHostView(@NonNull Context context) {
        mRenderView = new RenderView(context);
        mRenderView.setRef(getRef());
        mRenderView.createSurfaceView();
        return mRenderView;
    }

    @Override
    public void addChild(WXComponent child, int index) {
        if(child != null){
            child.lazy(true);
        }
        super.addChild(child, index);
    }



    @Override
    public void onActivityResume() {
        super.onActivityResume();
    }

    @Override
    public void updateProperties(Map<String, Object> props) {}

    protected boolean setProperty(String key, Object param) {
        switch (key) {
            case Constants.Name.BACKGROUND_COLOR:
                return true;
            case Constants.Name.OPACITY:
                return true;
            case Constants.Name.BORDER_RADIUS:
            case Constants.Name.BORDER_TOP_LEFT_RADIUS:
            case Constants.Name.BORDER_TOP_RIGHT_RADIUS:
            case Constants.Name.BORDER_BOTTOM_RIGHT_RADIUS:
            case Constants.Name.BORDER_BOTTOM_LEFT_RADIUS:
                return true;
            case Constants.Name.BORDER_STYLE:
            case Constants.Name.BORDER_RIGHT_STYLE:
            case Constants.Name.BORDER_BOTTOM_STYLE:
            case Constants.Name.BORDER_LEFT_STYLE:
            case Constants.Name.BORDER_TOP_STYLE:
                return true;
            case Constants.Name.BORDER_COLOR:
            case Constants.Name.BORDER_TOP_COLOR:
            case Constants.Name.BORDER_RIGHT_COLOR:
            case Constants.Name.BORDER_BOTTOM_COLOR:
            case Constants.Name.BORDER_LEFT_COLOR:
                return true;
            case Constants.Name.BOX_SHADOW:
                return true;
            default:break;
        }
        return super.setProperty(key, param);
    }

    @Override
    public void setBorderRadius(String key, float borderRadius) {}

    @Override
    public void setBorderStyle(String key, String borderStyle) {}

    @Override
    public void setBorderWidth(String key, float borderWidth) {}

    @Override
    protected MeasureOutput measure(int width, int height) {
        MeasureOutput measureOutput = new MeasureOutput();
        int screenH = WXViewUtils.getScreenHeight(WXEnvironment.sApplication);
        int weexH = WXViewUtils.getWeexHeight(getInstanceId());
        measureOutput.height = height > (weexH >= screenH ? screenH : weexH) ? FrameLayout.LayoutParams.MATCH_PARENT
                : height;
        measureOutput.width = width;
        return measureOutput;
    }

    @Override
    public void destroy() {
        synchronized (this){
            super.destroy();
        }
        if(mRenderView != null){
            mRenderView.onDestroy();
        }
    }

    private Map<String, String> toMap(Map<String, Object> style){
        if(style == null){
            return null;
        }
        Map<String, String> maps = new HashMap<>();
        Set<Map.Entry<String,Object>> entries = style.entrySet();
        for(Map.Entry<String,Object> entry: entries){
            if(entry.getValue() == null){
                continue;
            }
            maps.put(entry.getKey(), entry.getValue().toString());
        }
        return maps;
    }

    @Override
    public void notifyAppearStateChange(String wxEventType, String direction) {

    }

    @Override
    public void setDemission(GraphicSize size, GraphicPosition position) {
        super.setDemission(size, position);
    }
}
