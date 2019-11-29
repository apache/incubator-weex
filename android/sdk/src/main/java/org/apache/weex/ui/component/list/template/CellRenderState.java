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
package org.apache.weex.ui.component.list.template;

import android.support.v7.widget.RecyclerView;

import org.apache.weex.el.parse.ArrayStack;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by furture on 2018/1/24.
 * render state for one cell, manage it's render state
 */
public class CellRenderState {

    /**
     * dirty for current position, for example virtual component has update
     * */
    boolean hasVirtualCompoentUpdate = false;

    /**
     * has date update
     * */
    boolean hasDataUpdate = false;

    /**
     * position changed
     * */
    boolean hasPositionChange = false;

    /**
     *  may use position, when position changed should be rendered
     * */
    int  position;

    /**
     * unique itemId for current position, generate via key core
     * */
    long itemId = RecyclerView.NO_ID;



    /**
     * virtualCompoentId for cell, key is viewTreeKey, value is virutalComponentId.
     * lazy init,
     * */
    private Map<String, String> virtualComponentIds;
    private Map<String, Object> virtualComponentDatas;


    /**
     * mark once statements has rendered
     * */
    private Map<String, ArrayStack> onceComponentStates;


    public Map<String, String> getVirtualComponentIds() {
        if(virtualComponentIds == null){
            virtualComponentIds = new HashMap<>(8);
        }
        return virtualComponentIds;
    }

    /**
     * return current cell has virtual component
     * */
    public boolean hasVirtualComponents(){
        return virtualComponentIds != null && virtualComponentIds.size() > 0;
    }

    public Map<String, Object> getVirtualComponentDatas() {
        if(virtualComponentDatas == null){
            virtualComponentDatas = new HashMap<>(4);
        }
        return virtualComponentDatas;
    }

    public Map<String, ArrayStack> getOnceComponentStates() {
        if(onceComponentStates == null){
            onceComponentStates = new HashMap<>();
        }
        return onceComponentStates;
    }

    public boolean isDirty() {
        return hasDataUpdate
                || hasVirtualCompoentUpdate
                || hasPositionChange;
    }

    public boolean isHasDataUpdate() {
        return hasDataUpdate;
    }



    public void  resetDirty(){
        hasDataUpdate = false;
        hasVirtualCompoentUpdate = false;
        hasPositionChange = false;
    }


}
