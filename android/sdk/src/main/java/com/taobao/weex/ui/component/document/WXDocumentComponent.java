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
package com.taobao.weex.ui.component.document;

import android.content.Context;
import android.support.annotation.NonNull;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.Constants;
import com.taobao.weex.render.event.DocumentAdapter;
import com.taobao.weex.render.event.OnEventListener;
import com.taobao.weex.render.event.OnImgLoadListener;
import com.taobao.weex.render.image.BitmapTarget;
import com.taobao.weex.render.view.DocumentTextureView;
import com.taobao.weex.render.view.DocumentView;
import com.taobao.weex.ui.ComponentCreator;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.ui.component.WXA;
import com.taobao.weex.ui.component.WXBasicComponentType;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXImage;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.utils.WXDataStructureUtil;

import java.lang.reflect.InvocationTargetException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Created by furture on 2018/8/7.
 */

public class WXDocumentComponent extends WXVContainer<ViewGroup> implements OnImgLoadListener, OnEventListener {

    public static final String DOCUMENT_COMPONENT = "segment";

    private DocumentView documentView;
    private DocumentAdapter documentAdapter;
    public  DocumentTextureView documentTextureView;
    private WXDocumentHelper documentHelper;
    private WXDocumentMeasurement documentMeasurement;
    private boolean documentShouldInited;

    public void updateWatchComponentStatus() {
        if(documentHelper != null){
            documentHelper.updateChildWatchEvents();
        }
    }



    public static class Ceator implements ComponentCreator {
        public WXComponent createInstance(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) throws IllegalAccessException, InvocationTargetException, InstantiationException {
            return new WXDocumentComponent(instance, parent, basicComponentData);
        }
    }
    public WXDocumentComponent(WXSDKInstance instance, WXVContainer parent, String instanceId, boolean isLazy, BasicComponentData basicComponentData) {
       this(instance, parent, basicComponentData);
    }

    public WXDocumentComponent(WXSDKInstance instance, WXVContainer parent, boolean lazy, BasicComponentData basicComponentData) {
        this(instance, parent, basicComponentData);
    }

    public WXDocumentComponent(WXSDKInstance instance, WXVContainer parent, BasicComponentData basicComponentData) {
        super(instance, parent, basicComponentData);
        lazy(true);
        instance.setHasDocumentSegment(true);
        actionCreateBody();
        setContentBoxMeasurement(documentMeasurement);
    }


