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

package com.taobao.weex.ui.component.node;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;
import android.widget.ScrollView;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.common.WXRenderStrategy;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.dom.transition.WXTransition;
import com.taobao.weex.tracing.Stopwatch;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.ui.component.Scrollable;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXScroller;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.prerenderaction.PrerenderGraphicActionTransformNode;
import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.Map;

/**
 * Created by luciolong on 14/07/2018.
 */
public class WXComponentNode implements Runnable {
    private static final String TAG = "WXComponentNode";

    private static final int EXECUTE_STATE_CREATE_BODY = 1;
    private static final int EXECUTE_STATE_ADD_ELEMENT = 2;

    @NonNull
    private BasicComponentData mComponentData;

    // parent node
    @Nullable
    private WXComponentNode mParentNode;

    // child node
    @NonNull
    private final ArrayList<WXComponentNode> mChildNodes = new ArrayList<>();

    @NonNull
    private WXSDKInstance mWxInstance;

    @Nullable
    public WXComponent data;

    private int mIndex = 0;

    private GraphicPosition mLayoutPosition;
    private GraphicSize mLayoutSize;
    private boolean mIsLayoutRTL;
    private JSONObject mScrollOptions;

    private boolean mIsRenderSuccuess = false;

    private int mExecuteState = EXECUTE_STATE_ADD_ELEMENT;

    WXComponentNode(@NonNull BasicComponentData data,
                    @NonNull Map<String, WXComponentNode> map,
                    @NonNull WXSDKInstance instance) {
        this.mComponentData = data;
        this.mWxInstance = instance;
        map.put(data.mRef, this);
        if (!TextUtils.isEmpty(data.mParentRef)) {
            this.mParentNode = map.get(data.mParentRef);
        }
    }

    public void setIndex(int index) {
        this.mIndex = index;
    }

    public void setLayoutPosition(GraphicPosition layoutPosition) {
        this.mLayoutPosition = layoutPosition;
    }

    public void setLayoutSize(GraphicSize layoutSize) {
        this.mLayoutSize = layoutSize;
    }

    public void setIsLayoutRTL(boolean isRTL) {
        this.mIsLayoutRTL = isRTL;
    }

    public void startTransform() {
        postTransformAction();
        if (mChildNodes.size() > 0) {
            int i = 0;
            for (WXComponentNode node : mChildNodes) {
                node.startTransform();
                node.setIndex(i);
                i++;
            }
        }
        mIsRenderSuccuess = true;
    }

    private void postTransformAction() {
        WXBridgeManager.getInstance().post(this);
    }

    public void transformNode() {
        switch (mExecuteState) {
            case EXECUTE_STATE_CREATE_BODY:
                createBody();
                mWxInstance.firstScreenCreateInstanceTime(System.currentTimeMillis());
                onCreateFinish();
                break;
            case EXECUTE_STATE_ADD_ELEMENT:
                addElement();
                if (mIsRenderSuccuess && mWxInstance.isRenderSuccess().compareAndSet(false, true)) {
                   onRenderSuccess();
                }
                break;
            default:
                break;
        }
    }

    public void createComponent() {
        if (mWxInstance.getNeedInterceptRender()) {
            return;
        }

        if (data == null) {
            WXLogUtils.d(TAG, "node real create component, type:" + mComponentData.mComponentType);
            long createComponentStart = System.currentTimeMillis();
            String pageId = mWxInstance.getInstanceId();
            WXVContainer parent = null;
            if (mParentNode != null) {
                parent = (WXVContainer) WXSDKManager.getInstance().getWXRenderManager()
                        .getWXComponent(pageId, mParentNode.mComponentData.mRef);
            }

            WXComponent component = WXComponentFactory.newInstance(mWxInstance, parent, mComponentData);
            WXSDKManager.getInstance().getWXRenderManager().registerComponent(pageId, mComponentData.mRef, component);
            WXStyle style = mComponentData.getStyles();
            if (style.containsKey(Constants.Name.TRANSFORM) && component.getTransition() == null) {
                Map<String, Object> animationMap = new ArrayMap<>(2);
                animationMap.put(Constants.Name.TRANSFORM, style.get(Constants.Name.TRANSFORM));
                animationMap
                        .put(Constants.Name.TRANSFORM_ORIGIN, style.get(Constants.Name.TRANSFORM_ORIGIN));
                component.addAnimationForElement(animationMap);
            }
            mWxInstance.onComponentCreate(component, System.currentTimeMillis() - createComponentStart);
            component.setTransition(WXTransition.fromMap(component.getStyles(), component));

            if (null != parent && parent.isIgnoreInteraction){
                component.isIgnoreInteraction = true;
            }
            if (!component.isIgnoreInteraction ){
                Object flag = component.getAttrs().get("ignoreInteraction");
                if ("1".equals(flag) || "true".equals(flag) || component.isFixed()){
                    component.isIgnoreInteraction = true;
                }
            }
            data = component;
        }
    }

