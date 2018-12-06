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
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicBoolean;

import android.text.TextUtils;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXErrorCode.ErrorGroup;
import com.taobao.weex.common.WXErrorCode.ErrorType;
import com.taobao.weex.common.WXJSExceptionInfo;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXUtils;

public class WXInstanceExceptionRecord {

    public static int sErrorMsgSizeLimit = 5;
    public static final String KEY_EXP_STAGE_LIST = "wxStageList";

    private final Map<String,Long> mStageMap;
    public final List<String> errorList;
    public final String instanceId;
    public final AtomicBoolean hasAddView;
    public final AtomicBoolean hasDegrade;
    private boolean mHasReportScreenEmpty = false;
    private boolean mBeginRender = false;
    public boolean isDownLoadBundleFailed = false;
    public static boolean isReportWriteScreen = false;

    public WXInstanceExceptionRecord(String instanceId) {
        this.instanceId = instanceId;
        this.mStageMap = new ConcurrentHashMap<>();
        this.errorList = new CopyOnWriteArrayList<>();
        this.hasAddView = new AtomicBoolean(false);
        this.hasDegrade = new AtomicBoolean(false);
    }

    /**
     * record error on instance (js、native)
     * when checkEmptyScreen, report msg
     */
    public void recordErrorMsg(WXJSExceptionInfo exceptionInfo) {
        if (isReportWriteScreen){
            return;
        }
        if (null == exceptionInfo) {
            return;
        }
        //FOR RENDER_ERROR , record twice
        if (exceptionInfo.getErrCode().getErrorType() == ErrorType.RENDER_ERROR) {
            mHasReportScreenEmpty = true;
            return;
        }
        recordReportErrorMsg(exceptionInfo.toString());
    }

    public void recordReportErrorMsg(String appendStr){
        if (!isReportWriteScreen){
            return;
        }
        //screen has view, or degrade , will not be empty
        if (TextUtils.isEmpty(appendStr) || hasAddView.get() || hasDegrade.get()) {
            return;
        }
        if (errorList.size() > sErrorMsgSizeLimit) {
            errorList.remove(0);
        }
        errorList.add(new StringBuilder()
            .append("time ").append(System.currentTimeMillis())
            .append(",msg ").append(appendStr).toString()
        );
    }

    public void recordStage(String stage, long time) {
        if (WXInstanceApm.KEY_PAGE_STAGES_RENDER_ORGIGIN.equals(stage)
            || WXInstanceApm.KEY_PAGE_STAGES_DOWN_BUNDLE_START.equals(stage)
            || WXInstanceApm.KEY_PAGE_STAGES_LOAD_BUNDLE_END.equals(stage)
            ) {
            setBeginRender(true);
        }
        mStageMap.put(stage,time);
    }

    public void setBeginRender(boolean isBegin){
        mBeginRender = true;
    }

    public String convertStageToStr() {
        if (mStageMap.isEmpty()) {
            return "noStageRecord";
        }
        List<Map.Entry<String,Long>> list = new ArrayList<>(mStageMap.entrySet());
        Collections.sort(list, new Comparator<Entry<String, Long>>() {
            @Override
            public int compare(Entry<String, Long> o1, Entry<String, Long> o2) {
                return (int)(o1.getValue() - o2.getValue());
            }
        });

        StringBuilder builder = new StringBuilder();
        for (Map.Entry<String,Long> entry : list) {
            builder.append(entry.getKey()).append(':').append(entry.getValue()).append("->");
        }
        return builder.toString();
    }

    private String convertExceptionListToString() {
        if (errorList.isEmpty()) {
            return "";
        }
        StringBuilder builder = new StringBuilder();
        int i = 0;
        for (String info : errorList) {
            builder.append("error_").append(i).append(": ").append(info).append("--->");
        }
        return builder.toString();
    }

    public void checkEmptyScreenAndReport() {
        if (!isReportWriteScreen){
            return;
        }
        if (isDownLoadBundleFailed || !mBeginRender || mHasReportScreenEmpty || hasAddView.get() || hasDegrade.get()) {
            return;
        }

        if (!mStageMap.containsKey(WXInstanceApm.KEY_PAGE_STAGES_CREATE_FINISH)){
            return;
        }

        Long startExecJsTime = mStageMap.get(WXInstanceApm.KEY_PAGE_STAGES_LOAD_BUNDLE_END);
        if (null == startExecJsTime){
            //too fast to quit
            return;
        }
        long currentTime = WXUtils.getFixUnixTime();
        long jsExecTime = currentTime - startExecJsTime;
        //4s limit of instance stayTime (case in\quit very fast case)
        if (jsExecTime <= 4000){
            return;
        }

        String errorMsg;
        if(errorList.isEmpty()){
            errorMsg = "whiteScreen :never add view until page destroy,(js has execute > 4s,has createFinish)";
        }else {
            errorMsg =  "whiteScreen :history exception :"+ convertExceptionListToString();
        }


        Map<String,String> flagMap = new HashMap<>(4);
        flagMap.put("wxBeginRender",String.valueOf(mBeginRender));
        flagMap.put("wxHasAddView",String.valueOf(hasAddView.get()));
        flagMap.put("wxHasDegrade",String.valueOf(hasDegrade.get()));
        flagMap.put("wxHasReportScreenEmpty",String.valueOf(mHasReportScreenEmpty));
        flagMap.put("wxJSExecTime", String.valueOf(jsExecTime));

        //WXExceptionUtils.commitCriticalExceptionRT(
        //    instanceId,
        //    WXErrorCode.WX_RENDER_ERR_JS_RUNTIME,
        //    "checkEmptyScreenAndReport",
        //    errorMsg,
        //    flagMap
        //);
    }

    public Long getStageTime(String key){
        return mStageMap.get(key);
    }



    @Override
    public String toString() {
        return new StringBuilder()
            .append(super.toString())
            .append("wxStageList :").append(convertStageToStr())
            .append("wxErrorList :").append(convertExceptionListToString())
            .toString();
    }
}
