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
package com.taobao.weex.utils.batch;

import com.taobao.weex.utils.batch.BactchExecutor;
import com.taobao.weex.utils.batch.Interceptor;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Created by sospartan on 8/24/16.
 */
public class BatchOperationHelper implements Interceptor {


  private BactchExecutor mExecutor;
  private CopyOnWriteArrayList<Runnable> sRegisterTasks = new CopyOnWriteArrayList<>();
  private boolean isCollecting = false;

  public BatchOperationHelper(BactchExecutor executor){
    mExecutor = executor;
    executor.setInterceptor(this);
    isCollecting = true;
  }

  @Override
  public boolean take(Runnable runnable) {
    if(isCollecting){
      sRegisterTasks.add(runnable);
      return true;
    }
    return false;
  }

  /**
   * Post all tasks to executor. Can only be called once.
   */
  public void flush(){
    isCollecting = false;
    mExecutor.post(new Runnable() {
      @Override
      public void run() {
        Iterator<Runnable> iterator = sRegisterTasks.iterator();
        while(iterator.hasNext()){
          Runnable item = iterator.next();
          item.run();
//          iterator.remove();
          sRegisterTasks.remove(item);
        }
      }
    });
    mExecutor.setInterceptor(null);
  }
}
