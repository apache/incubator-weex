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
package org.apache.weex.ui.component.binding;



import android.os.AsyncTask;

import org.apache.weex.WXEnvironment;
import org.apache.weex.common.Constants;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXVContainer;
import org.apache.weex.ui.component.list.WXCell;
import org.apache.weex.ui.component.list.template.TemplateDom;
import org.apache.weex.ui.component.list.template.TemplateViewHolder;
import org.apache.weex.ui.component.list.template.WXRecyclerTemplateList;
import org.apache.weex.ui.component.list.template.jni.NativeRenderObjectUtils;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.WXUtils;

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
            AsyncTask<Void, Void, Void> asyncTask = new AsynLayoutTask(templateViewHolder, position, component);
            templateViewHolder.asyncTask = asyncTask;
            asyncTask.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR); //serial executor is better
        }else{
            doLayoutOnly(component, templateViewHolder);
            setLayout(component, false);
            if(templateViewHolder.getHolderPosition() >= 0){
                templateViewHolder.getTemplateList().fireEvent("_attach_slot", TemplateDom
                    .findAllComponentRefs(templateViewHolder.getTemplateList().getRef(), position, component));
            }
        }

    }
    /**
     * @param layoutHeight height
     * @param layoutWidth width
     * */
    public static void doLayoutSync(WXCell component, float layoutWidth, float layoutHeight){
        doSafeLayout(component, layoutWidth, layoutHeight);
        setLayout(component, false);
    }

        /**
         * safe layout
         * */
    public static void doLayoutOnly(WXComponent component, TemplateViewHolder holder){
        doSafeLayout(component, holder.getTemplateList().getLayoutWidth(), holder.getTemplateList().getLayoutHeight());
    }

    private static void doSafeLayout(WXComponent component, float layoutWidth, float layoutHeight){
        try{
            long start = System.currentTimeMillis();
            int height = NativeRenderObjectUtils.nativeLayoutRenderObject(component.getRenderObjectPtr(),
                    layoutWidth,
                    layoutHeight);
            if(WXEnvironment.isOpenDebugLog() && WXRecyclerTemplateList.ENABLE_TRACE_LOG) {
                WXLogUtils.d(WXRecyclerTemplateList.TAG, "WXTemplateList doSafeLayout " +
                        component.getAttrs().get(Constants.Name.Recycler.SLOT_TEMPLATE_CASE) + " " + Thread.currentThread().getName() + " doSafeLayout  used " +
                        (System.currentTimeMillis() - start));
            }
            if(!(height > 0)){
                WXLogUtils.e(WXRecyclerTemplateList.TAG, " WXTemplateList doSafeLayout wrong template " +
                        component.getAttrs().get(Constants.Name.Recycler.SLOT_TEMPLATE_CASE)  + " cell height " + height);
            }
        }catch (Exception e){
            if(WXEnvironment.isApkDebugable()){
                WXLogUtils.e(WXRecyclerTemplateList.TAG, e);
            }
        }
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
        if(component.getAttrs().containsKey(TemplateDom.KEY_RESET_ANIMATION)){
            if(WXUtils.getBoolean(component.getAttrs().get(TemplateDom.KEY_RESET_ANIMATION), true).booleanValue()){
                TemplateDom.resetAnimaiton(component.getHostView());
            }
        }
        long ptr = component.getRenderObjectPtr();
        if(NativeRenderObjectUtils.nativeRenderObjectHasNewLayout(ptr)){
            NativeRenderObjectUtils.nativeRenderObjectUpdateComponent(ptr, component);
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
