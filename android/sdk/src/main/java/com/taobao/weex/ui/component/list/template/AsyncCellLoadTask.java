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
import android.os.Looper;
import android.os.MessageQueue;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.ui.component.list.WXCell;
import com.taobao.weex.utils.WXLogUtils;

import java.util.Iterator;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * Created by furture on 2018/1/25.
 * async template cell component copy in background and init component when idle
 */
class AsyncCellLoadTask extends AsyncTask<Void,Void, Void> {

    private String template;
    private WXCell source;
    private WXRecyclerTemplateList templateList;

    public AsyncCellLoadTask(String template, WXCell source, WXRecyclerTemplateList templateList) {
        this.template = template;
        this.source = source;
        this.templateList = templateList;
    }

    /**
     * async template cell component copy in background
     * */
    @Override
    protected Void doInBackground(Void... params) {
        TemplateCache cellCache = templateList.getTemplatesCache().get(template);
        if(cellCache == null || cellCache.cells == null){
            return null;
        }
        while (cellCache.cells.size() < templateList.getTemplateCacheSize()){
            long start = System.currentTimeMillis();
            WXCell component = (WXCell) templateList.copyComponentFromSourceCell(source);
            if(WXEnvironment.isOpenDebugLog() && WXRecyclerTemplateList.ENABLE_TRACE_LOG){
                WXLogUtils.d(WXRecyclerTemplateList.TAG, " AsyncCellLoadTask load " + template
                        + "  " +  component.hashCode()
                + " used " + (System.currentTimeMillis() - start));
            }
            if(component == null){
                return null;
            }
            if(isDestory()){
                return null;
            }
            cellCache.cells.add(component);
        }
        return null;
    }

    /**
     * init component view when main thread idle
     * */
    @Override
    protected void onPostExecute(Void aVoid) {
        if(isDestory()){
            return;
        }
        final TemplateCache cellCache = templateList.getTemplatesCache().get(template);
        if(cellCache == null){
            return;
        }
        if(cellCache.cells == null
                || cellCache.cells.size() == 0){
            cellCache.isLoadIng = false;
            return;
        }
        Looper.myQueue().addIdleHandler(new MessageQueue.IdleHandler() {
            @Override
            public boolean queueIdle() {
                if(isDestory()){
                    return false;
                }
                ConcurrentLinkedQueue<WXCell> queue =  cellCache.cells;
                Iterator<WXCell> iterator =  queue.iterator();
                while (iterator.hasNext()){
                    WXCell component =  iterator.next();
                    if(component.isLazy()){
                        templateList.doCreateCellViewBindData(component, template, true);
                        return iterator.hasNext();
                    }
                }
                return false;
            }
        });
        cellCache.isLoadIng = false;
    }

    private boolean isDestory(){
        if(source.getInstance() == null
                || source.getInstance().isDestroy()){
            return true;
        }
        return templateList.isDestoryed();
    }

    /**
     * start cell load task on THREAD_POOL_EXECUTOR
     * */
    public void startTask(){
        executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

}
