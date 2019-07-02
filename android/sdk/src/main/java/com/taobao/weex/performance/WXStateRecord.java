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

package com.taobao.weex.performance;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import com.taobao.weex.utils.WXUtils;

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

    private static class SingleTonHolder {
        private static final WXStateRecord S_INSTANCE = new WXStateRecord();
    }

    public static WXStateRecord getInstance() {
        return SingleTonHolder.S_INSTANCE;
    }

    private WXStateRecord() {
        mExceptionHistory = new RecordList<>(5);
        mActionHistory = new RecordList<>(10);
        mJsfmInitHistory = new RecordList<>(3);
        mJscCrashHistory = new RecordList<>(3);
        mJscReloadHistory = new RecordList<>(5);
    }

    /**
     * check history exception (may be cause ws)
     */
    public void recordException(String instanceId, String exception) {
        String shortException = exception.length() > 200 ?exception.substring(0,200) : exception;
        mExceptionHistory.add(new Info(WXUtils.getFixUnixTime(), instanceId, shortException));
    }

    /**
     * check history action (may be occupy cpu by preInstance or some task)
     */
    public void recordAction(String instanceId, String action) {
        mActionHistory.add(new Info(WXUtils.getFixUnixTime(), instanceId, action));
    }

    /**
     * check onJSFMInit time,and we know when jsfm is init sucess in reloadJsEngine case
     */
    public void onJSFMInit() {
        mJsfmInitHistory.add(new Info(WXUtils.getFixUnixTime(), "JSFM", "onJsfmInit"));
    }

    /**
     * check onJSEngineReload time,and we know how many times reload and each reload time
     */
    public void onJSEngineReload() {
        mJscReloadHistory.add(new Info(WXUtils.getFixUnixTime(), "", "onJSEngineReload"));
    }

    /**
     * check jsc crash time,and we know how many times jscCrash and each crash time
     */
    public void onJSCCrash() {
        mJscCrashHistory.add(new Info(WXUtils.getFixUnixTime(), "", "onJSCCrash"));
    }

    public Map<String, String> getStateInfo() {
        Map<String, String> stateInfo = new HashMap<>(5);
        stateInfo.put("exceptionHistory", mExceptionHistory.toString());
        stateInfo.put("actionHistory", mActionHistory.toString());
        stateInfo.put("jsfmInitHistory", mJsfmInitHistory.toString());
        stateInfo.put("jscCrashHistory", mJscCrashHistory.toString());
        stateInfo.put("jscReloadHistory", mJscReloadHistory.toString());
        return stateInfo;
    }

    private static class RecordList<E> extends ArrayList<E> {
        private int maxSize;

        public RecordList(int maxSize) {
            super(maxSize);
            this.maxSize = maxSize;
        }

        @Override
        public boolean add(E e) {
            if (this.size()>0 && this.size() >= maxSize){
                remove(0);
            }
            return super.add(e);
        }

        @Override
        public String toString() {
            StringBuilder builder = new StringBuilder();
            int size = size();
            for (int i = 0; i < size; i++) {
                builder.append('[').append(get(i).toString()).append(']').append("->");
            }
            return builder.toString();
        }
    }

    private static class Info {
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
                .append(instanceId).append(',').append(time).append(',').append(msg)
                .toString();
        }
    }
}