    public void createBody() {
        mWxInstance.specifiedRootNode(this);
        if (mWxInstance.getNeedInterceptRender()) {
            mExecuteState = EXECUTE_STATE_CREATE_BODY;
            mWxInstance.updateRootNode();
            return;
        }
        try {
            ensureDataNotNull();
            if (data != null) {
                WXLogUtils.d(TAG, "node real create body");
                data.mIsAddElementToTree = true;
                data.createView();
                data.applyLayoutAndEvent(data);
                data.bindData(data);

                if (data instanceof WXScroller) {
                    WXScroller scroller = (WXScroller) data;
                    if (scroller.getInnerView() instanceof ScrollView) {
                        mWxInstance.setRootScrollView((ScrollView) scroller.getInnerView());
                    }
                }

                mWxInstance.onRootCreated(data);

                if (mWxInstance.getRenderStrategy() != WXRenderStrategy.APPEND_ONCE) {
                    mWxInstance.onCreateFinish();
                }
            }

        } catch (Exception e) {
            WXLogUtils.e("create body failed.", e);
        }
    }

    public void addElement() {
        if (mParentNode == null) {
            return;
        }
        mParentNode.addNode(this, mIndex);
        if (mWxInstance.getNeedInterceptRender()) {
            mExecuteState = EXECUTE_STATE_ADD_ELEMENT;
            return;
        }

        try {
            ensureDataNotNull();
            if (data != null) {
                if (!TextUtils.equals(mComponentData.mComponentType, "video") && !TextUtils.equals(mComponentData.mComponentType, "videoplus"))
                    data.mIsAddElementToTree = true;
                mParentNode.addChild(this, mIndex);
                mParentNode.createChildViewAt(mIndex);

                data.setIsLayoutRTL(mIsLayoutRTL);
                if (mLayoutPosition != null && mLayoutSize != null) {
                    data.setDemission(mLayoutSize, mLayoutPosition);
                }

                data.applyLayoutAndEvent(data);

                data.bindData(data);
                scrollToElementInternal();
            }

        } catch (Exception e) {
            WXLogUtils.e("add component failed.", e);
        }
    }

    private void addElementInternal(WXComponentNode childNode, int index) {
        if (childNode.mParentNode != null) {
            childNode.mParentNode.removeElementInternal(childNode, false);
        }
        childNode.mIndex = index;
        addNode(childNode, index);
        if (!mWxInstance.getNeedInterceptRender()) {
            ensureDataNotNull();
            childNode.ensureDataNotNull();
            if (data instanceof WXVContainer && childNode.data != null) {
                ((WXVContainer) data).addChild(childNode.data, index);
                if (!data.isVirtualComponent()) {
                    ((WXVContainer) data).addSubView(childNode.data.getHostView(), mIndex);
                }
            }
        }
    }

    private void addNode(@NonNull WXComponentNode node, int index) {
        int count = mChildNodes.size();
        index = index >= count ? -1 : index;
        if (index == -1) {
            mChildNodes.add(node);
        } else {
            mChildNodes.add(index, node);
        }
    }

    private void addChild(@NonNull WXComponentNode childNode, int index) {
        ensureDataNotNull();
        childNode.ensureDataNotNull();

        WXVContainer curContainer = (WXVContainer) WXSDKManager.getInstance().getWXRenderManager()
                .getWXComponent(mWxInstance.getInstanceId(), mComponentData.mRef);
        if (curContainer != null) {
            curContainer.addChild(childNode.data, index);
        }
    }

    private void createChildViewAt(int index) {
        ensureDataNotNull();

        WXVContainer curContainer = (WXVContainer) WXSDKManager.getInstance().getWXRenderManager()
                .getWXComponent(mWxInstance.getInstanceId(), mComponentData.mRef);
        if (curContainer != null) {
            curContainer.createChildViewAt(index);
        }
    }

    private void ensureDataNotNull() {
        if (data == null) {
            createComponent();
        }
    }

    public void addEvent(String event) {
        if (mWxInstance.getNeedInterceptRender()) {
            mComponentData.addEvent(event);
            return;
        }
        ensureDataNotNull();
        if (data != null) {
            Stopwatch.tick();
            if (!data.getEvents().contains(event)) {
                data.getEvents().addEvent(event);
            }
            data.addEvent(event);
            Stopwatch.split("addEventToComponent");
        }
    }

    public void invokeMethod(String method, JSONArray args) {
        if (mWxInstance.getNeedInterceptRender()) {
            return;
        }

        ensureDataNotNull();
        if(data == null){
            WXLogUtils.e(WXLogUtils.WEEX_TAG, "invoke method, target component not found.");
            return;
        }
        data.invoke(method, args);
    }

    public void updateLayout(GraphicPosition layoutPosition, GraphicSize layoutSize, boolean isLayoutRTL) {
        mLayoutSize = layoutSize;
        mLayoutPosition = layoutPosition;
        mIsLayoutRTL = isLayoutRTL;
        if (mWxInstance.getNeedInterceptRender()) {
            return;
        }

        ensureDataNotNull();
        if (data == null) {
            WXLogUtils.e(WXLogUtils.WEEX_TAG, "update layout, target component not found.");
            return;
        }

        data.setIsLayoutRTL(mIsLayoutRTL);
        data.setDemission(mLayoutSize, mLayoutPosition);
        data.setSafeLayout(data);
        data.setPadding(data.getPadding(), data.getBorder());
    }

