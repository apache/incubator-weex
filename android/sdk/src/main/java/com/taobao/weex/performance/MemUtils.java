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
package com.taobao.weex.performance;

import android.app.ActivityManager;
import android.content.Context;
import android.os.Build;
import android.os.Debug;
import android.util.Log;

import com.taobao.weex.WXEnvironment;

/**
 * @author chenpeihan
 * @date 2017/12/12
 */

public class MemUtils {

    /**
     * bad performance ,40ms-60ms
     */
    public static Debug.MemoryInfo getMemoryInfo(Context context) {
        long start = System.currentTimeMillis();
        if (null == context){
            return null;
        }
        Debug.MemoryInfo meminfo = null;
        ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        if (activityManager != null) {
            if (Build.VERSION.SDK_INT >= 23) {
                int[] pids = {android.os.Process.myPid()};
                Debug.MemoryInfo[] meminfos = activityManager.getProcessMemoryInfo(pids);
                if (meminfos != null && meminfos.length > 0) {
                    meminfo = meminfos[0];
                }
            } else {
                meminfo = new Debug.MemoryInfo();
                Debug.getMemoryInfo(meminfo);
                int privateDirty = meminfo.getTotalPrivateDirty();
                if (privateDirty == 0) {
                    Debug.getMemoryInfo(meminfo);
                }
            }
        }
        if (WXEnvironment.isApkDebugable()){
            Log.d("MemUtils", "getMemoryInfo , time :"+(System.currentTimeMillis() -start));
        }

        return meminfo;
    }

    public static long getTotalPss(Context context) {

        long totalPss = 0L;

        Debug.MemoryInfo meminfo = getMemoryInfo(context);
        if (meminfo != null) {
            totalPss = meminfo.getTotalPss();
        }
        return totalPss;
    }

}
