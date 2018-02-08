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



import android.os.AsyncTask;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.flex.CSSLayoutContext;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.component.list.template.TemplateViewHolder;
import com.taobao.weex.ui.component.list.template.WXRecyclerTemplateList;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Created by furture on 2017/8/21.
 */
public class Layouts {
    /**
     * do dom layout async or sync , and set layout to component on main.
     * on first use do sync layout, when compontnet reuse do async layout
     * */
    public static void doLayoutAsync(final TemplateViewHolder templateViewHolder, boolean async){
        final WXComponent component = templateViewHolder.getComponent();
        final  int position = templateViewHolder.getHolderPosition();
        if(templateViewHolder.asyncTask != null){
            templateViewHolder.asyncTask.cancel(false);
            templateViewHolder.asyncTask = null;
        }
        if(async){
            AsyncTask<Void, Void, Void> asyncTask = new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    if(templateViewHolder.getHolderPosition() == position){
                        if(component.getInstance() != null && !component.getInstance().isDestroy()) {
                            doSafeLayout(component, templateViewHolder.getLayoutContext());
                        }
                    }
                    return null;
                }

                @Override
                protected void onPostExecute(Void aVoid) {
                    if(position == templateViewHolder.getHolderPosition()) {
                        if(component.getInstance() != null && !component.getInstance().isDestroy()) {
                            setLayout(component, false);
                        }
                    }
                }
            };
            templateViewHolder.asyncTask = asyncTask;
            asyncTask.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR); //serial executor is better
        }else{
            doSafeLayout(component, templateViewHolder.getLayoutContext());
            setLayout(component, false);
        }

    }

    /**
     * safe layout
     * */
    public static void doSafeLayout(WXComponent component, final  CSSLayoutContext layoutContext){
        try{
            long start = System.currentTimeMillis();
            doLayout(component, layoutContext);
            if(WXEnvironment.isOpenDebugLog() && WXRecyclerTemplateList.ENABLE_TRACE_LOG) {
                WXLogUtils.d(WXRecyclerTemplateList.TAG, "WXTemplateList doSafeLayout" +
                        component.getDomObject().getAttrs().get(Constants.Name.Recycler.SLOT_TEMPLATE_CASE) + Thread.currentThread().getName() + " doSafeLayout  used " +
                        (System.currentTimeMillis() - start));
            }
            if(!(component.getLayoutHeight() > 0)){
                WXLogUtils.e(WXRecyclerTemplateList.TAG, " WXTemplateList doSafeLayout wrong template " +
                        component.getDomObject().getAttrs().get(Constants.Name.Recycler.SLOT_TEMPLATE_CASE)  + " cell height " + component.getLayoutHeight());
            }
        }catch (Exception e){
            if(WXEnvironment.isApkDebugable()){
                WXLogUtils.e(WXRecyclerTemplateList.TAG, e);
            }
        }
    }

    private static void doLayout(WXComponent component, final  CSSLayoutContext layoutContext){
        WXDomObject domObject = (WXDomObject) component.getDomObject();
        final WXSDKInstance instance = component.getInstance();
        domObject.traverseUpdateTree(new WXDomObject.Consumer() {
            @Override
            public void accept(WXDomObject dom) {
                if(instance == null || instance.isDestroy()){
                    return;
                }
                if(!dom.hasUpdate()){
                    return;
                }
                if(!dom.isShow()){ //not show just skip
                   return;
                }
                dom.layoutBefore();
            }
        });
        if(instance != null && !instance.isDestroy()){
            domObject.calculateLayout(layoutContext);
        }
        domObject.traverseUpdateTree( new WXDomObject.Consumer() {
            @Override
            public void accept(WXDomObject dom) {
                if(instance == null || instance.isDestroy()){
                    return;
                }
                if(!dom.isShow()){
                    return;
                }
                if (dom.hasUpdate()) {
                    dom.layoutAfter();
                }
            }
        });
    }




    /**
     * recursive set layout to component,
     * dom extra will also be updated from dom object to component.
     * if force is true, always set layout
     * */
    public static final void setLayout(WXComponent component, boolean force){
        if(component.isWaste()){
            return;
        }
        WXDomObject domObject = (WXDomObject) component.getDomObject();
        if(domObject.hasUpdate() || force){
            domObject.markUpdateSeen();
            if(domObject.hasUpdate()){
                domObject.markLayoutStateUpdated();
            }
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

    private static final void setLayoutWaste(WXComponent component, boolean force){
        WXDomObject domObject = (WXDomObject) component.getDomObject();
        if(domObject.hasUpdate() || force){
            domObject.markUpdateSeen();
            if(domObject.hasUpdate()){
                domObject.markLayoutStateUpdated();
            }
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            int count = container.getChildCount();
            for (int i = 0; i < count; ++i) {
                WXComponent child = container.getChild(i);
                if (child != null) {
                    setLayoutWaste(child, force);
                }
            }
        }
    }
}
