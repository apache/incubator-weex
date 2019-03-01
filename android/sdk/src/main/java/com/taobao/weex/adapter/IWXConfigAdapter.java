package com.taobao.weex.adapter;

/**
 * @author zhongcang
 * @date 2019/3/20
 */
public interface IWXConfigAdapter {
    String getConfig(String nameSpace,String key,String defaultValue);
}
