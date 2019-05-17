package com.taobao.weex.utils;

import android.content.Context;
import android.os.Build;

public class WXDeviceUtils {


    public static boolean isAutoResize(Context context){
        return isMateX(context) || isGalaxyFold(context);
    }

    /**
     * Mate X
     * */
    public static boolean isMateX(Context context) {
        return "HUAWEI".equalsIgnoreCase(Build.BRAND) && ("unknownRLI".equalsIgnoreCase(Build.DEVICE) || ("HWTAH".equalsIgnoreCase(Build.DEVICE)));
    }


    /**
     * Galaxy Fopld
     * */
    public static boolean isGalaxyFold(Context context) {

        if("samsung".equalsIgnoreCase(Build.BRAND) && "SM-F9000".equalsIgnoreCase(Build.MODEL)) {
            return true;
        }

        return false;
    }

}
