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

import org.apache.weex.el.parse.ArrayStack;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by furture on 2018/1/23.
 * render context for cell template
 */
public class CellRenderContext {
    /**
     * code execute stack
     * */
    public ArrayStack stack = new ArrayStack();
    /**
     * init context data
     * */
    public Map map = new HashMap(8);


    /**
     * component data context
     * */
    public CellRenderState renderState;

    /**
     * current position
     * */
    public int position;

    /**
     * current list component
     * */
    public WXRecyclerTemplateList templateList;


    /**
     * get current render state
     * */
    public CellRenderState getRenderState() {
        if(renderState != null) {
            renderState =  templateList.getCellDataManager().getRenderState(position);
        }
        return renderState;
    }


    public void clear(){
        if(stack.getList().size() > 0) {
            stack.getList().clear();
        }
        if(map.size() > 0) {
            map.clear();
        }
        renderState  = null;
        position = 0;
        templateList = null;
    }
}
