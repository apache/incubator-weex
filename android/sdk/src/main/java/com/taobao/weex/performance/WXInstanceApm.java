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

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import android.text.TextUtils;
import android.util.Log;
import com.taobao.weex.BuildConfig;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.WXSDKManager;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.utils.WXExceptionUtils;
import com.taobao.weex.utils.WXUtils;

public class WXInstanceApm {

    public static final String WEEX_PAGE_TOPIC = "weex_page";

    /************** properties *****************/
    public static final String KEY_PROPERTIES_ERROR_CODE = "wxErrorCode";
    //public static final String KEY_PAGE_PROPERTIES_LAUNCH_ID = "wxLaunchId";
    public static final String KEY_PAGE_PROPERTIES_BIZ_ID = "wxBizID";
    public static final String KEY_PAGE_PROPERTIES_BUBDLE_URL = "wxBundleUrl";
    public static final String KEY_PAGE_PROPERTIES_JSLIB_VERSION = "wxJSLibVersion";
    public static final String KEY_PAGE_PROPERTIES_WEEX_VERSION = "wxSDKVersion";
    public static final String KEY_PAGE_PROPERTIES_REQUEST_TYPE = "wxRequestType";
    public static final String KEY_PAGE_PROPERTIES_CACHE_TYPE = "wxCacheType";
    public static final String KEY_PAGE_PROPERTIES_CACHE_INFO = "wxZCacheInfo";
    public static final String KEY_PAGE_PROPERTIES_JS_FM_INI = "wxJsFrameworkInit";
    public static final String KEY_PAGE_PROPERTIES_CONTAINER_NAME = "wxContainerName";
    public static final String KEY_PAGE_PROPERTIES_INSTANCE_TYPE = "wxInstanceType";
    public static final String KEY_PAGE_PROPERTIES_PARENT_PAGE = "wxParentPage";
    public static final String KEY_PAGE_PROPERTIES_BUNDLE_TYPE = "wxBundleType";

    /************** stages *****************/
    public static final String KEY_PAGE_STAGES_DOWN_BUNDLE_START = "wxStartDownLoadBundle";
    public static final String KEY_PAGE_STAGES_DOWN_BUNDLE_END = "wxEndDownLoadBundle";
    public static final String KEY_PAGE_STAGES_RENDER_ORGIGIN = "wxRenderTimeOrigin";
    public static final String KEY_PAGE_STAGES_LOAD_BUNDLE_START = "wxStartLoadBundle";
    public static final String KEY_PAGE_STAGES_LOAD_BUNDLE_END = "wxEndLoadBundle";
    public static final String KEY_PAGE_STAGES_FIRST_INTERACTION_VIEW = "wxFirstInteractionView";
    public static final String KEY_PAGE_STAGES_CREATE_FINISH= "wxJSBundleCreateFinish";
    public static final String KEY_PAGE_STAGES_FSRENDER = "wxFsRender";
    public static final String KEY_PAGE_STAGES_NEW_FSRENDER = "wxNewFsRender";
    public static final String KEY_PAGE_STAGES_INTERACTION = "wxInteraction";
    public static final String KEY_PAGE_STAGES_DESTROY = "wxDestroy";

    /************** stats *****************/
    public static final String KEY_PAGE_STATS_BUNDLE_SIZE = "wxBundleSize";
    public static final String KEY_PAGE_STATS_FS_CALL_JS_TIME = "wxFSCallJsTotalTime";
    public static final String KEY_PAGE_STATS_FS_CALL_JS_NUM = "wxFSCallJsTotalNum";
    public static final String KEY_PAGE_STATS_FS_TIMER_NUM = "wxFSTimerCount";
    public static final String KEY_PAGE_STATS_FS_CALL_NATIVE_TIME = "wxFSCallNativeTotalTime";
    public static final String KEY_PAGE_STATS_FS_CALL_NATIVE_NUM = "wxFSCallNativeTotalNum";
    public static final String KEY_PAGE_STATS_FS_CALL_EVENT_NUM = "wxFSCallEventTotalNum";
    public static final String KEY_PAGE_STATS_FS_REQUEST_NUM = "wxFSRequestNum";
    public static final String KEY_PAGE_STATS_CELL_EXCEED_NUM = "wxCellExceedNum";
    public static final String KEY_PAGE_STATS_MAX_DEEP_VIEW = "wxMaxDeepViewLayer";
    public static final String KEY_PAGE_STATS_MAX_DEEP_DOM = "wxMaxDeepVDomLayer";
    public static final String KEY_PAGE_STATS_MAX_COMPONENT_NUM = "wxMaxComponentCount";
    public static final String KEY_PAGE_STATS_WRONG_IMG_SIZE_COUNT = "wxWrongImgSizeCount";
    public static final String KEY_PAGE_STATS_EMBED_COUNT = "wxEmbedCount";
    public static final String KEY_PAGE_STATS_LARGE_IMG_COUNT = "wxLargeImgMaxCount";
    public static final String KEY_PAGE_STATS_BODY_RATIO = "wxBodyRatio";
    public static final String KEY_PAGE_STATS_SCROLLER_NUM = "wxScrollerCount";
    public static final String KEY_PAGE_STATS_CELL_DATA_UN_RECYCLE_NUM = "wxCellDataUnRecycleCount";
    public static final String KEY_PAGE_STATS_CELL_UN_RE_USE_NUM = "wxCellUnReUseCount";
    public static final String KEY_PAGE_STATS_IMG_UN_RECYCLE_NUM = "wxImgUnRecycleCount";

