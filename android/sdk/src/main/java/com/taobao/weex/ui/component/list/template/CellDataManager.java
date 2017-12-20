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
package com.taobao.weex.ui.component.list.template;


import android.support.v4.util.ArrayMap;
import android.text.TextUtils;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.utils.WXUtils;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by furture on 2018/1/23.
 * template data manager, manage data and render state
 */
public class CellDataManager {

    private static final String VIRTUAL_COMPONENT_SEPRATOR = "@";

    public static final String SUB_COMPONENT_TEMPLATE_ID = "@templateId";

    public static final String VIRTUAL_COMPONENT_ID = "@virtualComponentId";

    /**
     * template list data, update should vis data manager interface, this field is read only
     * */
    JSONArray listData;

    /**
     * current template list
     * */
    public final WXRecyclerTemplateList templateList;

    /**
     * data render state for position, itemId、dirty、and other info
     * */
    private Map<Integer, CellRenderState> renderStates = new ArrayMap<>();


    /**
     * list virtual component data, rendet state
     * key virtual component id, value is cell render state.
     * */
    private Map<String, CellRenderState> virtualComponentRenderStates;



    /**
     * template list data manager
     * */
    public CellDataManager(WXRecyclerTemplateList templateList) {
        this.templateList = templateList;
    }

    /**
     * get current render state for current position, never return null.
     * */
    public CellRenderState getRenderState(int position){
        CellRenderState renderState = renderStates.get(position);
        if(renderState == null){
            renderState = new CellRenderState();
            renderState.position = position;
            renderStates.put(position, renderState);
        }
        if(position != renderState.position) {
            renderState.position = position;
            renderState.hasPositionChange = true;
        }
        return renderState;
    }

    /**
     * @param  virutalComponentId virutalComponentId
     * @param  data   update    virutalComponent's data
     * update virtual component data
     * */
    public void updateVirtualComponentData(String virutalComponentId, Object data){
        if(virtualComponentRenderStates != null){
            CellRenderState cellRenderState = virtualComponentRenderStates.get(virutalComponentId);
            if(cellRenderState != null){
                cellRenderState.getVirtualComponentDatas().put(virutalComponentId, data);
                cellRenderState.hasVirtualCompoentUpdate = true;
            }else{
                if(WXEnvironment.isApkDebugable()) {
                    throw new IllegalArgumentException("virtualComponentDatas illegal state empty render state" + virutalComponentId);
                }
            }
        }else{
            if(WXEnvironment.isApkDebugable()){
                throw  new IllegalArgumentException("virtualComponentDatas illegal state " + virutalComponentId);
            }
        }
    }

    /**
     * @param  position  current position virtual component
     * @param  virutalComponentId virutalComponentId
     * @param  data   update    virutalComponent's data
     * create virtual component data
     * */
    public void createVirtualComponentData(int position, String virutalComponentId, Object data){
        if(virtualComponentRenderStates == null){
            virtualComponentRenderStates = new HashMap<>(8);
        }
        CellRenderState renderState = renderStates.get(position);
        renderState.getVirtualComponentDatas().put(virutalComponentId, data);
        virtualComponentRenderStates.put(virutalComponentId, renderState);

    }




    /**
     * @param  listData setList data
     * clear render state for current cell, and virtual component data
     * */
    public void setListData(JSONArray listData) {
        if(this.listData != listData) {
            if(this.listData != null){
                if(WXUtils.getBoolean(templateList.getDomObject().getAttrs().get("exitDetach"), true)){
                    for(int i=0; i<this.listData.size(); i++){
                        cleanRenderState(renderStates.remove(i));
                    }
                }
            }
            this.listData = listData;
            renderStates.clear();
            if (virtualComponentRenderStates != null) {
                virtualComponentRenderStates.clear();
            }
        }
    }

    /**
     * @param index insert index
     * @param  data  data object
     * insert data at current index
     * */
    public boolean insertData(int index, Object data){
        listData.add(index, data);
        boolean renderStateChange = false;
        for(int i=listData.size(); i>= index; i--){
            CellRenderState state = renderStates.remove(i);
            if(state != null){
                renderStates.put(i + 1, state);
                renderStateChange = true;
            }
        }
        return renderStateChange;
    }

    /**
     *
     * @param index insert index
     * @param  data  data object
     * insert data at current index
     * */
    public boolean insertRange(int index, JSONArray data){
        listData.addAll(index, data);
        boolean renderStateChange = false;
        for(int i = listData.size()-1; i >= index; i--){
            CellRenderState state = renderStates.remove(i);
            if(state != null){
                renderStates.put(i + 1, state);
                renderStateChange = true;
            }
        }
        return renderStateChange;
    }

    /**
     * @param  data  data object
     * @param index insert index
     * update data, reset new render state,
     * return true if only data changed, false if viewType changed
     * */
    public boolean  updateData(Object data, int index){
        boolean onlyDataChange = TextUtils.equals(templateList.getTemplateKey(index), templateList.getTemplateKey(data));
        listData.set(index, data);
        if(!onlyDataChange){
            //structure changed, reset render state
            cleanRenderState(renderStates.remove(index));
        }else{
            CellRenderState renderState = renderStates.get(index);
            if(renderState != null){
                renderState.hasDataUpdate = true;
            }
        }
        return onlyDataChange;
    }

    /**
     * @param index
     * remove data, and its render state  and  virtual's data
     * */
    public void  removeData(Integer index){
        listData.remove((int)index); //remove by index
        cleanRenderState(renderStates.remove(index));
        int count = listData.size() + 1;
        for(int i=index + 1; i < count; i++){
            CellRenderState state = renderStates.remove(i);
            if(state != null){
                renderStates.put(i-1, state);
            }
        }
    }

    /**
     *  clean render state, if has virtual component, call virtual component detach lifecycle
     * */
    private void  cleanRenderState(CellRenderState renderState){
        if(renderState == null){
            return;
        }
        if(renderState.hasVirtualComponents()){
            Collection<String> virtualComponentIds =  renderState.getVirtualComponentIds().values();
            for(String virtualComponentId : virtualComponentIds){
                if(virtualComponentRenderStates != null){
                    virtualComponentRenderStates.remove(virtualComponentId);
                }
                WXBridgeManager.getInstance().asyncCallJSEventVoidResult(WXBridgeManager.METHD_COMPONENT_HOOK_SYNC,
                        templateList.getInstanceId(),
                        null,
                        virtualComponentId, VirtualComponentLifecycle.LIFECYCLE, VirtualComponentLifecycle.DETACH, null);

            }
        }
    }


    /**
     * create virtualComponentId
     * */
    public static String createVirtualComponentId(String listRef, String viewTreeKey, long itemId){
        return  listRef + VIRTUAL_COMPONENT_SEPRATOR  + viewTreeKey + VIRTUAL_COMPONENT_SEPRATOR + itemId;
    }

    /**
     * get list ref from virtualComponentId
     * */
    public static String getListRef(String virtualComponentId){
        if(virtualComponentId == null){
            return null;
        }
        return virtualComponentId.split(VIRTUAL_COMPONENT_SEPRATOR)[0];
    }
    
}
