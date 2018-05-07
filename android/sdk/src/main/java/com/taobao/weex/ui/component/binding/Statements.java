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

import android.os.Looper;
import android.support.v4.util.ArrayMap;
import android.text.TextUtils;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.bridge.EventResult;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXAttr;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.dom.WXStyle;
import com.taobao.weex.dom.binding.ELUtils;
import com.taobao.weex.dom.binding.JSONUtils;
import com.taobao.weex.dom.binding.WXStatement;
import com.taobao.weex.el.parse.ArrayStack;
import com.taobao.weex.el.parse.Operators;
import com.taobao.weex.el.parse.Token;
import com.taobao.weex.ui.action.BasicComponentData;
import com.taobao.weex.ui.action.GraphicPosition;
import com.taobao.weex.ui.action.GraphicSize;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXComponentFactory;
import com.taobao.weex.ui.component.WXImage;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.component.list.WXCell;
import com.taobao.weex.ui.component.list.template.CellDataManager;
import com.taobao.weex.ui.component.list.template.CellRenderContext;
import com.taobao.weex.ui.component.list.template.VirtualComponentLifecycle;
import com.taobao.weex.ui.component.list.template.WXRecyclerTemplateList;
import com.taobao.weex.ui.component.list.template.jni.NativeRenderObjectUtils;
import com.taobao.weex.utils.WXLogUtils;
import com.taobao.weex.utils.WXUtils;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Created by jianbai.gbj on 2017/8/17.
 * simple statement execute, render component for template list
 */
public class Statements {


    /**
     * parse statements token from attrs and style and event
     * */
    public static void  parseStatementsToken(WXComponent component){
        if(component.getBasicComponentData().isRenderPtrEmpty()){
           component.getBasicComponentData().setRenderObjectPr(component.getRenderObjectPtr());
        }
        if(component.getBasicComponentData() != null){
            BasicComponentData basicComponentData =  component.getBasicComponentData();
            basicComponentData.getAttrs().parseStatements();
            basicComponentData.getStyles().parseStatements();
            basicComponentData.getEvents().parseStatements();
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            int count = container.getChildCount();
            for (int i = 0; i < count; ++i) {
                parseStatementsToken(container.getChild(i));
            }
        }
    }



