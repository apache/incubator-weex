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
package org.apache.weex;

import android.net.Uri;
import android.text.TextUtils;

import org.apache.weex.adapter.IWXUserTrackAdapter;
import org.apache.weex.common.WXErrorCode;
import org.apache.weex.common.WXPerformance;
import org.apache.weex.common.WXRenderStrategy;
import org.apache.weex.common.WXResponse;
import org.apache.weex.performance.WXInstanceApm;
import org.apache.weex.tracing.WXTracing;
import org.apache.weex.utils.WXLogUtils;
import org.apache.weex.utils.tools.LogDetail;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.apache.weex.adapter.IWXHttpAdapter.OnHttpListener;

/**
 * load bundle js listener
 */
public class WXHttpListener implements OnHttpListener {

    private String pageName;
    private Map<String, Object> options;
    private String jsonInitData;
    private WXRenderStrategy flag;
    private WXSDKInstance instance;
    private long startRequestTime;
    private int traceId;
    private WXPerformance mWXPerformance;
    private WXInstanceApm mApmForInstance;
    private IWXUserTrackAdapter mUserTrackAdapter;
    public boolean isPreDownLoadMode = false;
    private boolean isInstanceReady =false;
    private boolean isResponseHasWait = false;
    private WXResponse mResponse;
    private LogDetail mLogDetail;

    private String mBundleUrl;

    public WXHttpListener(WXSDKInstance instance) {
        if(instance != null) {
            mLogDetail = instance.mTimeCalculator.createLogDetail("downloadBundleJS");
        }
        this.instance = instance;
        this.traceId = WXTracing.nextId();
        this.mWXPerformance = instance.getWXPerformance();
        this.mApmForInstance = instance.getApmForInstance();
        this.mUserTrackAdapter = WXSDKManager.getInstance().getIWXUserTrackAdapter();
        if (WXTracing.isAvailable()) {
            WXTracing.TraceEvent event = WXTracing.newEvent("downloadBundleJS", instance.getInstanceId(), -1);
            event.iid = instance.getInstanceId();
            event.tname = "Network";
            event.ph = "B";
            event.traceId = traceId;
            event.submit();
        }
    }

    public WXHttpListener(WXSDKInstance instance, String bundleUrl) {
        this(instance);
        this.startRequestTime = System.currentTimeMillis();
        this.mBundleUrl = bundleUrl;
    }

    public WXHttpListener(WXSDKInstance instance, String pageName, Map<String, Object> options, String jsonInitData, WXRenderStrategy flag, long startRequestTime) {
        this(instance);
        this.pageName = pageName;
        this.options = options;
        this.jsonInitData = jsonInitData;
        this.flag = flag;
        this.startRequestTime = startRequestTime;
        this.mBundleUrl = instance.getBundleUrl();
    }

    public void setSDKInstance(WXSDKInstance instance) {
        this.instance = instance;
    }

    protected WXSDKInstance getInstance() {
        return instance;
    }

    @Override
    public void onHttpStart() {
        if (this.instance != null
                && this.instance.getWXStatisticsListener() != null) {
            this.instance.getWXStatisticsListener().onHttpStart();
            if(mLogDetail != null) {
                mLogDetail.taskStart();
            }
        }
    }

    @Override
    public void onHeadersReceived(int statusCode, Map<String,List<String>> headers) {
        if (this.instance != null
                && this.instance.getWXStatisticsListener() != null) {
            this.instance.getWXStatisticsListener().onHeadersReceived();
            this.instance.onHttpStart();
        }
        if(this.instance != null
                && this.instance.responseHeaders != null
                && headers != null){
            this.instance.responseHeaders.putAll(headers);
        }
    }

    @Override
    public void onHttpUploadProgress(int uploadProgress) {

    }

    @Override
    public void onHttpResponseProgress(int loadedLength) {
        instance.getApmForInstance().extInfo.put(WXInstanceApm.VALUE_BUNDLE_LOAD_LENGTH,loadedLength);
    }

