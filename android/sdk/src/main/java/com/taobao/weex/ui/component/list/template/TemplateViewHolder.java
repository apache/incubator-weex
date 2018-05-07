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

import android.os.AsyncTask;
import android.view.View;

import com.taobao.weex.ui.component.list.WXCell;
import com.taobao.weex.ui.view.listview.adapter.ListBaseViewHolder;

/**
 * Created by furture on 2017/8/17.
 */

public class TemplateViewHolder extends ListBaseViewHolder {

    /**
     * strong reference, prevent recycled
     * */
    private WXCell template;

    private WXRecyclerTemplateList templateList;

    private int holderPosition = -1;

    public AsyncTask<Void, Void, Void> asyncTask;

    public Object  data;

    /**
     * header position
     * */

    public TemplateViewHolder(WXRecyclerTemplateList templateList, WXCell component, int viewType) {
        super(component, viewType);
        this.template = component;
        this.templateList = templateList;
    }

    public TemplateViewHolder(WXRecyclerTemplateList templateList, View view, int viewType) {
        super(view, viewType);
        this.templateList = templateList;
    }




    public int getHolderPosition() {
        return holderPosition;
    }

    public void setHolderPosition(int holderPosition) {
        this.holderPosition = holderPosition;
    }

    public WXCell getTemplate() {
        return template;
    }

    public WXRecyclerTemplateList getTemplateList() {
        return templateList;
    }
}