    /**
     * init component  if component is lazy,
     * if component is not lazy, do nothing
     * */
    public static void initLazyComponent(WXComponent component, WXVContainer mParent){
        if(component.isLazy() || component.getHostView() == null){
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
     * recursive copy component, none parent connect
     * */
    public static WXComponent copyComponentTree(WXComponent component){
        WXComponent copy =  copyComponentTree(component, component.getParent());
        return copy;
    }

    /**
     * recursive copy component,
     * */
    private static final WXComponent copyComponentTree(WXComponent source, WXVContainer parent){
        BasicComponentData basicComponentData = null;
        try {
            basicComponentData = source.getBasicComponentData().clone();
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        WXComponent component = WXComponentFactory.newInstance(source.getInstance(), parent, basicComponentData);
        GraphicPosition graphicPosition = source.getLayoutPosition();
        GraphicSize graphicSize = source.getLayoutSize();
        component.updateDemission(graphicPosition.getTop(), graphicPosition.getBottom(), graphicPosition.getLeft(), graphicPosition.getRight(),
                 graphicSize.getHeight(), graphicSize.getWidth());
        component.updateExtra(source.getExtra());
        if(source instanceof WXVContainer){
            WXVContainer container = (WXVContainer) source;
            WXVContainer childParent = (WXVContainer) component;
            int count = container.getChildCount();
            for (int i = 0; i < count; ++i) {
                WXComponent sourceChild = container.getChild(i);
                if (sourceChild != null) {
                    WXComponent targetChild = copyComponentTree(sourceChild,  childParent);childParent.addChild(targetChild);
                    NativeRenderObjectUtils.nativeAddChildRenderObject(childParent.getRenderObjectPtr(),
                            targetChild.getRenderObjectPtr());
                }
            }
        }
        //copy info need be sync
        if(source.isWaste()){
            component.setWaste(true);
        }
        return  component;
    }

    /**
     *  @param component component with v-for statement, v-if statement and bind attrs
     *  @param stack  execute context
     *  render component in context, the function do the following  work.
     *  execute component's v-for statement, v-if statement in context,
     *  and rebuild component's tree with the statement, v-for reuse component execute by pre render.
     *  if executed, component will be removed, don't remove, just mark it waste;
     *  may be next render it can be used.
     *  after statement has executed, render component's binding attrs in context and bind it to component.
     * */
    public static final List<WXComponent> doRender(WXComponent component, CellRenderContext stack){
        List<WXComponent> updates = new ArrayList<>(4);
        try{
            doRenderComponent(component, stack, updates);
        }catch (Exception e){
            WXLogUtils.e("WeexStatementRender", e);
        }
        return updates;
    }

    public static final void doInitCompontent(List<WXComponent> updates) {
        if(updates == null || updates.size() == 0){
            return;
        }
        for(WXComponent renderNode : updates){
            if(renderNode.getParent() == null){
                throw new IllegalArgumentException("render node parent cann't find");
            }
            WXVContainer parent = renderNode.getParent();
            int renderIndex = parent.indexOf(renderNode);
            if(renderIndex < 0){
                throw new IllegalArgumentException("render node cann't find");
            }
            parent.createChildViewAt(renderIndex);
            renderNode.applyLayoutAndEvent(renderNode);
            renderNode.bindData(renderNode);
        }
    }

        /**
         *  @param component component with v-for statement, v-if statement and bind attrs
         *  @param context   execute context
         *  render component in context, the function do the following  work.
         *  execute component's v-for statement, v-if statement in context,
         *  and rebuild component's tree with the statement, v-for reuse component execute by pre render.
         *  if executed, component will be removed, don't remove, just mark it waste;
         *  may be next render it can be used.
         *  after statement has executed, render component's binding attrs in context and bind it to component.
         * */
    private static final int doRenderComponent(WXComponent component, CellRenderContext context,
                                               List<WXComponent> updates){
        WXVContainer parent = component.getParent();
        WXAttr attrs = component.getAttrs();
        WXStatement statement =  attrs.getStatement();
        if(statement != null){
            Token vif =  null;
            JSONObject vfor = null;
            if(statement.get(WXStatement.WX_IF) instanceof Token){
                vif = (Token) statement.get(WXStatement.WX_IF);
            }
            if(statement.get(WXStatement.WX_FOR) instanceof  JSONObject){
                vfor = (JSONObject) statement.get(WXStatement.WX_FOR);
            }
            // execute v-for content
            if(vfor != null){
                int    renderIndex = parent.indexOf(component);
                if(vfor.get(WXStatement.WX_FOR_LIST) instanceof Token){
                    Token listBlock = (Token) vfor.get(WXStatement.WX_FOR_LIST);
                    String indexKey = vfor.getString(WXStatement.WX_FOR_INDEX);
                    String itemKey = vfor.getString(WXStatement.WX_FOR_ITEM);
                    Object data = null;
                    if(listBlock != null) {
                        data = listBlock.execute(context.stack);
                    }
                    if((data instanceof List || data instanceof  Map)){

                        Collection collection = null;
                        Map  map = null;
                        if(data instanceof  List){
                            collection = (List)data;
                        }else{
                            map = (Map)data;
                            collection = map.keySet();
                        }
                        Map<String, Object> loop = new HashMap<>();
                        int index = 0;
                        for(Object item : collection){
                            Object key = null;
                            Object value = item;
                            if(map == null){
                                key = index;
                                value = item;
                                index ++;
                            }else{
                                key = item;
                                value = map.get(item);
                            }
                            if(indexKey != null){
                                loop.put(indexKey, key);
                            }

                            if(itemKey != null){
                                loop.put(itemKey, value);
                            }else{
                                context.stack.push(value);
                            }
                            if(loop.size() > 0){
                                context.stack.push(loop);
                            }


                            if(vif != null){
                                if(!Operators.isTrue(vif.execute(context.stack))){
                                    continue;
                                }
                            }
                            //find resuable renderNode
                            WXComponent renderNode = null;
                            if(renderIndex < parent.getChildCount()){
                                renderNode = parent.getChild(renderIndex);
                                //check is same statment, if true, it is usabled.
                                if(!isCreateFromNodeStatement(renderNode, component)){
                                    renderNode = null;
                                }
                                if(renderNode != null){
                                    if(renderNode.isWaste()){
                                        renderNode.setWaste(false);
                                    }
                                }
                            }
                            //none resuable render node, create node, add to parent, but clear node's statement
                            if(renderNode == null){
                                long start = System.currentTimeMillis();
                                renderNode = copyComponentTree(component, parent);
                                renderNode.setWaste(false);
                                if(renderNode.getAttrs().getStatement() != null) {
                                    renderNode.getAttrs().getStatement().remove(WXStatement.WX_FOR);
                                    renderNode.getAttrs().getStatement().remove(WXStatement.WX_IF); //clear node's statement
                                }
                                parent.addChild(renderNode, renderIndex);
                                NativeRenderObjectUtils.nativeAddChildRenderObject(parent.getRenderObjectPtr(),
                                        renderNode.getRenderObjectPtr());
                                updates.add(renderNode);
                                if(WXEnvironment.isApkDebugable()){
                                    WXLogUtils.d(WXRecyclerTemplateList.TAG, Thread.currentThread().getName() +  renderNode.getRef() + renderNode.getComponentType() + "statements copy component tree used " + (System.currentTimeMillis() - start));
                                }
                            }
                            doBindingAttrsEventAndRenderChildNode(renderNode, context, updates);
                            renderIndex++;
                            if(loop.size() > 0){
                                context.stack.push(loop);
                            }
                            if(itemKey == null) {
                                context.stack.pop();
                            }
                        }
                    }
                }else{
                    WXLogUtils.e(WXRecyclerTemplateList.TAG,  vfor.toJSONString() + " not call vfor block, for pre compile");
                }
                //after v-for execute, remove component created pre v-for.
                for(;renderIndex<parent.getChildCount(); renderIndex++){
                    WXComponent wasteNode = parent.getChild(renderIndex);
                    if(!isCreateFromNodeStatement(wasteNode, component)){
                        break;
                    }
                    wasteNode.setWaste(true);
                }
                return renderIndex - parent.indexOf(component);
            }

            //execute v-if context
            if(vif != null){
                if(!Operators.isTrue(vif.execute(context.stack))){
                    component.setWaste(true);
                    return 1;
                }else{
                    component.setWaste(false);
                }

            }
        }
        doBindingAttrsEventAndRenderChildNode(component, context, updates);
        return  1;
    }


    /**
     * bind attrs and doRender component child
     * */
    private static void doBindingAttrsEventAndRenderChildNode(WXComponent component, CellRenderContext context,
                                                              List<WXComponent> updates){
        WXAttr attr = component.getAttrs();

        /**
         * sub component supported, sub component new stack
         * */
        ArrayStack stack = context.stack;



        if(attr.get(ELUtils.IS_COMPONENT_ROOT) != null
                && WXUtils.getBoolean(attr.get(ELUtils.IS_COMPONENT_ROOT), false)){
            if(attr.get(ELUtils.COMPONENT_PROPS) != null
                    && JSONUtils.isJSON(attr.get(ELUtils.COMPONENT_PROPS))){
                 String compoentId = (String) attr.get(CellDataManager.SUB_COMPONENT_TEMPLATE_ID);
                Object compoentData = null;
                if(!TextUtils.isEmpty(compoentId)){
                    String virtualComponentId =  context.getRenderState().getVirtualComponentIds().get(component.getViewTreeKey());
                   if(virtualComponentId == null){ //none virtualComponentId, create and do attach
                        virtualComponentId =  CellDataManager.createVirtualComponentId(context.templateList.getRef(),
                                component.getViewTreeKey(), context.templateList.getItemId(context.position));
                        Map<String, Object>  props  = renderProps(JSONUtils.toJSON(attr.get(ELUtils.COMPONENT_PROPS)), context.stack);
                        EventResult result = WXBridgeManager.getInstance().syncCallJSEventWithResult(WXBridgeManager.METHD_COMPONENT_HOOK_SYNC, component.getInstanceId(), null, compoentId, VirtualComponentLifecycle.LIFECYCLE, VirtualComponentLifecycle.CREATE,  new Object[]{
                                virtualComponentId,
                                props
                        }, null);
                        if(result != null
                                && result.getResult() != null
                                && result.getResult() instanceof Map){
                            props.putAll((Map<? extends String, ?>) result.getResult());
                        }
                        compoentData  =  props;
                        context.getRenderState().getVirtualComponentIds().put(component.getViewTreeKey(), virtualComponentId);
                        context.templateList.getCellDataManager().createVirtualComponentData(context.position, virtualComponentId, compoentData);
                        //create virtual componentId
                        WXBridgeManager.getInstance().asyncCallJSEventVoidResult(WXBridgeManager.METHD_COMPONENT_HOOK_SYNC, component.getInstanceId(), null, virtualComponentId, VirtualComponentLifecycle.LIFECYCLE, VirtualComponentLifecycle.ATTACH, null);
                     }else{ // get virtual component data check has dirty's update
                       compoentData = context.getRenderState().getVirtualComponentDatas().get(virtualComponentId);
                       if(context.getRenderState().isHasDataUpdate()){
                           Map<String, Object>  props  = renderProps((JSONObject) attr.get(ELUtils.COMPONENT_PROPS), context.stack);
                           EventResult result = WXBridgeManager.getInstance().syncCallJSEventWithResult(WXBridgeManager.METHD_COMPONENT_HOOK_SYNC, component.getInstanceId(), null, virtualComponentId, VirtualComponentLifecycle.LIFECYCLE, VirtualComponentLifecycle.SYNSTATE,  new Object[]{
                                   virtualComponentId,
                                   props
                           }, null);

                           if(result != null
                                   && result.getResult() != null
                                   && result.getResult() instanceof Map){
                               props.putAll((Map<? extends String, ?>) result.getResult());
                               context.templateList.getCellDataManager().updateVirtualComponentData(virtualComponentId, props);
                               compoentData = props;
                           }
                       }
                    }
                    component.getAttrs().put(CellDataManager.VIRTUAL_COMPONENT_ID, virtualComponentId);
                }else{ //stateless component
                    Map<String, Object>  props  = renderProps((JSONObject) attr.get(ELUtils.COMPONENT_PROPS), context.stack);
                    compoentData = props;
                }
                //virtual component is new context
                context.stack = new ArrayStack();
                if(compoentData != null){
                    context.stack.push(compoentData);
                }
            }
        }

        /**
         * check node is render only once, if render once, and has rendered, just return
         * */
        Object vonce = null;
        if(attr.getStatement() != null){
            vonce = attr.getStatement().get(WXStatement.WX_ONCE);
        }
        if(vonce != null){
            ArrayStack onceStack = context.getRenderState().getOnceComponentStates().get(component.getViewTreeKey());
            if(onceStack == null){
                onceStack = context.templateList.copyStack(context, stack);
                context.getRenderState().getOnceComponentStates().put(component.getViewTreeKey(), onceStack);
            }
            context.stack = onceStack;
        }

        doRenderBindingAttrsAndEvent(component, context);
        if(component instanceof WXVContainer){
            if(component.isWaste()){
                if(!(component instanceof WXCell)){
                     return;
                }
            }
            WXVContainer container = (WXVContainer) component;
            for(int k=0; k<container.getChildCount();){
                WXComponent next = container.getChild(k);
                k += doRenderComponent(next, context, updates);
            }
        }
        if(stack != context.stack){
            context.stack = stack;
        }
    }


    /**
     * check whether render node is create from component node statement.
     * */
    private static boolean isCreateFromNodeStatement(WXComponent renderNode, WXComponent component){
        return (renderNode.getRef() != null && renderNode.getRef().equals(component.getRef()));
    }


    /**
     * render dynamic binding attrs and bind them to component node.
     * */
    private static void doRenderBindingAttrsAndEvent(WXComponent component, CellRenderContext context){
        ArrayStack stack  = context.stack;
        component.setWaste(false);
        WXAttr attr = component.getAttrs();
        if(attr != null
                && attr.getBindingAttrs() != null
                && attr.getBindingAttrs().size() > 0){
            ArrayMap<String, Object> bindAttrs = component.getAttrs().getBindingAttrs();
            Map<String, Object> dynamic =  renderBindingAttrs(bindAttrs, stack);
            Set<Map.Entry<String, Object>> entries = dynamic.entrySet();
            /**
             * diff attrs, see attrs has update, remove none update attrs
             * */
            Iterator<Map.Entry<String, Object>> iterator = entries.iterator();
            while (iterator.hasNext()){
                Map.Entry<String, Object> entry = iterator.next();
                String key = entry.getKey();
                Object value = entry.getValue();
                Object oldValue = attr.get(key);
                if(value == null){
                    if(oldValue == null){
                        iterator.remove();
                        continue;
                    }
                }else{
                    if(value.equals(oldValue)){
                        iterator.remove();
                    }
                }
            }

            if(dynamic.size() > 0) {
                if(dynamic.size() == 1
                        && dynamic.get(Constants.Name.SRC) != null
                        && component instanceof WXImage){
                    //for image avoid dirty layout, only update src attrs
                    component.getAttrs().put(Constants.Name.SRC, dynamic.get(Constants.Name.SRC));
                }else {
                    component.nativeUpdateAttrs(dynamic);
                }
                if(isMainThread()) {
                    component.updateProperties(dynamic);
                }
                dynamic.clear();
            }
        }


        WXStyle style = component.getStyles();
        if(style != null && style.getBindingStyle() != null){
            ArrayMap<String, Object> bindStyle = style.getBindingStyle();
            Map<String, Object> dynamic =  renderBindingAttrs(bindStyle, stack);
            Set<Map.Entry<String, Object>> entries = dynamic.entrySet();
            /**
             * diff attrs, see attrs has update, remove none update attrs
             * */
            Iterator<Map.Entry<String, Object>> iterator = entries.iterator();
            while (iterator.hasNext()){
                Map.Entry<String, Object> entry = iterator.next();
                String key = entry.getKey();
                Object value = entry.getValue();
                Object oldValue = style.get(key);
                if(value == null){
                    if(oldValue == null){
                        iterator.remove();
                        continue;
                    }
                }else{
                    if(value.equals(oldValue)){
                        iterator.remove();
                    }
                }
            }
            if(dynamic.size() > 0) {
                  component.updateNativeStyles(dynamic);
                 if(isMainThread()) {
                    component.updateProperties(dynamic);
                 }
            }
        }

        WXEvent event = component.getEvents();
        if(event == null || event.getEventBindingArgs() == null){
            return;
        }
        Set<Map.Entry<String, Object>> eventBindArgsEntrySet = event.getEventBindingArgs().entrySet();
        for(Map.Entry<String, Object> eventBindArgsEntry : eventBindArgsEntrySet){
             List<Object> values = getBindingEventArgs(stack, eventBindArgsEntry.getValue());
             if(values != null){
                 event.putEventBindingArgsValue(eventBindArgsEntry.getKey(), values);
             }
        }

    }


    /**
     * @param  bindAttrs  none null,
     * @param  context  context
     * return binding attrs rended value in context
     * */
    private static final  ThreadLocal<Map<String, Object>> dynamicLocal = new ThreadLocal<>();
    public static Map<String, Object> renderBindingAttrs(ArrayMap bindAttrs, ArrayStack stack){
        Set<Map.Entry<String, Object>> entrySet = bindAttrs.entrySet();
        Map<String, Object> dynamic = dynamicLocal.get();
        if(dynamic == null) {
            dynamic = new HashMap<>();
            dynamicLocal.set(dynamic);
        }
        if(dynamic.size() > 0){
            dynamic.clear();
        }
        for(Map.Entry<String, Object> entry : entrySet){
            Object value = entry.getValue();
            String key = entry.getKey();
            if(value instanceof  JSONObject
                    && (((JSONObject) value).get(ELUtils.BINDING)  instanceof Token)){
                JSONObject binding = (JSONObject) value;
                Token block = (Token) (binding.get(ELUtils.BINDING));
                Object blockValue = block.execute(stack);
                dynamic.put(key, blockValue);
            }else if(value instanceof JSONArray){
                JSONArray array = (JSONArray) value;
                StringBuilder builder = new StringBuilder();
                for(int i=0; i<array.size(); i++){
                    Object element = array.get(i);
                    if(element instanceof  CharSequence){
                        builder.append(element);
                        continue;
                    }
                    if(element instanceof JSONObject
                            && (((JSONObject) element).get(ELUtils.BINDING) instanceof Token)){
                        JSONObject binding = (JSONObject) element;
                        Token block = (Token) (binding.get(ELUtils.BINDING));
                        Object blockValue = block.execute(stack);
                        if(blockValue == null){
                            blockValue = "";
                        }
                        builder.append(blockValue);
                    }
                }
                String builderString = builder.toString();
                if(builderString.length() > 256){
                    if(WXEnvironment.isApkDebugable()){
                        WXLogUtils.w(WXRecyclerTemplateList.TAG, " warn too big string " + builderString);
                    }
                }
                dynamic.put(key, builderString);
            }
        }
        return  dynamic;
    }


    public static Map<String, Object> renderProps(JSONObject props, ArrayStack stack){
        Set<Map.Entry<String, Object>> entrySet = props.entrySet();
        Map<String, Object> renderProps = new ArrayMap<>(4);
        for(Map.Entry<String, Object> entry : entrySet){
            Object value = entry.getValue();
            String key = entry.getKey();
            if(value instanceof  JSONObject
                    && (((JSONObject) value).get(ELUtils.BINDING)  instanceof Token)){
                JSONObject binding = (JSONObject) value;
                Token block = (Token) (binding.get(ELUtils.BINDING));
                Object blockValue = block.execute(stack);
                renderProps.put(key, blockValue);
            }else{
                renderProps.put(key, value);
            }
        }
        return  renderProps;
    }

    public static List<Object> getBindingEventArgs(ArrayStack stack, Object bindings){
          List<Object>  params = new ArrayList<>(4);
          if(bindings instanceof  JSONArray){
              JSONArray array = (JSONArray) bindings;
              for(int i=0; i<array.size(); i++){
                  Object value = array.get(i);
                  if(value instanceof  JSONObject
                          && (((JSONObject) value).get(ELUtils.BINDING) instanceof Token)){
                      Token block = (Token)(((JSONObject) value).get(ELUtils.BINDING));
                      Object blockValue = block.execute(stack);
                      params.add(blockValue);
                  }else{
                      params.add(value);
                  }
              }
          }else if(bindings instanceof  JSONObject){
              JSONObject binding = (JSONObject) bindings;
               if(binding.get(ELUtils.BINDING) instanceof Token){
                   Token block = (Token) binding.get(ELUtils.BINDING);
                   Object blockValue = block.execute(stack);
                   params.add(blockValue);
               }else{
                   params.add(bindings.toString());
               }
          }else{
              params.add(bindings.toString());
          }
          return  params;
    }


    private static boolean isMainThread(){
        return  Thread.currentThread() == Looper.getMainLooper().getThread();
    }



    public static String getComponentId(WXComponent component){
        if(component instanceof WXCell || component == null){
            return  null;
        }
        WXAttr attr = component.getAttrs();
        if(attr.get(ELUtils.IS_COMPONENT_ROOT) != null
                && WXUtils.getBoolean(attr.get(ELUtils.IS_COMPONENT_ROOT), false)){
            if(attr.get(ELUtils.COMPONENT_PROPS) != null
                    && attr.get(ELUtils.COMPONENT_PROPS) instanceof  JSONObject){
                Object componentId = attr.get(CellDataManager.VIRTUAL_COMPONENT_ID);
                if(componentId == null){
                    return null;
                }
                return componentId.toString();
            }
        }
        return getComponentId(component.getParent());
    }



}
