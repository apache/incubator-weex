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
package com.taobao.weex.render.task;

import android.util.Log;

import com.taobao.weex.render.log.RenderLog;
import com.taobao.weex.render.manager.RenderManager;
import com.taobao.weex.render.view.DocumentView;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by furture on 2018/8/1.
 */

public abstract class GLTask {

    private DocumentView documentView;
    private static AtomicInteger taskNum = new AtomicInteger(0);

    public GLTask(DocumentView documentView) {
        this.documentView = documentView;
        taskNum.incrementAndGet();
    }

    public DocumentView getDocumentView() {
        return documentView;
    }


    public final void execute(){
        try {
            run();
        }finally {
            taskNum.decrementAndGet();
        }
    }

    public abstract void run();


    public static int getTaskNum(){
        return taskNum.get();
    }

    public static void waitIfTaskBlock(){
        if(RenderLog.isRenderLogEnabled()){
            Log.d(RenderLog.RENDER_LOG_TAG, "RenderTask waitIfTaskBlock " + RenderManager.getInstance().getDocumentsMap().size() + " taskNum " + taskNum.get() + " renderNum " + OpenGLRender.getRenderNum());
        }
        int maxTimes = taskNum.get();
        while (taskNum.get() >= 8 && maxTimes > 0){
            try {
                Thread.sleep(4);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            maxTimes--;
        }
    }
}
