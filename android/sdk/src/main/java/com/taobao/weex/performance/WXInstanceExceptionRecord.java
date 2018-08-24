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

import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXErrorCode.ErrorGroup;
import com.taobao.weex.common.WXErrorCode.ErrorType;
import com.taobao.weex.common.WXJSExceptionInfo;
import com.taobao.weex.utils.WXExceptionUtils;

public class WXInstanceExceptionRecord {

    public static int sErrorMsgSizeLimit = 5;
    public static final String KEY_EXP_STAGE_LIST = "wxStageList";

    private final Map<String,Long> mStageMap;
    public final List<WXJSExceptionInfo> errorList;
    public final String instanceId;
    public final AtomicBoolean hasAddView;
    public final AtomicBoolean hasDegrade;
    private boolean mHasReportScreenEmpty = false;
    private boolean mBeginRender = false;

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
        if (null == exceptionInfo) {
            return;
        }
        //FOR RENDER_ERROR , record twice
        if (exceptionInfo.getErrCode().getErrorType() == ErrorType.RENDER_ERROR) {
            mHasReportScreenEmpty = true;
            return;
        }
        //screen has view, or degrade , will not be empty
        if (hasAddView.get() || hasDegrade.get()) {
            return;
        }
        if (errorList.size() > sErrorMsgSizeLimit) {
            errorList.remove(0);
        }
        errorList.add(exceptionInfo);
    }

    public void recordStage(String stage, long time) {
        if (WXInstanceApm.KEY_PAGE_STAGES_RENDER_ORGIGIN.equals(stage)
            || WXInstanceApm.KEY_PAGE_STAGES_DOWN_BUNDLE_START.equals(stage)
            || WXInstanceApm.KEY_PAGE_STAGES_LOAD_BUNDLE_START.equals(stage)
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
            return "emptyStage";
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
            return "empty";
        }
        StringBuilder builder = new StringBuilder();
        builder.append("======== error stack start (top5) =====\n");
        for (WXJSExceptionInfo info : errorList) {
            builder.append("time :").append(info.time).append("\n")
                .append("extErrorCode :").append(info.getErrCode().getErrorCode()).append('\n')
                .append("extErrorType :").append(info.getErrCode().getErrorType()).append('\n')
                .append("extErrorGroup :").append(info.getErrCode().getErrorGroup()).append('\n')
                .append("extErrorMsg :").append(info.getErrCode().getErrorMsg()).append('\n')
                .append("extErrorExceptionDetail :").append(info.getException()).append('\n')
                .append("extErrorStageList :").append(
                info.getExtParams().containsKey(KEY_EXP_STAGE_LIST) ? info.getExtParams().get(KEY_EXP_STAGE_LIST)
                    : "empty"
            ).append("\n")
                .append("=============  next ==============\n");
        }
        builder.append("======== error stack end =====\n");
        return builder.toString();
    }

    public void checkEmptyScreenAndReport() {
        if (!mBeginRender || mHasReportScreenEmpty || hasAddView.get() || hasDegrade.get()) {
            return;
        }
        boolean hasJsException = false;
        for (WXJSExceptionInfo info : errorList) {
            if (info.getErrCode().getErrorGroup() == ErrorGroup.JS) {
                hasJsException = true;
                break;
            }
        }
        Map<String,String> flagMap = new HashMap<>(4);
        flagMap.put("wxBeginRender",String.valueOf(mBeginRender));
        flagMap.put("wxHasAddView",String.valueOf(hasAddView.get()));
        flagMap.put("wxHasDegrade",String.valueOf(hasDegrade.get()));
        flagMap.put("wxHasReportScreenEmpty",String.valueOf(mHasReportScreenEmpty));

        WXExceptionUtils.commitCriticalExceptionRT(
            instanceId,
            hasJsException ? WXErrorCode.WX_RENDER_ERR_JS_RUNTIME : WXErrorCode.WX_RENDER_ERR_NATIVE_RUNTIME,
            "checkEmptyScreenAndReport",
            convertExceptionListToString(),
            flagMap
        );
    }

    @Override
    public String toString() {
        return new StringBuilder()
            .append(super.toString())
            .append("wxStageList :\n").append(convertStageToStr())
            .append("wxErrorList :\n").append(convertExceptionListToString())
            .toString();
    }
}
