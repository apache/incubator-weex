package com.alibaba.weex.extend.module;

import com.taobao.weex.annotation.JSMethod;
import com.taobao.weex.common.WXModule;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by zhengshihan on 2016/12/28.
 */

public class SyncTestModule extends WXModule {

//    @JSMethod(uiThread = false)
//    public Object getString(){
//        ArrayList list = new ArrayList();
//        list.add(1,"222");
//        list.add("test");
//        return list;
//    }
    @JSMethod(uiThread = false)
    public String  getString(){

        return "i am string ";
    }


    @JSMethod(uiThread = false)
    public int getNumber(){
        return 1111;
    }

    @JSMethod(uiThread = false)
    public Object getObject(){

        Map map = new HashMap();
        map.put(11,"test11");
        map.put(22,"test22");
        return map;
    }



}
