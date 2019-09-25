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

import android.text.TextUtils;

import com.alibaba.fastjson.JSONObject;
import org.apache.weex.WXSDKInstance;
import org.apache.weex.WXSDKManager;
import org.apache.weex.bridge.SimpleJSCallback;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.list.template.CellDataManager;
import org.apache.weex.ui.component.list.template.WXRecyclerTemplateList;
import org.apache.weex.utils.WXLogUtils;


/**
 * Created by furture on 2018/1/23.
 */

public class UpdateComponentDataAction extends BasicGraphicAction{

    private String virtualComponentId;
    private JSONObject data;
    private String callback;


    public UpdateComponentDataAction(WXSDKInstance instance, String virtualComponentId,
                                     JSONObject data, String callback) {
        super(instance, CellDataManager.getListRef(virtualComponentId));
        this.virtualComponentId = virtualComponentId;
        this.data = data;
        this.callback = callback;
    }

    @Override
    public void executeAction() {
        if(TextUtils.isEmpty(getRef())){
            WXLogUtils.e("wrong virtualComponentId split error " + virtualComponentId);
            return;
        }
        WXComponent component = WXSDKManager
            .getInstance().getWXRenderManager().getWXComponent(getPageId(), getRef());
        if(component instanceof WXRecyclerTemplateList){
            WXRecyclerTemplateList templateList = (WXRecyclerTemplateList) component;
            templateList.getCellDataManager().updateVirtualComponentData(virtualComponentId, data);
            templateList.notifyUpdateList();
            SimpleJSCallback jsCallback = new SimpleJSCallback(component.getInstanceId(), callback);
            jsCallback.invoke(true);
        }else{
            WXLogUtils.e("recycler-list wrong virtualComponentId " + virtualComponentId);
        }
    }

}
