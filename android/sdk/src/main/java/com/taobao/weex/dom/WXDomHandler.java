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
package com.taobao.weex.dom;

import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.dom.action.Actions;
import com.taobao.weex.dom.action.TraceableAction;

/**
 * Handler for dom operations.
 */
public class WXDomHandler implements Handler.Callback {

  /**
   * The batch operation in dom thread will run at most once in 16ms.
   */
  public static final int DELAY_TIME = 16;//ms
  public static final int TRANSITION_DELAY_TIME = 2;//2ms, start transition as soon as

  private WXDomManager mWXDomManager;
  private boolean mHasBatch = false;

  public WXDomHandler(WXDomManager domManager) {
    mWXDomManager = domManager;
  }

  @Override
  public boolean handleMessage(Message msg) {
    if (msg == null) {
      return false;
    }
    int what = msg.what;
    Object obj = msg.obj;
    WXDomTask task = null;

    if (obj != null && obj instanceof WXDomTask) {
      task = (WXDomTask) obj;
      Object action = ((WXDomTask) obj).args.get(0);
      if (action != null && action instanceof TraceableAction) {
        ((TraceableAction) action).mDomQueueTime = SystemClock.uptimeMillis() - msg.getWhen();
      }
    }

    if (!mHasBatch) {
      mHasBatch = true;
      if(what != WXDomHandler.MsgType.WX_DOM_BATCH) {
        int delayTime = DELAY_TIME;
        if(what == MsgType.WX_DOM_TRANSITION_BATCH){
          delayTime = TRANSITION_DELAY_TIME;
        }
        mWXDomManager.sendEmptyMessageDelayed(WXDomHandler.MsgType.WX_DOM_BATCH, delayTime);
      }
     }
    switch (what) {
      case MsgType.WX_EXECUTE_ACTION:
        mWXDomManager.executeAction(task.instanceId, (DOMAction) task.args.get(0), (boolean) task.args.get(1));
        break;
      case MsgType.WX_DOM_UPDATE_STYLE:
        //keep this for direct native call
        mWXDomManager.executeAction(task.instanceId, Actions.getUpdateStyle((String) task.args.get(0),
            (JSONObject) task.args.get(1),
            task.args.size() > 2 && (boolean) task.args.get(2)),false);
        break;
      case MsgType.WX_DOM_BATCH:

        mWXDomManager.batch();
        mHasBatch = false;
        break;
      case MsgType.WX_CONSUME_RENDER_TASKS:
        mWXDomManager.consumeRenderTask(task.instanceId);
        break;
      default:
        break;
    }
    return true;
  }


  public static class MsgType {

    @Deprecated
    public static final int WX_DOM_CREATE_BODY = 0x0;
    @Deprecated
    public static final int WX_DOM_UPDATE_ATTRS = 0x01;
    @Deprecated
    public static final int WX_DOM_UPDATE_STYLE = 0x02;
    @Deprecated
    public static final int WX_DOM_ADD_DOM = 0x03;
    @Deprecated
    public static final int WX_DOM_REMOVE_DOM = 0x04;
    @Deprecated
    public static final int WX_DOM_MOVE_DOM = 0x05;
    @Deprecated
    public static final int WX_DOM_ADD_EVENT = 0x06;
    @Deprecated
    public static final int WX_DOM_REMOVE_EVENT = 0x07;
    @Deprecated
    public static final int WX_DOM_SCROLLTO = 0x08;
    @Deprecated
    public static final int WX_DOM_CREATE_FINISH = 0x09;
    @Deprecated
    public static final int WX_DOM_REFRESH_FINISH = 0x0a;
    @Deprecated
    public static final int WX_DOM_UPDATE_FINISH = 0x0b;
    @Deprecated
    public static final int WX_ANIMATION=0xc;
    @Deprecated
    public static final int WX_DOM_ADD_RULE=0xd;
    @Deprecated
    public static final int WX_DOM_INVOKE=0xe;

    public static final int WX_EXECUTE_ACTION = 0xfe;
    public static final int WX_DOM_BATCH = 0xff;
    public static final int WX_CONSUME_RENDER_TASKS = 0xfa;


    public static final int WX_DOM_TRANSITION_BATCH = 0xfb;


    public static final int WX_DOM_START_BATCH = 0xfc;

    @Deprecated
    public static final int WX_COMPONENT_SIZE= 0xff1;

  }
}
