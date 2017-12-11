package com.alibaba.weex.extend;

import android.app.Application;
import android.content.pm.PackageInfo;

import com.alibaba.ha.adapter.AliHaAdapter;
import com.alibaba.ha.adapter.AliHaConfig;

/**
 * @author chenpeihan
 * @date 2017/12/8
 */

public class AliHaIniter {
    public void init(Application application) {
        String appKey = "24643779";
        String channel = "60000";
        String userNick = "fengchao";
        String appSecret = "8951ae070be6560f4fc1401e90a83a4e";
        String appVersion = getAppVersionName(application);


        //ha初始化
        AliHaConfig config = new AliHaConfig();
        config.isAliyunos = false;
        config.appKey = appKey;
        config.userNick = userNick;
        config.channel = channel;
        config.appVersion = appVersion;
        config.appSecret = appSecret;
        config.application = application;
        config.context = application.getApplicationContext();
        AliHaAdapter.getInstance().start(config);
    }

    private String getAppVersionName(Application application) {
        String versionName = "";
        try {
            PackageInfo info = application.getPackageManager().getPackageInfo(application.getPackageName(), 0);
            versionName = info.versionName;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return versionName;
    }
}
