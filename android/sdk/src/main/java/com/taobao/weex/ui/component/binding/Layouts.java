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
package com.taobao.weex.ui.component.binding;



import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.flex.CSSLayoutContext;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;

/**
 * Created by furture on 2017/8/21.
 */
public class Layouts {
    /**
     * do dom layout, and set layout to component
     * */
    public static void doLayout(WXComponent component, CSSLayoutContext layoutContext){
        final WXSDKInstance instance = component.getInstance();
        WXDomObject domObject = (WXDomObject) component.getDomObject();
        domObject.traverseTree(new WXDomObject.Consumer() {
            @Override
            public void accept(WXDomObject dom) {
                if(instance == null || instance.isDestroy()){
                    return;
                }
                if(!dom.hasUpdate()){
                    return;
                }
                dom.layoutBefore();
            }
        });
        domObject.calculateLayout(layoutContext);
        domObject.traverseTree( new WXDomObject.Consumer() {
            @Override
            public void accept(WXDomObject dom) {
                if(instance == null || instance.isDestroy()){
                    return;
                }
                if (dom.hasUpdate()) {
                    dom.layoutAfter();
                }
            }
        });
        setLayout(component, false);
    }


    /**
     * recursive set layout to component,
     * dom extra will also be updated from dom object to component.
     * if force is true, always set layout
     * */
    public static final void setLayout(WXComponent component, boolean force){
        WXDomObject domObject = (WXDomObject) component.getDomObject();
        if(domObject.hasUpdate() || force){
            domObject.markUpdateSeen();
            component.setLayout(component.getDomObject());
            if(component.getDomObject().getExtra() != null){
                component.updateExtra(component.getDomObject().getExtra());
            }
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            int count = container.getChildCount();
            for (int i = 0; i < count; ++i) {
                WXComponent child = container.getChild(i);
                if (child != null) {
                    setLayout(child, force);
                }
            }
        }
    }
}