    public void moveElement(String parentRef, int index) {
        WXComponentNode oldParent = mParentNode;
        WXComponentNode newParent = WXSDKManager.getInstance().getWXRenderManager().getWXComponentNode(mWxInstance.getInstanceId(), parentRef);

        if (oldParent == null || newParent == null) {
            WXLogUtils.e(WXLogUtils.WEEX_TAG, "move element, find parent error");
            return;
        }

        oldParent.removeElementInternal(this, false);
        newParent.addElementInternal(this, index);
    }

    public void removeElement() {
        if (!mWxInstance.getNeedInterceptRender() && data != null) {
            clearRegistryForComponent(data);
        }
        if (mParentNode != null && data != null) {
            mParentNode.removeElementInternal(this, true);
        }
    }

    private void removeElementInternal(@NonNull WXComponentNode node, boolean destroy) {
        if (this.mChildNodes.contains(node)) {
            this.mChildNodes.remove(node);
        }
        node.mParentNode = null;

        if (!mWxInstance.getNeedInterceptRender() && data instanceof WXVContainer && node.data != null) {
            ((WXVContainer)data).remove(node.data, destroy);
        }
    }

    private void clearRegistryForComponent(WXComponent component) {
        WXComponent removedComponent = WXSDKManager.getInstance().getWXRenderManager().unregisterComponent(mWxInstance.getInstanceId(), mComponentData.mRef);
        if (removedComponent != null) {
            removedComponent.removeAllEvent();
            removedComponent.removeStickyStyle();
        }
        if (component instanceof WXVContainer) {
            WXVContainer container = (WXVContainer) component;
            int count = container.childCount();
            for (int i = count - 1; i >= 0; --i) {
                clearRegistryForComponent(container.getChild(i));
            }
        }
    }

    public void removeEvent(String event) {
        if (mWxInstance.getNeedInterceptRender()) {
            mComponentData.removeEvent(event);
            return;
        }
        ensureDataNotNull();
        if (data != null) {
            Stopwatch.tick();
            data.removeEvent(event);
            Stopwatch.split("removeEventFromComponent");
        }
    }

    public void scrollToElement(JSONObject scrollOptions) {
        mScrollOptions = scrollOptions;
        if (!mWxInstance.getNeedInterceptRender()) {
            scrollToElementInternal();
        }
    }

    private void scrollToElementInternal() {
        ensureDataNotNull();
        if (data != null && mScrollOptions != null) {
            Scrollable scroller = data.getParentScroller();
            if (scroller == null) {
                return;
            }
            scroller.scrollTo(data, mScrollOptions);
            mScrollOptions = null;
        }
    }

    public void addAttrs(Map<String, String> attrs) {
        if (attrs != null) {
            mComponentData.addAttr(attrs);

            if (!mWxInstance.getNeedInterceptRender()) {
                ensureDataNotNull();
                if (data != null) {
                    data.addAttr(attrs);
                }
            }
        }
    }

    public void updateAttrs(Map<String, String> attrs) {
        if (attrs != null) {
            mComponentData.getAttrs().mergeAttr();
            if (!mWxInstance.getNeedInterceptRender()) {
                ensureDataNotNull();
                if (data != null) {
                    data.updateAttrs(attrs);
                }
            }
        }
    }

    @NonNull
    public BasicComponentData getComponentData() {
        return mComponentData;
    }

    @NonNull
    public WXSDKInstance getWxInstance() {
        return mWxInstance;
    }

    @Override
    public void run() {
        new PrerenderGraphicActionTransformNode(this, mWxInstance, mComponentData.mRef).executeActionOnRender();
    }

    public void onCreateFinish() {
        if (mWxInstance.getNeedInterceptRender()) {
            return;
        }

        mWxInstance.mHasCreateFinish = true;

        if (mWxInstance.getRenderStrategy() == WXRenderStrategy.APPEND_ONCE) {
            mWxInstance.onCreateFinish();
        }

        if (null != mWxInstance.getWXPerformance()){
            mWxInstance.getWXPerformance().callCreateFinishTime = System.currentTimeMillis() - mWxInstance.getWXPerformance().renderTimeOrigin;
        }
        mWxInstance.onOldFsRenderTimeLogic();
    }

    public void onRenderSuccess() {
        if (mWxInstance.getNeedInterceptRender()) {
            return;
        }

        ensureDataNotNull();

        int layoutWidth = 0;
        int layoutHeight = 0;
        if (null != data) {
            layoutWidth = (int) data.getLayoutWidth();
            layoutHeight = (int) data.getLayoutHeight();
        }
        mWxInstance.onRenderSuccess(layoutWidth, layoutHeight);
    }

}
