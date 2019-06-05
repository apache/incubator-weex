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
package com.taobao.weex.utils;

import android.content.Context;
import android.os.Build;

public class WXDeviceUtils {


    public static boolean isAutoResize(Context context){
        if(context == null){
            return false;
        }
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