    public static final String KEY_PAGE_STATS_I_SCREEN_VIEW_COUNT = "wxInteractionScreenViewCount";
    public static final String KEY_PAGE_STATS_I_ALL_VIEW_COUNT = "wxInteractionAllViewCount";
    public static final String KEY_PAGE_STATS_I_COMPONENT_CREATE_COUNT = "wxInteractionComponentCreateCount";
    public static final String KEY_PAGE_ANIM_BACK_NUM = "wxAnimationInBackCount";
    public static final String KEY_PAGE_TIMER_BACK_NUM = "wxTimerInBackCount";
    public static final String KEY_PAGE_STATS_ACTUAL_DOWNLOAD_TIME = "wxActualNetworkTime";

    public static final String KEY_PAGE_STATS_IMG_LOAD_NUM = "wxImgLoadCount";
    public static final String KEY_PAGE_STATS_IMG_LOAD_SUCCESS_NUM = "wxImgLoadSuccessCount";
    public static final String KEY_PAGE_STATS_IMG_LOAD_FAIL_NUM = "wxImgLoadFailCount";
    public static final String KEY_PAGE_STATS_NET_NUM = "wxNetworkRequestCount";
    public static final String KEY_PAGE_STATS_NET_SUCCESS_NUM = "wxNetworkRequestSuccessCount";
    public static final String KEY_PAGE_STATS_NET_FAIL_NUM = "wxNetworkRequestFailCount";
    public static final String KEY_PAGE_STATS_JSLIB_INIT_TIME = "wxJSLibInitTime";

    /************** value *****************/
    public static final String VALUE_ERROR_CODE_DEFAULT = "0";

    private String mInstanceId;
    private IWXApmMonitorAdapter apmInstance;
    private Map<String, Double> recordStatsMap;
    private boolean isFSEnd;
    private boolean mHasInit = false;
    private boolean mEnd = false;
    private boolean hasRecordFistInteractionView =false;
    public final Map<String,Object> extInfo;

    public WXInstanceApm(String instanceId) {
        mInstanceId = instanceId;
        extInfo = new ConcurrentHashMap<>();
        IApmGenerator generator = WXSDKManager.getInstance().getApmGenerater();
        if (null != generator) {
            apmInstance = generator.generateApmInstance(WEEX_PAGE_TOPIC);
            recordStatsMap = new ConcurrentHashMap<>();
        }
    }

    /**
     * record event
     */
    public void onEvent(String name, Object value) {
        if (null == apmInstance) {
            return;
        }
        apmInstance.onEvent(name, value);
    }

    /**
     * record stage
     */
    public void onStage(String name) {
        long time =  WXUtils.getFixUnixTime();
        onStageWithTime(name,time);
    }

    /**
     *
     * @param name stage
     * @param time unixTime ,plz use WXUtils.getFixUnixTime
     */
    public void onStageWithTime(String name,long time){
        WXSDKInstance instance = WXSDKManager.getInstance().getAllInstanceMap().get(mInstanceId);
        if (null != instance){
            instance.getExceptionRecorder().recordStage(name, time);
        }
        if (null == apmInstance || mEnd) {
            return;
        }
        apmInstance.onStage(name, time);
    }


    /**
     * record property
     */
    public void addProperty(String key, Object value) {
        if (null == apmInstance || mEnd) {
            return;
        }
        apmInstance.addProperty(key, value);
    }

