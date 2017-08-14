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
package com.taobao.weex.dom.action;

import com.taobao.weex.dom.DOMAction;
import com.taobao.weex.dom.DOMActionContext;
import com.taobao.weex.dom.RenderAction;
import com.taobao.weex.dom.RenderActionContext;

class ExecutableRenderAction implements DOMAction, RenderAction {

    private Runnable mRunnable;

    ExecutableRenderAction(Runnable runnable) {
        this.mRunnable = runnable;
    }

    @Override
    public void executeDom(DOMActionContext context) {
        if (context.isDestory()) {
            return;
        }
        context.postRenderTask(this);
    }

    @Override
    public void executeRender(RenderActionContext context) {
        if (mRunnable != null) {
            mRunnable.run();
        }
    }
}
