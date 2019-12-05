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

import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.ui.component.richtext.WXRichText;

public class GraphicActionRemoveChildFromRichtext extends BasicGraphicAction{
    private String ref;
    private String parentRef;
    private WXRichText richText;
    public GraphicActionRemoveChildFromRichtext(WXSDKInstance instance,String ref,String parentRef,String richtextRef){
        super(instance, richtextRef);
        this.ref = ref;
        this.parentRef = parentRef;
        richText = (WXRichText) WXSDKManager.getInstance().getWXRenderManager().
                getWXComponent(instance.getInstanceId(),richtextRef);
        if(richText != null){
            richText.removeChildNode(parentRef,ref);
        }
    }
    @Override
    public void executeAction() {
    }
}