    /**
     * record statistic
     */
    public void addStats(String key, double value) {
        if (null == apmInstance || mEnd) {
            return;
        }
        apmInstance.addStats(key, value);
    }


    public boolean hasInit(){
        return mHasInit;
    }

    /**
     * start record
     */
    public void doInit() {
        if (mHasInit){
            return;
        }
        mHasInit = true;
        if (null == apmInstance) {
            return;
        }
        apmInstance.onStart(mInstanceId);
        WXSDKInstance instance = WXSDKManager.getInstance().getAllInstanceMap().get(mInstanceId);
        String url = null == instance ? "unKnowUrl" : instance.getBundleUrl();
        addProperty(KEY_PAGE_PROPERTIES_BUBDLE_URL, url);
        addProperty(KEY_PROPERTIES_ERROR_CODE, VALUE_ERROR_CODE_DEFAULT);
        addProperty(KEY_PAGE_PROPERTIES_JSLIB_VERSION, WXEnvironment.JS_LIB_SDK_VERSION);
        addProperty(KEY_PAGE_PROPERTIES_WEEX_VERSION, WXEnvironment.WXSDK_VERSION);
        if (null != instance) {
            for (Map.Entry<String, String> entry : instance.getContainerInfo().entrySet()) {
                addProperty(entry.getKey(), entry.getValue());
            }
        }
    }

    public void setPageName(String pageName) {
        if (TextUtils.isEmpty(pageName)) {
            WXSDKInstance instance = WXSDKManager.getInstance().getAllInstanceMap().get(mInstanceId);
            if (null != instance) {
                pageName = instance.getContainerInfo().get(KEY_PAGE_PROPERTIES_CONTAINER_NAME);
            }
        }
        String fixPageName = TextUtils.isEmpty(pageName) ? "emptyPageName" : pageName;
        addProperty(KEY_PAGE_PROPERTIES_BIZ_ID, fixPageName);
    }

    public void onAppear(){
        if (null == apmInstance) {
            return;
        }
        apmInstance.onAppear();
    }

    public void onDisAppear(){
        if (null == apmInstance) {
            return;
        }
        apmInstance.onDisappear();
    }

    /**
     * end record
     */
    public void onEnd() {
        if (null == apmInstance || mEnd) {
            return;
        }
        onStage(KEY_PAGE_STAGES_DESTROY);
        apmInstance.onEnd();
        mEnd = true;
    }

    public void arriveNewFsRenderTime(){
        if (null == apmInstance){
            return;
        }
        onStage(WXInstanceApm.KEY_PAGE_STAGES_NEW_FSRENDER);
    }

    public void arriveFSRenderTime() {
        if (null == apmInstance){
            return;
        }
        isFSEnd = true;
        onStage(WXInstanceApm.KEY_PAGE_STAGES_FSRENDER);
    }

    public void arriveInteraction(WXComponent targetComponent) {
        if (null == apmInstance || null == targetComponent || targetComponent.getInstance() == null ) {
            return;
        }
        WXPerformance performanceRecord = targetComponent.getInstance().getWXPerformance();
        if (null == performanceRecord){
            return;
        }

        if (!hasRecordFistInteractionView){
            onStage(KEY_PAGE_STAGES_FIRST_INTERACTION_VIEW);
            hasRecordFistInteractionView = true;
        }

        long curTime = WXUtils.getFixUnixTime();

        if (BuildConfig.DEBUG){
            Log.d("wxapm", "screenComponent ["+targetComponent.getComponentType()+","+targetComponent.getRef()
                +"], renderTime:"+ (curTime -performanceRecord.renderUnixTimeOrigin)
                +",style:"+targetComponent.getStyles()
                +",attrs:"+targetComponent.getAttrs());
        }

        performanceRecord.interactionTime = curTime - performanceRecord.renderUnixTimeOrigin;
        onStageWithTime(KEY_PAGE_STAGES_INTERACTION,curTime);

        updateDiffStats(KEY_PAGE_STATS_I_SCREEN_VIEW_COUNT, 1);
        updateMaxStats(KEY_PAGE_STATS_I_ALL_VIEW_COUNT, performanceRecord.localInteractionViewAddCount);
        WXSDKInstance instance = WXSDKManager.getInstance().getSDKInstance(mInstanceId);
        if (null != instance) {
            updateMaxStats(KEY_PAGE_STATS_I_COMPONENT_CREATE_COUNT, instance.getWXPerformance().componentCount);
        }
    }

