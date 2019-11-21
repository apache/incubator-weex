/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

package org.apache.weex.performance;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentLinkedQueue;

import android.support.annotation.NonNull;
import org.apache.weex.WXSDKManager;
import org.apache.weex.adapter.IWXConfigAdapter;
import org.apache.weex.bridge.WXBridgeManager;
import org.apache.weex.utils.WXUtils;

/**
 * @author zhongcang
 * @date 2019/6/19
 */
public class WXStateRecord {

    private RecordList<Info> mExceptionHistory;
    private RecordList<Info> mActionHistory;
    private RecordList<Info> mJsfmInitHistory;
    private RecordList<Info> mJscCrashHistory;
    private RecordList<Info> mJscReloadHistory;
    private RecordList<Info> mJsThradWatchHistory;
    private RecordList<Info> mIPCExceptionHistory;

    private static class SingleTonHolder {
        private static final WXStateRecord S_INSTANCE = new WXStateRecord();
    }

    public static WXStateRecord getInstance() {
        return SingleTonHolder.S_INSTANCE;
    }

    private WXStateRecord() {
        mExceptionHistory = new RecordList<>(10);
        mActionHistory = new RecordList<>(20);
        mJsfmInitHistory = new RecordList<>(10);
        mJscCrashHistory = new RecordList<>(10);
        mJscReloadHistory = new RecordList<>(10);
        mJsThradWatchHistory = new RecordList<>(20);
        mIPCExceptionHistory = new RecordList<>(20);
    }

    /**
     * check history exception (may be cause ws)
     */
    public void recordException(String instanceId, String exception) {
        String shortException = exception.length() > 200 ?exception.substring(0,200) : exception;
        recordCommon(mExceptionHistory,new Info(WXUtils.getFixUnixTime(), instanceId, shortException));
    }

    /**
     * check history action (may be occupy cpu by preInstance or some task)
     */
    public void recordAction(String instanceId, String action) {
        recordCommon(mActionHistory,new Info(WXUtils.getFixUnixTime(), instanceId, action));
    }

    public void recordIPCException (String instanceId,String exception){
        String shortException = exception.length() > 200 ?exception.substring(0,200) : exception;
        recordCommon(mIPCExceptionHistory,new Info(WXUtils.getFixUnixTime(), instanceId, shortException));
    }

    /**
     * check onJSFMInit time,and we know when jsfm is init sucess in reloadJsEngine case
     */
    public void onJSFMInit() {
        recoreJsfmInitHistory("setJsfmVersion");
    }

    public void recoreJsfmInitHistory(String msg){
        recordCommon(mJsfmInitHistory,new Info(WXUtils.getFixUnixTime(), "JSFM", msg));
    }

    public void recordJsThreadWatch(String msg){
        recordCommon(mJsThradWatchHistory,new Info(WXUtils.getFixUnixTime(), "jsWatch", msg));
    }

    /**
     * check onJSEngineReload time,and we know how many times reload and each reload time
     */
    public void onJSEngineReload(String instanceId) {
        recordCommon(mJscReloadHistory,new Info(WXUtils.getFixUnixTime(), instanceId, "onJSEngineReload"));
    }

    /**
     * check jsc crash time,and we know how many times jscCrash and each crash time
     */
    public void onJSCCrash(String instanceId) {
        recordCommon(mJscCrashHistory,new Info(WXUtils.getFixUnixTime(), instanceId, "onJSCCrash"));
    }

    private void recordCommon(RecordList<Info> list ,Info info){
        if (null == list || null == info){
            return;
        }
        try {
            list.add(info);
            if (!list.isEmpty() && list.size()>list.maxSize){
                list.poll();
            }
        }catch (Throwable e){
            e.getStackTrace();
        }
    }

    public Map<String, String> getStateInfo() {
        Map<String, String> stateInfo = new HashMap<>(5);
        stateInfo.put("reInitCount", String.valueOf(WXBridgeManager.reInitCount));

        int size = mExceptionHistory.size()+mActionHistory.size()+mJsfmInitHistory.size()
            +mJscCrashHistory.size()+mJscReloadHistory.size()+mJsThradWatchHistory.size();

        List<Info> reportTimeLineInfo = new ArrayList<>(size);
        reportTimeLineInfo.addAll(mExceptionHistory);
        reportTimeLineInfo.addAll(mActionHistory);
        reportTimeLineInfo.addAll(mJsfmInitHistory);
        reportTimeLineInfo.addAll(mJscCrashHistory);
        reportTimeLineInfo.addAll(mJscReloadHistory);
        reportTimeLineInfo.addAll(mJsThradWatchHistory);
        reportTimeLineInfo.addAll(mIPCExceptionHistory);
        Collections.sort(reportTimeLineInfo);
        stateInfo.put("stateInfoList",reportTimeLineInfo.toString());

        IWXConfigAdapter adapter = WXSDKManager.getInstance().getWxConfigAdapter();
        if (null != adapter && "true".equalsIgnoreCase(adapter.getConfig("wxapm","dumpIpcPageInfo","true"))){
            stateInfo.put("pageQueueInfo",WXBridgeManager.getInstance().dumpIpcPageInfo());
        }
        return stateInfo;
    }

    private static class RecordList<E> extends ConcurrentLinkedQueue<E> {
        private int maxSize;

        public RecordList(int maxSize) {
            super();
            this.maxSize = maxSize;
        }
    }

    private static class Info implements Comparable<Info>{
        private long time;
        private String instanceId;
        private String msg;

        public Info(long time, String instance, String msg) {
            this.time = time;
            this.instanceId = instance;
            this.msg = msg;
        }

        @Override
        public String toString() {
            return new StringBuilder()
                .append('[').append(instanceId).append(',').append(time).append(',').append(msg).append("]->")
                .toString();
        }

        @Override
        public int compareTo(@NonNull Info next) {
            if (this.time == next.time){
                return 0;
            }
            return this.time > next.time? 1:-1;
        }
    }

    public void startJSThreadWatchDog(){
        WXBridgeManager.getInstance().post(jsThreadWatchTask);
    }

    private  long jsThreadTime =-1;

    private Runnable jsThreadWatchTask = new Runnable() {
        @Override
        public void run() {
            if (jsThreadTime == -1){
                jsThreadTime = WXUtils.getFixUnixTime();
            }
            long diff = WXUtils.getFixUnixTime() - jsThreadTime;
            recordJsThreadWatch("diff:"+diff);
            jsThreadTime = WXUtils.getFixUnixTime();
            WXBridgeManager.getInstance().postDelay(jsThreadWatchTask,500);
        }
    };
}
