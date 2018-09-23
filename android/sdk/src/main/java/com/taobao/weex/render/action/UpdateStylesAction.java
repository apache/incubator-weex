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
package com.taobao.weex.render.action;

import com.taobao.weex.render.bridge.RenderBridge;
import com.taobao.weex.render.view.DocumentView;

import java.util.Map;

/**
 * Created by furture on 2018/8/11.
 */

public class UpdateStylesAction extends  Action {

    private String ref;
    private Map<String, String> styles;

    public UpdateStylesAction(DocumentView documentView, String ref, Map<String, String> styles) {
        super(documentView);
        this.ref = ref;
        this.styles = styles;
    }


    @Override
    protected void execute() {
        RenderBridge.getInstance().actionUpdateStyles(getDocumentView().getNativeDocument(), ref, styles);
        getDocumentView().requestLayout();
    }
}
