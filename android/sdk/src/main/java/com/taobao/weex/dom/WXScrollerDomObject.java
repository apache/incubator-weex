/*
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
package com.taobao.weex.dom;

import android.support.v4.util.ArrayMap;

import com.taobao.weex.common.Constants;

import java.util.Map;

public class WXScrollerDomObject extends WXDomObject {

    @Override
    protected Map<String, String> getDefaultStyle() {
        Map<String, String> map = new ArrayMap<>();

        boolean isVertical = true;
        if (parent != null) {
            String direction = (String) parent.getAttrs().get(Constants.Name.SCROLL_DIRECTION);
            if (direction != null && direction.equals("horizontal")) {
                isVertical = false;
            }
        }

        String prop = isVertical?Constants.Name.HEIGHT:Constants.Name.WIDTH;
        if (getStyles().get(prop) == null &&
            getStyles().get(Constants.Name.FLEX) == null) {
            map.put(Constants.Name.FLEX, "1");
        }

        return map;
    }
}