    @Override
    public void onHttpFinish(WXResponse response) {
        if(mLogDetail != null) {
            mLogDetail.taskEnd();
        }
        if (this.instance != null
                && this.instance.getWXStatisticsListener() != null) {
            this.instance.getWXStatisticsListener().onHttpFinish();
        }

        if (WXTracing.isAvailable()) {
            WXTracing.TraceEvent event = WXTracing.newEvent("downloadBundleJS", instance.getInstanceId(), -1);
            event.traceId = traceId;
            event.tname = "Network";
            event.ph = "E";
            event.extParams = new HashMap<>();
            if (response != null && response.originalData != null) {
                event.extParams.put("BundleSize", response.originalData.length);
            }
            event.submit();
        }

        mWXPerformance.networkTime = System.currentTimeMillis() - startRequestTime;
        if(null!= response && response.extendParams!=null){
            mApmForInstance.updateRecordInfo(response.extendParams);
            Object actualNetworkTime=response.extendParams.get("actualNetworkTime");
            mWXPerformance.actualNetworkTime=actualNetworkTime instanceof Long?(long)actualNetworkTime:0;

            Object pureNetworkTime=response.extendParams.get("pureNetworkTime");
            mWXPerformance.pureNetworkTime=pureNetworkTime instanceof Long?(long)pureNetworkTime:0;

            Object connectionType=response.extendParams.get("connectionType");
            mWXPerformance.connectionType=connectionType instanceof String?(String)connectionType:"";

            Object packageSpendTime=response.extendParams.get("packageSpendTime");
            mWXPerformance.packageSpendTime=packageSpendTime instanceof Long ?(long)packageSpendTime:0;

            Object syncTaskTime=response.extendParams.get("syncTaskTime");
            mWXPerformance.syncTaskTime=syncTaskTime instanceof Long ?(long)syncTaskTime:0;

            Object requestType=response.extendParams.get("requestType");
            mWXPerformance.requestType=requestType instanceof String?(String)requestType:"none";

            Object cacheType = response.extendParams.get(WXPerformance.Dimension.cacheType.toString());
            if(cacheType instanceof String){
                mWXPerformance.cacheType = (String) cacheType;
            }

            Object zCacheInfo = response.extendParams.get("zCacheInfo");
            mWXPerformance.zCacheInfo = zCacheInfo instanceof String?(String)zCacheInfo:"";

            if(isNet(mWXPerformance.requestType) && mUserTrackAdapter!=null){
                WXPerformance performance=new WXPerformance(instance.getInstanceId());
                if(!TextUtils.isEmpty(mBundleUrl)){
                    try {
                        performance.args= Uri.parse(mBundleUrl).buildUpon().clearQuery().toString();
                    } catch (Exception e) {
                        performance.args=pageName;
                    }
                }
                if(!"200".equals(response.statusCode)){
                    performance.errCode= WXErrorCode.WX_ERR_JSBUNDLE_DOWNLOAD.getErrorCode();
                    performance.appendErrMsg(response.errorCode);
                    performance.appendErrMsg("|");
                    performance.appendErrMsg(response.errorMsg);

                }else if("200".equals(response.statusCode) && (response.originalData==null || response.originalData.length<=0)){
                    performance.errCode=WXErrorCode.WX_ERR_JSBUNDLE_DOWNLOAD.getErrorCode();
                    performance.appendErrMsg(response.statusCode);
                    performance.appendErrMsg("|template is null!");
                }else {
                    performance.errCode=WXErrorCode.WX_SUCCESS.getErrorCode();
                }

                if (mUserTrackAdapter != null) {
                    mUserTrackAdapter.commit(instance.getContext(), null, IWXUserTrackAdapter.JS_DOWNLOAD, performance, null);
                }
            }
        }

        if (isPreDownLoadMode){
            if (isInstanceReady){
                WXLogUtils.d("test->", "DownLoad didHttpFinish on http" );
                didHttpFinish(response);
            }else {
                WXLogUtils.d("test->", "DownLoad end before activity created" );
                mResponse = response;
                isResponseHasWait = true;
            }
        }else {
            didHttpFinish(response);
        }

    }

    public void onInstanceReady(){
        if (!isPreDownLoadMode){
            return;
        }
        this.isInstanceReady = true;
        if (isResponseHasWait){
            WXLogUtils.d("test->", "preDownLoad didHttpFinish on ready" );
            this.didHttpFinish(mResponse);
        }

    }

    private void didHttpFinish(WXResponse response){
        String wxErrorCode = WXInstanceApm.VALUE_ERROR_CODE_DEFAULT;
        if (response!=null && response.originalData!=null && TextUtils.equals("200", response.statusCode)) {
            mApmForInstance.onStage(WXInstanceApm.KEY_PAGE_STAGES_DOWN_BUNDLE_END);
            onSuccess(response);

            // check content-type
        } else if (TextUtils.equals(WXErrorCode.WX_DEGRAD_ERR_BUNDLE_CONTENTTYPE_ERROR.getErrorCode(),
                response.statusCode)) {
            WXLogUtils.e("user intercept: WX_DEGRAD_ERR_BUNDLE_CONTENTTYPE_ERROR");
            wxErrorCode = WXErrorCode.WX_DEGRAD_ERR_BUNDLE_CONTENTTYPE_ERROR.getErrorCode();
            instance.onRenderError(wxErrorCode,
                    "|response.errorMsg==" + response.errorMsg +
                            "|instance bundleUrl = \n" + instance.getBundleUrl() +
                            "|instance requestUrl = \n" + Uri.decode(WXSDKInstance.requestUrl)
            );
            onFail(response);

            // check content-length
        } else if (response!=null && response.originalData!=null && TextUtils.equals("-206", response.statusCode)) {
            WXLogUtils.e("user intercept: WX_DEGRAD_ERR_NETWORK_CHECK_CONTENT_LENGTH_FAILED");
            wxErrorCode =  WXErrorCode.WX_DEGRAD_ERR_NETWORK_CHECK_CONTENT_LENGTH_FAILED.getErrorCode();
            instance.onRenderError(wxErrorCode ,
                    WXErrorCode.WX_DEGRAD_ERR_NETWORK_CHECK_CONTENT_LENGTH_FAILED.getErrorCode() +
                            "|response.errorMsg==" + response.errorMsg
            );
            onFail(response);
        }
        else {
            wxErrorCode = WXErrorCode.WX_DEGRAD_ERR_NETWORK_BUNDLE_DOWNLOAD_FAILED.getErrorCode();
            instance.onRenderError(wxErrorCode,
                    response.errorMsg);
            onFail(response);
        }
        if (!WXInstanceApm.VALUE_ERROR_CODE_DEFAULT.equals(wxErrorCode)){
            mApmForInstance.addProperty(WXInstanceApm.KEY_PROPERTIES_ERROR_CODE,wxErrorCode);
        }
    }

    private boolean isNet(String requestType){

        return "network".equals(requestType) || "2g".equals(requestType) || "3g".equals(requestType)
                || "4g".equals(requestType) || "wifi".equals(requestType) || "other".equals(requestType)
                || "unknown".equals(requestType);
    }

    public void onSuccess(WXResponse response) {
        if (flag==WXRenderStrategy.DATA_RENDER_BINARY){
            instance.render(pageName, response.originalData, options, jsonInitData);
        }else {
            String template = new String(response.originalData);
            instance.render(pageName, template, options, jsonInitData, flag);
        }
    }

    public void onFail(WXResponse response) {

    }
}


