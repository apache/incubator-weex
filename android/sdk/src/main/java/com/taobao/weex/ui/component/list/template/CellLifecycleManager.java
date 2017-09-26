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


    private Map<Integer,Boolean> firedCreateEvent; // only call once


    public CellLifecycleManager(WXRecyclerTemplateList recyclerTemplateList) {
        this.recyclerTemplateList = recyclerTemplateList;
        this.eventSlotWatchRefs = new HashMap<>();
        this.firedCreateEvent = new HashMap<>();
    }



    public void filterLifecycleWatchEvent(WXCell cell, WXComponent component){
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
                filterLifecycleWatchEvent(cell, child);
            }
        }
    }

    /**
     * onCreate event
     * */
    public void onCreate(int position){
        WXCell  cell = recyclerTemplateList.getSourceTemplate(position);
        if(cell == null){
            return;
        }
        if(firedCreateEvent.get(position) != null){
            return;
        }
        firedCreateEvent.put(position, true);
        Map<String,List<String>>  slotWatchCreateRefs = eventSlotWatchRefs.get(Constants.Event.SLOT_LIFECYCLE.CREATE);
        if(slotWatchCreateRefs == null
                || slotWatchCreateRefs.get(cell.getRef()) == null
                || slotWatchCreateRefs.get(cell.getRef()).size() == 0){
            return;
        }
        List<String> refs = slotWatchCreateRefs.get(cell.getRef());
        if(refs == null || refs.size() == 0){
            return;
        }
        fireChildEvent(Constants.Event.SLOT_LIFECYCLE.CREATE, cell, refs, position);
    }

    public void onInsert(int position){
        WXCell  cell = recyclerTemplateList.getSourceTemplate(position);
        if(cell == null){
            return;
        }
        firedCreateEvent.put(position, true);
        firedCreateEvent.put(firedCreateEvent.size(), true);
        Map<String,List<String>>  slotWatchCreateRefs = eventSlotWatchRefs.get(Constants.Event.SLOT_LIFECYCLE.CREATE);
        if(slotWatchCreateRefs == null
                || slotWatchCreateRefs.get(cell.getRef()) == null
                || slotWatchCreateRefs.get(cell.getRef()).size() == 0){
            return;
        }
        List<String> refs = slotWatchCreateRefs.get(cell.getRef());
        if(refs == null || refs.size() == 0){
            return;
        }
        fireChildEvent(Constants.Event.SLOT_LIFECYCLE.CREATE, cell, refs, position);
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
        List<String> refs = slotWatchAttachRefs.get(cell.getRef());
        fireChildEvent(Constants.Event.SLOT_LIFECYCLE.ATTACH, cell, refs, position);
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
        List<String> refs = slotWatchDetachRefs.get(cell.getRef());
        fireChildEvent(Constants.Event.SLOT_LIFECYCLE.DETACH, cell, refs, position);
    }

    /**
     * onDestory event
     * */
    public void onDestory(int position){
        Boolean hasCreated = firedCreateEvent.remove(position);
        if(hasCreated == null){
            return;
        }
        Map<String,List<String>> slotWatchDestroyRefs = eventSlotWatchRefs.get(Constants.Event.SLOT_LIFECYCLE.DESTORY);
        if(slotWatchDestroyRefs == null
                || slotWatchDestroyRefs.size() == 0){
            return;
        }
        WXCell cell = recyclerTemplateList.getSourceTemplate(position);
        if(cell == null){
            return;
        }
        List<String> refs = slotWatchDestroyRefs.get(cell.getRef());
        if(refs == null || refs.size() == 0){
            return;
        }
        fireChildEvent(Constants.Event.SLOT_LIFECYCLE.DESTORY, cell, refs, position);
    }

    public Map<Integer, Boolean> getFiredCreateEvent() {
        return firedCreateEvent;
    }

    private final  void  fireChildEvent(String event, WXCell cell, List<String> refs, int position){
        if(refs == null || refs.size() == 0){
            return;
        }
        for(String ref : refs){
            List<WXComponent> components = recyclerTemplateList.findChildListByRef(cell, ref);
            if(components == null || components.size() == 0){
                continue;
            }
            for(WXComponent component : components) {
                Map<String, Object> params = new HashMap<>(8);
                params.put("index", position);
                component.fireEvent(event, params);
            }
        }
    }
}
