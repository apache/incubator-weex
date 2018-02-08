package com.alibaba.weex.extend.module;

import android.util.Log;

import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.bridge.JSCallback;
import com.taobao.weex.common.WXModule;
import com.taobao.weex.wson.Wson;

/**
 * Created by furture on 2018/1/18.
 */

public class WXWsonTestModule extends WXModule {

    @JSMethod(uiThread = false)
    public boolean callPass(JSONObject params) {
        if(params.get("refundId") != null){
            if(params.get("refundId").getClass() != Long.class){
                Log.e("Weex", "weex wson refundId failed case");
                return false;
            }
            return  ((Long)params.get("refundId")) == 6419458776149741l;
        }

        if(params.get("refundId2") != null){
            if(params.get("refundId2").getClass() != Long.class){
                Log.e("Weex", "weex wson refundId failed case");
                return false;
            }
            return ((Long)params.get("refundId2")) == 64194587761497416l;
        }




        return true;
    }

    @JSMethod(uiThread = false)
    public Object back(JSONObject params) {
        JSONObject back = new JSONObject(params);
        back.put("longMax", Long.MAX_VALUE);
        back.put("longMin", Long.MIN_VALUE);
        back.put("javaJSON", back.toJSONString());
        return back;
    }

    @JSMethod(uiThread = false)
    public void  backAsync(JSONObject params, JSCallback callback) {
        JSONObject back = new JSONObject(params);
        back.put("longMax", Long.MAX_VALUE);
        back.put("longMin", Long.MIN_VALUE);
        back.put("javaJSON", back.toJSONString());
        callback.invoke(back);
    }
}
