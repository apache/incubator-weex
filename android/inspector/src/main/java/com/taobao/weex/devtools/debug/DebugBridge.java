package com.taobao.weex.devtools.debug;

import android.util.Log;

import com.alibaba.fastjson.JSON;
import com.taobao.weex.bridge.WXBridgeManager;
import com.taobao.weex.bridge.WXJSObject;
import com.taobao.weex.bridge.WXParams;
import com.taobao.weex.common.IWXBridge;
import com.taobao.weex.devtools.inspector.jsonrpc.JsonRpcPeer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by budao on 16/6/25.
 */
public class DebugBridge implements IWXBridge {
    private static final String TAG = "DebugBridge";
    private static volatile DebugBridge sInstance;
    private WXBridgeManager mJsManager;
    private JsonRpcPeer mPeer;

    private DebugBridge() {

    }

    public static DebugBridge getInstance() {
        if (sInstance == null) {
            synchronized (DebugBridge.class) {
                if (sInstance == null) {
                    sInstance = new DebugBridge();
                }
            }
        }

        return sInstance;
    }

    public void setPeer(JsonRpcPeer peer) {
        mPeer = peer;
    }

    public void setBridgeManager(WXBridgeManager bridgeManager) {
        mJsManager = bridgeManager;
    }

    @Override
    public int initFramework(String framework, WXParams params) {
//            if (!mInit) {
//                return -1;
//            }

        Map<String, Object> func = new HashMap<>();
        func.put("source", framework);

        Map<String, Object> map = new HashMap<>();
        map.put("method", "WxDebug.initJSRuntime");
        map.put("params", func);
        Log.v(TAG, JSON.toJSONString(map));
        if (mPeer != null) {
            mPeer.getWebSocket().sendText(JSON.toJSONString(map));
            return 1;
        }
        return 0;
    }

    @Override
    public int execJS(String instanceId, String namespace, String function, WXJSObject[] args) {
        //        if (!mInit || (TextUtils.isEmpty(instanceId) && !WXBridgeManager.METHOD_REGISTER_MODULES.equals(function))
//                || TextUtils.isEmpty(function)) {
//            return -1;
//        }

        ArrayList<Object> array = new ArrayList<>();
        int argsCount = args == null ? 0 : args.length;
        for (int i = 0; i < argsCount; i++) {
            if (args[i].type != WXJSObject.String) {
                array.add(JSON.parse(args[i].data.toString()));
            } else {
                array.add(args[i].data);
            }
        }

        Map<String, Object> func = new HashMap<>();
        func.put("method", function);
        func.put("args", array);

        Log.v(TAG, "callJS: function is " + function + ", args " + array);
        Map<String, Object> map = new HashMap<>();
        map.put("method", "WxDebug.callJS");
        map.put("params", func);
        if (mPeer != null) {
            mPeer.getWebSocket().sendText(JSON.toJSONString(map));
        }

        return 0;
    }

    @Override
    public void callNative(String instanceId, String tasks, String callback) {
//            if (!mInit || mJsManager == null)
//                return;
        if (mJsManager != null) {
            mJsManager.callNative(instanceId, tasks, callback);
        }
    }

    @Override
    public void reportJSException(String instanceId, String func, String exception) {
        if (mJsManager != null) {
            mJsManager.reportJSException(instanceId, func, exception);
        }
    }
}
