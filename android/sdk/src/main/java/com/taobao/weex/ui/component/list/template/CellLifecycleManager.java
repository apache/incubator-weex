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

import android.util.Log;

import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.component.list.WXCell;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * cell-slot lifecycle manager, onCreate onAttach onDetach destroy
 * Created by furture on 2017/9/19.
 */
public class CellLifecycleManager {

    private static final String[] lifecycleEventNames = {
            Constants.Event.SLOT_LIFECYCLE.CREATE,
            Constants.Event.SLOT_LIFECYCLE.ATTACH,
            Constants.Event.SLOT_LIFECYCLE.DETACH,
            Constants.Event.SLOT_LIFECYCLE.DESTORY
    };

    private WXRecyclerTemplateList recyclerTemplateList;
    private Map<String,Map<String,List<String>>> eventSlotWatchRefs;
    private List<String> createEvent; // only call once


    public CellLifecycleManager(WXRecyclerTemplateList recyclerTemplateList) {
        this.recyclerTemplateList = recyclerTemplateList;
        this.eventSlotWatchRefs = new HashMap<>();
        this.createEvent = new ArrayList<>();
    }



    public void updateSlotLifecycleWatch(WXCell cell, WXComponent component){
        WXEvent wxEvent = component.getDomObject().getEvents();
        for(String event : lifecycleEventNames){
            if(wxEvent.contains(event)){
                Map<String,List<String>> slotWatchRefs = eventSlotWatchRefs.get(event);
                if(slotWatchRefs == null){
                    slotWatchRefs = new HashMap<>();
                    eventSlotWatchRefs.put(event, slotWatchRefs);
                }
                List<String> refs =  slotWatchRefs.get(cell.getRef());
                if(refs == null){
                    refs = new ArrayList<>(8);
                    slotWatchRefs.put(cell.getRef(), refs);
                }
                if(!refs.contains(component.getRef())){
                    refs.add(component.getRef());
                }
            }
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            for(int i=0; i<container.getChildCount(); i++){
                WXComponent child = container.getChild(i);
                updateSlotLifecycleWatch(cell, child);
            }
        }
    }

    /**
     * onCreate event
     * */
    public void onCreate(int position){
        String key = recyclerTemplateList.getTemplateKey(position);
        Map<String,List<String>>  slotWatchCreateRefs = eventSlotWatchRefs.get(Constants.Event.SLOT_LIFECYCLE.CREATE);
        if(slotWatchCreateRefs == null
                || slotWatchCreateRefs.get(key) == null
                || slotWatchCreateRefs.get(key).size() == 0){
            return;
        }
        if(createEvent.size() >= position){
            if(createEvent.contains(key + position)){
                return;
            }
        }
        WXCell cell = recyclerTemplateList.getTemplates().get(key);
        if(cell == null){
            return;
        }
        Map<String, Object> params = new HashMap<>(8);
        params.put("position", position);
        cell.fireEvent(Constants.Event.SLOT_LIFECYCLE.CREATE, params);
        createEvent.add(key + position);
    }


    /**
     * onAttach event
     * */
    public void onAttach(int position, WXCell cell){
        if(cell == null || position < 0){
            return;
        }
        Map<String,List<String>> slotWatchAttachRefs = eventSlotWatchRefs.get(Constants.Event.SLOT_LIFECYCLE.ATTACH);
        if(slotWatchAttachRefs == null
                || slotWatchAttachRefs.get(cell.getRef()) == null
                || slotWatchAttachRefs.get(cell.getRef()).size() == 0){
            return;
        }
        Map<String, Object> params = new HashMap<>(8);
        params.put("position", position);
        cell.fireEvent(Constants.Event.SLOT_LIFECYCLE.ATTACH, params);
    }

    /**
     * onDetach event
     * */
    public void onDetach(int position, WXCell cell){
        if(cell == null || position < 0){
            return;
        }
        Map<String,List<String>> slotWatchDetachRefs = eventSlotWatchRefs.get(Constants.Event.SLOT_LIFECYCLE.ATTACH);
        if(slotWatchDetachRefs == null
                || slotWatchDetachRefs.get(cell.getRef()) == null
                || slotWatchDetachRefs.get(cell.getRef()).size() == 0){
            return;
        }
        Map<String, Object> params = new HashMap<>(8);
        params.put("position", position);
        cell.fireEvent(Constants.Event.SLOT_LIFECYCLE.DETACH, params);
    }

    /**
     * onDestory event
     * */
    public void onDestory(){
        Map<String,List<String>> slotWatchDestroyRefs = eventSlotWatchRefs.get(Constants.Event.SLOT_LIFECYCLE.DESTORY);
        if(slotWatchDestroyRefs == null
                || slotWatchDestroyRefs.size() == 0){
            return;
        }
        int size = recyclerTemplateList.getItemCount();
        for(int position=0; position<size; position++){
            String key = recyclerTemplateList.getTemplateKey(position);
            if(slotWatchDestroyRefs.get(key) == null
                    || slotWatchDestroyRefs.get(key).size() == 0){
                return;
            }
            WXCell cell = recyclerTemplateList.getTemplates().get(key);
            if(cell == null){
                return;
            }
            Map<String, Object> params = new HashMap<>(8);
            params.put("position", position);
            cell.fireEvent(Constants.Event.SLOT_LIFECYCLE.DESTORY, params);
        }
        eventSlotWatchRefs.clear();
    }
}
