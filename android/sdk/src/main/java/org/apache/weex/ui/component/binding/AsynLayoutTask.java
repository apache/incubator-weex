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
package org.apache.weex.ui.component.binding;

import android.os.AsyncTask;

import org.apache.weex.ui.component.WXComponent;
import org.apache.weex.ui.component.list.template.TemplateDom;
import org.apache.weex.ui.component.list.template.TemplateViewHolder;

/**
 * Created by furture on 2018/5/11.
 */

class AsynLayoutTask extends AsyncTask<Void, Void, Void> {

    private final TemplateViewHolder templateViewHolder;
    private final int position;
    private final WXComponent component;

    AsynLayoutTask(TemplateViewHolder templateViewHolder, int position, WXComponent component) {
        this.templateViewHolder = templateViewHolder;
        this.position = position;
        this.component = component;
    }

    @Override
    protected Void doInBackground(Void... params) {
        if(templateViewHolder.getHolderPosition() == position){
            if(component.getInstance() != null && !component.getInstance().isDestroy()) {
                synchronized (templateViewHolder.getTemplateList()){
                    if(templateViewHolder.getTemplateList().isDestoryed()){
                        return null;
                    }
                    Layouts.doLayoutOnly(component, templateViewHolder);
                }
            }
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void aVoid) {
        if(position == templateViewHolder.getHolderPosition()) {
            if(component.getInstance() != null && !component.getInstance().isDestroy()) {
                Layouts.setLayout(component, false);
                if(templateViewHolder.getHolderPosition() >= 0){
                    templateViewHolder.getTemplateList().fireEvent("_attach_slot", TemplateDom
                        .findAllComponentRefs(templateViewHolder.getTemplateList().getRef(), position, component));
                }
            }
        }
    }
}
