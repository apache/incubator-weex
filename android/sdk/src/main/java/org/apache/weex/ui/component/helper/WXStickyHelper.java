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
package org.apache.weex.ui.component.helper;

import org.apache.weex.ui.component.Scrollable;
import org.apache.weex.ui.component.WXComponent;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by miomin on 16/7/7.
 */
public class WXStickyHelper {

    private Scrollable scrollable;

    public WXStickyHelper(Scrollable scrollable) {
        this.scrollable = scrollable;
    }

    public void bindStickStyle(WXComponent component, Map<String, Map<String, WXComponent>> mStickyMap) {
        Scrollable scroller = component.getParentScroller();
        if (scroller == null) {
            return;
        }
        Map<String, WXComponent> stickyMap = mStickyMap.get(scroller
                .getRef());
        if (stickyMap == null) {
            stickyMap = new ConcurrentHashMap<>();
        }
        if (stickyMap.containsKey(component.getRef())) {
            return;
        }
        stickyMap.put(component.getRef(), component);
        mStickyMap.put(scroller.getRef(), stickyMap);
    }

    public void unbindStickStyle(WXComponent component, Map<String, Map<String, WXComponent>> mStickyMap) {
        Scrollable scroller = component.getParentScroller();
        if (scroller == null) {
            return;
        }
        Map<String, WXComponent> stickyMap = mStickyMap.get(scroller
                .getRef());
        if (stickyMap == null) {
            return;
        }
        stickyMap.remove(component.getRef());
    }
}
