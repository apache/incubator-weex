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

import com.taobao.weex.common.Constants;
import com.taobao.weex.dom.WXEvent;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXVContainer;
import com.taobao.weex.ui.component.list.WXCell;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * cell-slot lifecycle manager
 * Created by furture on 2017/9/19.
 */
public class LifecycleManager {

    private static final String[] eventNames = {
            Constants.Event.Recycler.CREATE,
            Constants.Event.Recycler.ATTACH,
            Constants.Event.Recycler.DETACH,
            Constants.Event.Recycler.DESTORY
    };


    private WXRecyclerTemplateList recyclerTemplateList;
    private Map<String,Map<String,List<String>>> eventSlotWatchRefs;


    private Map<String,List<String>> slotWatchCreateRefs;
    private Map<String,List<String>> slotWatchAttachRefs;
    private Map<String,List<String>> slotWatchDetachRefs;
    private Map<String,List<String>> slotWatchDestroyRefs;
    private List<String> createEvent;
    private Map<String, Object> params;
    public LifecycleManager(WXRecyclerTemplateList recyclerTemplateList) {
        this.recyclerTemplateList = recyclerTemplateList;
        this.eventSlotWatchRefs = new HashMap<>();
        this.slotWatchCreateRefs = new ArrayMap<>();
        this.slotWatchAttachRefs = new ArrayMap<>();
        this.slotWatchDetachRefs = new ArrayMap<>();
        this.slotWatchDestroyRefs = new ArrayMap<>();
        this.createEvent = new ArrayList<>();
        this.params = new HashMap<>();
    }



    public void updateListData(){
        Map<String, WXCell>  templates =  recyclerTemplateList.getTemplates();
        Set<Map.Entry<String, WXCell>> entries = templates.entrySet();
        for(Map.Entry<String, WXCell> entry : entries){
            entry.getValue();
        }
    }

    public void updateSlotLifecycle(WXCell cell, WXComponent component){
        WXEvent wxEvent = component.getDomObject().getEvents();
        for(String event : eventNames){
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
                if(refs.contains(component.getRef())){
                    refs.add(component.getRef());
                }
            }
        }
        if(component instanceof WXVContainer){
            WXVContainer container = (WXVContainer) component;
            for(int k=0; k<container.getChildCount();){
                WXComponent next = container.getChild(k);
                updateSlotLifecycle(cell, next);
            }
        }
    }

    /**
     * create event
     * */
    public void create(int position){
        String key = recyclerTemplateList.getTemplateKey(position);
        if(slotWatchCreateRefs.get(key) == null
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
        params.clear();
        params.put("position", position);
        cell.fireEvent(Constants.Event.Recycler.CREATE, params);
        createEvent.add(key + position);
    }


    /**
     * attach event
     * */
    public void attach(int position, WXCell cell){
        if(cell == null || position < 0){
            return;
        }
        if(slotWatchAttachRefs.get(cell.getRef()) == null
                || slotWatchAttachRefs.get(cell.getRef()).size() == 0){
            return;
        }
        params.clear();
        params.put("position", position);
        cell.fireEvent(Constants.Event.Recycler.ATTACH, params);
    }

    /**
     * detach event
     * */
    public void detach(int position, WXCell cell){
        if(cell == null || position < 0){
            return;
        }
        if(slotWatchDetachRefs.get(cell.getRef()) == null
                || slotWatchDetachRefs.get(cell.getRef()).size() == 0){
            return;
        }
        params.clear();
        params.put("position", position);
        cell.fireEvent(Constants.Event.Recycler.DETACH, params);
    }

    /**
     * destory event
     * */
    public void destory(){
        if(slotWatchDestroyRefs.size() == 0){
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
            params.clear();
            params.put("position", position);
            cell.fireEvent(Constants.Event.Recycler.DESTORY, params);
        }
    }
}