    public void updateFSDiffStats(String name, double diffValue) {
        if (null == apmInstance || isFSEnd) {
            return;
        }
        updateDiffStats(name, diffValue);
    }

    public void updateDiffStats(String name, double diffValue) {
        if (null == apmInstance) {
            return;
        }
        Double preVal = recordStatsMap.containsKey(name) ? recordStatsMap.get(name) : 0;
        //fix by use ConcurrentHashMap,but not sure,so report if error still happen
        if (null == preVal){
            WXExceptionUtils.commitCriticalExceptionRT(
                "",
                WXErrorCode.WX_ERR_HASH_MAP_TMP,
                "updateDiffStats",
                "key : "+name,
                null
            );
            return;
        }

        double currentValue = preVal + diffValue;
        recordStatsMap.put(name, currentValue);
        addStats(name, currentValue);
    }

    public void updateMaxStats(String name, double currentVal) {
        if (null == apmInstance) {
            return;
        }
        Double maxValue = recordStatsMap.containsKey(name) ? recordStatsMap.get(name) : 0;
        //fix by use ConcurrentHashMap,but not sure,so report if error still happen
        if (null == maxValue){
            WXExceptionUtils.commitCriticalExceptionRT(
                "",
                WXErrorCode.WX_ERR_HASH_MAP_TMP,
                "updateMaxStats",
                "key : "+name,
                null
            );
            return;
        }
        if (maxValue < currentVal) {
            maxValue = currentVal;
            recordStatsMap.put(name, currentVal);
            addStats(name, maxValue);
        }
    }

    public void updateRecordInfo(Map<String, Object> extParams) {
        if (null == apmInstance || null == extParams) {
            return;
        }

        addPropeyFromExtParms(KEY_PAGE_PROPERTIES_REQUEST_TYPE, KEY_PAGE_PROPERTIES_REQUEST_TYPE, extParams);
        addPropeyFromExtParms("cacheType", KEY_PAGE_PROPERTIES_CACHE_TYPE, extParams);
        addPropeyFromExtParms("zCacheInfo", KEY_PAGE_PROPERTIES_CACHE_INFO, extParams);

        addStats(KEY_PAGE_STATS_JSLIB_INIT_TIME, WXEnvironment.sJSLibInitTime);
        addProperty(KEY_PAGE_PROPERTIES_JS_FM_INI, WXEnvironment.JsFrameworkInit);

        Object wxNetLibDownBundleTime = extParams.get("actualNetworkTime");
        if (wxNetLibDownBundleTime instanceof Long) {
            updateDiffStats(KEY_PAGE_STATS_ACTUAL_DOWNLOAD_TIME, ((Long)wxNetLibDownBundleTime).doubleValue());
        }
    }

    private void addPropeyFromExtParms(String fromKey, String toKey, Map<String, Object> extParams) {
        Object value = extParams.get(fromKey);
        if (value instanceof String) {
            addProperty(toKey, value);
        }
    }

    /************** called by IWXHttpAdapter implementer *****************/

    public void actionNetRequest() {
        if (!isFSEnd) {
            updateFSDiffStats(WXInstanceApm.KEY_PAGE_STATS_FS_REQUEST_NUM, 1);
        }
        updateDiffStats(WXInstanceApm.KEY_PAGE_STATS_NET_NUM, 1);
    }

    public void actionNetResult(boolean succeed, String errorCode) {
        if (succeed) {
            updateDiffStats(WXInstanceApm.KEY_PAGE_STATS_NET_SUCCESS_NUM, 1);
        } else {
            updateDiffStats(WXInstanceApm.KEY_PAGE_STATS_NET_FAIL_NUM, 1);
        }
    }

    /************** called by IWXImgLoaderAdapter implementer *****************/

    public void actionLoadImg() {
        updateDiffStats(WXInstanceApm.KEY_PAGE_STATS_IMG_LOAD_NUM, 1);
    }

    public void actionLoadImgResult(boolean succeed, String errorCode) {
        if (succeed) {
            updateDiffStats(WXInstanceApm.KEY_PAGE_STATS_IMG_LOAD_SUCCESS_NUM, 1);
        } else {
            updateDiffStats(WXInstanceApm.KEY_PAGE_STATS_IMG_LOAD_FAIL_NUM, 1);
        }
    }
}
