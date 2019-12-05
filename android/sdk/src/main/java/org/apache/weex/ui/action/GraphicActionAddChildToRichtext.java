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
package org.apache.weex.ui.action;

import android.support.annotation.NonNull;

import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.ui.component.richtext.WXRichText;

import java.util.HashMap;

public class GraphicActionAddChildToRichtext extends BasicGraphicAction {

    public GraphicActionAddChildToRichtext(@NonNull WXSDKInstance instance, String nodeType, String ref, String parentRef, String richTextRef,
                                           HashMap<String, String> styles, HashMap<String, String> attrs){
        super(instance,richTextRef);
        if(WXSDKManager.getInstance() != null && WXSDKManager.getInstance().getWXRenderManager() != null) {
            WXRichText richText = (WXRichText) WXSDKManager.getInstance().getWXRenderManager()
                    .getWXComponent(getPageId(), richTextRef);
            if (richText != null) {
                richText.AddChildNode(ref, nodeType, parentRef, styles, attrs);
            }
        }
    }
    @Override
    public void executeAction() {

    }
}
