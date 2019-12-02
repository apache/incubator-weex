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
package org.apache.weex.ui.module;

import org.apache.weex.WXSDKManager;
import org.apache.weex.annotation.JSMethod;
import org.apache.weex.common.WXModule;
import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.WXWeb;

public class WXWebViewModule extends WXModule {

    private enum Action {
        reload,
        goBack,
        goForward,
        postMessage
    }

    @JSMethod(uiThread = true)
    public void goBack(String ref) {
        action(Action.goBack, ref);
    }

    @JSMethod(uiThread = true)
    public void goForward(String ref) {
        action(Action.goForward, ref);
    }

    @JSMethod(uiThread = true)
    public void reload(String ref) {
        action(Action.reload, ref);
    }

    @JSMethod(uiThread = true)
    public void postMessage(String ref, Object msg) {
        action(Action.postMessage, ref, msg);
    }

    private void action(Action action, String ref, Object data) {
        WXComponent webComponent =
            WXSDKManager.getInstance()
                .getWXRenderManager()
                .getWXComponent(mWXSDKInstance.getInstanceId(), ref);
        if(webComponent instanceof WXWeb) {
            ((WXWeb) webComponent).setAction(action.name(), data);
        }
    }

    private void action(Action action, String ref) {
        action(action, ref, null);
    }

}