    @Override
    protected FrameLayout initComponentHostView(@NonNull Context context) {
        documentTextureView = new DocumentTextureView(context);
        documentTextureView.setDocumentView(documentView);
        FrameLayout frameLayout = new FrameLayout(context);
        frameLayout.addView(documentTextureView, new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
       return frameLayout;
    }

    @Override
    public void addChild(WXComponent child, int index) {
        if(child != null){
            child.lazy(true);
        }
        super.addChild(child, index);
    }

    private void actionCreateBody(){
        documentView = new DocumentView(getContext());
        documentView.setInstance(getInstance());
        documentAdapter = new DocumentAdapter();
        documentAdapter.setSizeChangedOnMainThread(false);
        documentMeasurement = new WXDocumentMeasurement(this);
        documentAdapter.setOnDocumentSizeChangedListener(documentMeasurement);
        documentAdapter.setOnImgLoadListener(this);
        documentAdapter.setOnEventListener(this);
        documentView.setDocumentAdapter(documentAdapter);
        documentHelper = new WXDocumentHelper(this);


        String ref = getRef();
        Map<String, String> styles =  toMap(getStyles());
        Map<String, String> attrs = toMap(getAttrs());
        Collection<String> events = getEvents();
        actionCreateBody(ref, styles, attrs, events);
    }

    public void actionCreateBody(String ref, Map<String, String> style, Map<String, String> attrs, Collection<String> events){
        documentView.actionCreateBody(ref, style, attrs, events);
    }

    public void actionAddElement(String ref, String componentType, String parentRef, int index, Map<String, String> style, Map<String, String> attrs, Collection<String> events){
        documentView.actionAddElement(ref, componentType, parentRef, index, style, attrs, events);
    }

    public void actionUpdateStyles(String ref, Map<String, Object> styles){
        Map<String, String> stylesStrs = new HashMap<>();
        Set<Map.Entry<String, Object>> entries = styles.entrySet();
        for(Map.Entry<String, Object> entry : entries){
            stylesStrs.put(entry.getKey(), entry.getValue().toString());
        }
        documentView.actionUpdateStyles(ref, stylesStrs);
    }

    public void actionUpdateAttrs(String ref, Map<String, String> attrs){
        documentView.actionUpdateAttrs(ref, attrs);
    }

    public void actionAddEvent(String ref, Object event){
        documentView.actionAddEvent(ref, event.toString());

    }

    public void actionRemoveEvent(String ref, Object event) {
        documentView.actionRemoveEvent(ref, event.toString());
    }


    public void actionMoveElement(String ref, String parentRef, int index){
        documentView.actionMoveElement(ref, parentRef, index);
    }

    public void actionRemoveElement(String ref) {
        documentView.actionRemoveElement(ref);
    }



    @Override
    public void updateStyles(WXComponent component) {
        if(component != this){
            documentView.actionUpdateStyles(getRef(),toMap(component.getStyles()));
        }
    }
    @Override
    public void updateAttrs(WXComponent component) {
        if(component != this){
            documentView.actionUpdateAttrs(getRef(),toMap(component.getAttrs()));
        }
    }

    @Override
    public void updateStyles(Map<String, Object> styles) {
        documentView.actionUpdateStyles(getRef(),toMap(styles));
    }

    @Override
    public void updateAttrs(Map<String, Object> attrs) {
        documentView.actionUpdateAttrs(getRef(),toMap(attrs));
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
    public void destroy() {
        synchronized (this){
            super.destroy();
        }
        if(documentTextureView != null){
            documentTextureView.destroy();
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



    public static  WXDocumentComponent getDocument(WXComponent parent){
        while (parent != null){
            if(DOCUMENT_COMPONENT.equals(parent.getComponentType())){
                return (WXDocumentComponent) parent;
            }
            if(WXBasicComponentType.CELL.equals(parent.getComponentType())){
                return null;
            }
            if(WXBasicComponentType.LIST.equals(parent.getComponentType())){
                return null;
            }
            parent = parent.getParent();
        }
        return null;
    }

    @Override
    public void notifyAppearStateChange(String wxEventType, String direction) {
        if(documentHelper != null){
            documentHelper.notifyAppearStateChange(wxEventType, direction);
        }
    }

    @Override
    public void setDemission(GraphicSize size, GraphicPosition position) {
        super.setDemission(size, position);
        if(getHostView() == null){
            if(documentShouldInited){
                new InitDocumentViewAction(this).run();
            }
        }
    }

    @Override
    public void onLoadImage(BitmapTarget imageTarget) {
        if(imageTarget.isPlaceholder()){
            return;
        }
        WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getInstanceId(), imageTarget.getRef());
        if(component == null || !(component instanceof WXImage)){
            return;
        }
        WXImage image = (WXImage) component;
        int width = 0;
        int height =0;
        boolean result = false;
        if(imageTarget.getBitmap() != null){
            width = imageTarget.getBitmap().getWidth();
            height = imageTarget.getBitmap().getHeight();
            result = true;
        }
        image.fireImageOnLoad(width, height, result);
    }

    @Override
    public void onClick(String ref, int x, int y, int width, int height) {
        if(getHostView() == null || isDestoryed()){
            return;
        }
        WXComponent component = WXSDKManager.getInstance().getWXRenderManager().getWXComponent(getInstanceId(), ref);
        if(component == null){
            return;
        }
        int[] location = new int[2];
        getHostView().getLocationOnScreen(location);
        x += location[0];
        y += location[1];
        Map<String, Object> param = WXDataStructureUtil.newHashMapWithExpectedSize(1);
        Map<String, Object> position = WXDataStructureUtil.newHashMapWithExpectedSize(4);
        position.put("x", x);
        position.put("y",y);
        position.put("width", width);
        position.put("height", height);
        param.put(Constants.Name.POSITION, position);
        component.fireEvent(Constants.Event.CLICK, param);
        if(component instanceof WXA){
            ((WXA)(component)).onHostViewClick();
        }
    }

    public DocumentView getDocumentView() {
        return documentView;
    }

    public boolean isDocumentShouldInited() {
        return documentShouldInited;
    }

    public void setDocumentShouldInited(boolean documentShouldInited) {
        this.documentShouldInited = documentShouldInited;
    }
}
