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
package com.taobao.weex.ui.component;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.action.TraceableAction;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Created by furture on 2018/1/25.
 */

public class ComponentUtils {

    private static final String TAG = "ComponentUtils";

    /**
     * init component  if component is lazy,
     * if component is not lazy, do nothing
     * */
    public static void initLazyComponent(WXComponent component, WXVContainer mParent){
        if(component.isLazy()){
            component.lazy(false);
            if(mParent != null){
                int index = mParent.indexOf(component);
                mParent.createChildViewAt(index);
            }else {
                component.createView();
            }
            component.applyLayoutAndEvent(component);
            component.bindData(component);
        }
    }

    /**
     * build component tree and dom tree for template list
     * */
    public static synchronized final WXComponent buildTree(WXDomObject domObject, WXVContainer parent){
        DOMActionContext domActionContext = WXSDKManager.getInstance().getWXDomManager().getDomContext(parent.getInstanceId());
        if(domActionContext == null){
            return null;
        }
        DomTreeBuilder builder = new DomTreeBuilder();
        domObject.traverseTree(
                domActionContext.getAddDOMConsumer(),
                domActionContext.getApplyStyleConsumer()
        );
        return builder.buildComponentTree(domActionContext, domObject, parent);
    }


    /**
     * dom tree build
     * */
    static class DomTreeBuilder extends TraceableAction {

        private WXComponent buildComponentTree(DOMActionContext context, WXDomObject dom, WXVContainer parent) {
            if (dom == null) {
                return null;
            }
            long startNanos = System.nanoTime();
            dom.setCloneThis(true);
            WXComponent component = WXComponentFactory.newInstance(context.getInstance(), dom, parent);
            if (component != null) {
                component.mTraceInfo.domThreadStart = dom.mDomThreadTimestamp;
                component.mTraceInfo.rootEventId = mTracingEventId;
                component.mTraceInfo.domQueueTime = mDomQueueTime;
            }
            context.registerComponent(dom.getRef(), component);
            if (component instanceof WXVContainer) {
                WXVContainer container = (WXVContainer) component;
                WXDomObject parentDom = (WXDomObject) container.getDomObject();
                for (int i = 0; i < dom.childCount(); ++i) {
                    WXDomObject child = dom.getChild(i);
                    if (child != null) {
                        WXComponent childComponent = buildComponentTree(context, child, container);
                        container.addChild(childComponent);
                        WXDomObject childDomObject = (WXDomObject) childComponent.getDomObject();
                        if(childDomObject != child) {
                            int index = parentDom.index(child);
                            parentDom.add(childDomObject, index);
                            if(index >= 0) {
                                parentDom.remove(child);
                                i--;
                            }
                            RuntimeException exception = new IllegalArgumentException(childDomObject.getClass().getName()
                                    + " not support clone this");
                            WXLogUtils.e("weex", exception);
                            if(WXEnvironment.isApkDebugable()){
                                throw  exception;
                            }
                        }
                    }
                }
            }
            dom.setCloneThis(false);
            if (component != null) {
                component.mTraceInfo.domThreadNanos = System.nanoTime() - startNanos;
            }
            return component;
        }
    }
}
