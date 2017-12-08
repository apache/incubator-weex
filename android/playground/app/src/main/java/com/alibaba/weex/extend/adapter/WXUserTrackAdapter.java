package com.alibaba.weex.extend.adapter;

import java.io.Serializable;
import java.util.Map;

import com.alibaba.fastjson.JSON;
import com.alibaba.mtl.appmonitor.AppMonitor;
import com.alibaba.mtl.appmonitor.model.DimensionSet;
import com.alibaba.mtl.appmonitor.model.DimensionValueSet;
import com.alibaba.mtl.appmonitor.model.Measure;
import com.alibaba.mtl.appmonitor.model.MeasureSet;
import com.alibaba.mtl.appmonitor.model.MeasureValueSet;

import android.content.Context;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXPerformance;
import com.taobao.weex.utils.WXLogUtils;

public class WXUserTrackAdapter implements IWXUserTrackAdapter {

    private static final String TAG = "UserTrack";
    private static boolean initAppMonitor;

    @Override
    public void commit(Context context, String eventId, String type, WXPerformance perf,
                       Map<String, Serializable> params) {
        try {
            initAppMonitor();
            if (IWXUserTrackAdapter.LOAD.equals(type) && perf != null) {
                Uri pageNameUri = Uri.parse(perf.pageName);
                String scheme = pageNameUri.getScheme();
                String spm = pageNameUri.getQueryParameter("spm");

                perf.pageName = handleUTPageNameScheme(perf.pageName);

                DimensionValueSet dimensionValues = DimensionValueSet.create();
                Map<String, String> dimensionMap = perf.getDimensionMap();

                dimensionMap.put("scheme", scheme);
                dimensionMap.put("spm", spm);

                MeasureValueSet measureValues = MeasureValueSet.create();

                StringBuilder builder = new StringBuilder("维度埋点数据:");

                if (params != null) {
                    try {
                        String paramsStr = JSON.toJSONString(params);
                        dimensionMap.put("customMonitorInfo", paramsStr);
                    } catch (Throwable throwable) {
                        Log.w(TAG, "unexpected throws", throwable);
                    }

                }

                for (String key : dimensionMap.keySet()) {
                    dimensionValues.setValue(key, dimensionMap.get(key));

                    if (WXEnvironment.isApkDebugable()) {
                        builder.append(key);
                        builder.append(":");
                        builder.append(dimensionMap.get(key));
                        builder.append("||");
                    }
                }

                builder.append("指标埋点数据:");

                Map<String, Double> measureMap = perf.getMeasureMap();
                if (measureMap != null) {
                    for (String key : measureMap.keySet()) {
                        measureValues.setValue(key, measureMap.get(key));

                        if (WXEnvironment.isApkDebugable()) {
                            builder.append(key);
                            builder.append(":");
                            builder.append(measureMap.get(key));
                            builder.append("||");
                        }
                    }
                }
                AppMonitor.Stat.commit(IWXUserTrackAdapter.MODULE_NAME, IWXUserTrackAdapter.LOAD, dimensionValues,
                    measureValues);

                if (WXEnvironment.isApkDebugable()) {
                    WXLogUtils.d(TAG, builder.toString());
                }

            } else if ((IWXUserTrackAdapter.DOM_MODULE.equals(type)
                || IWXUserTrackAdapter.JS_BRIDGE.equals(type)
                || WXEnvironment.ENVIRONMENT.equals(type)
                || IWXUserTrackAdapter.STREAM_MODULE.equals(type)
                || IWXUserTrackAdapter.JS_FRAMEWORK.equals(type)
                || IWXUserTrackAdapter.JS_DOWNLOAD.equals(type))
                && perf != null) {
                if (!WXErrorCode.WX_SUCCESS.getErrorCode().equals(perf.errCode)) {
                    AppMonitor.Alarm.commitFail(IWXUserTrackAdapter.MODULE_NAME, type, perf.args, perf.errCode,
                        perf.getErrMsg());
                } else {
                    AppMonitor.Alarm.commitSuccess(IWXUserTrackAdapter.MODULE_NAME, type, perf.args);
                }
            } else if ("invokeModule".equals(type)) {
                //TODO : 未免 weex sdk 未升级时此逻辑异常,临时使用 "invokeModule",后续改为 IWXUserTrackAdapter.INVOKE_MODULE
                String arg = (String)params.get("arg");
                String errMsg = (String)params.get("errMsg");
                String errCode = (String)params.get("errCode");
                AppMonitor.Alarm.commitFail(IWXUserTrackAdapter.MODULE_NAME, type, arg, errCode, errMsg);
            }
        } catch (Throwable e) {
            e.printStackTrace();
        }

    }

    private void initAppMonitor() {
        if (initAppMonitor) {
            return;
        }
        // 定义维度集合
        DimensionSet dimensionSet = DimensionSet.create();

        String[] dimensions = WXPerformance.getDimensions();
        StringBuilder builder = new StringBuilder("维度集:");

        for (String dimension : dimensions) {
            dimensionSet.addDimension(dimension);

            if (WXEnvironment.isApkDebugable()) {
                builder.append(dimension);
                builder.append("||");
            }
        }

        builder.append("指标集:");
        // 定义指标集合
        MeasureSet measureSet = MeasureSet.create();
        String[] measures = WXPerformance.getMeasures();
        for (String measure : measures) {

            measureSet.addMeasure(new Measure(measure));

            if (WXEnvironment.isApkDebugable()) {
                builder.append(measure);
            }
        }

        if (WXEnvironment.isApkDebugable()) {
            WXLogUtils.d(TAG, builder.toString());
        }

        AppMonitor.register(IWXUserTrackAdapter.MODULE_NAME, IWXUserTrackAdapter.LOAD, measureSet, dimensionSet);
        initAppMonitor = true;
    }

    private String handleUTPageNameScheme(String pageName) {

        try {
            if (TextUtils.isEmpty(pageName)) {
                pageName = WXPerformance.DEFAULT;
                return pageName;
            }

            Uri pageNameUri = Uri.parse(pageName);
            Uri.Builder pageNameBuilder = pageNameUri.buildUpon();

            pageNameBuilder.scheme("");
            pageName = pageNameBuilder.toString().
                replaceFirst("^(/|://|:/|//)", "");

        } catch (Exception e) {
            WXLogUtils.e("pageNameError", e);
        }
        return pageName;
    }
}