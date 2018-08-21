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

import com.alibaba.fastjson.JSONAware;
import com.alibaba.fastjson.annotation.JSONType;

/**
 * position render state, when render state change, position changed
 * Created by furture on 2018/2/2.
 */
public class PositionRef extends  Number implements JSONAware {

    private CellRenderState renderState;

    public PositionRef(CellRenderState renderState) {
        this.renderState = renderState;
    }

    @Override
    public int intValue() {
        return getPosition();
    }

    @Override
    public long longValue() {
        return getPosition();
    }

    @Override
    public float floatValue() {
        return getPosition();
    }

    @Override
    public double doubleValue() {
        return getPosition();
    }

    private int getPosition(){
        if(renderState == null){
            return  -1;
        }
        return renderState.position;
    }

    @Override
    public String toString() {
        return String.valueOf(getPosition());
    }

    @Override
    public String toJSONString() {
        return String.valueOf(getPosition());
    }
}
