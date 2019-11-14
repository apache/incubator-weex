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
package org.apache.weex.utils.tools;

import android.util.Log;

import com.alibaba.fastjson.JSON;
import org.apache.weex.WXEnvironment;
import org.apache.weex.WXSDKInstance;

import java.util.concurrent.CopyOnWriteArrayList;

public class TimeCalculator {

    public static final String TIMELINE_TAG = "timeline";

    public static final String PLATFORM_ANDROID = "Android";

    private CopyOnWriteArrayList<LogDetail> logRecorder;
    public TimeCalculator(WXSDKInstance instance) {
        logRecorder = new CopyOnWriteArrayList<LogDetail>();
    }

    public void addLog(LogDetail logDetail) {
        if(!WXEnvironment.isPerf()) {
            return;
        }
        logRecorder.add(logDetail);
    }

    public LogDetail createLogDetail(String name) {
        LogDetail logDetail = new LogDetail();
        logDetail.name(name);
        addLog(logDetail);
        return logDetail;
    }

    public void println() {
        if(!WXEnvironment.isPerf()) {
            return;
        }

        for (LogDetail l: logRecorder) {
            Log.e(TIMELINE_TAG, JSON.toJSONString(l));
        }

    }


    private String test = "{\"time\":{\"execTime\":0,\"constructor\":0,\"destructor\":0,\"taskStart\":0,\"taskEnd\":0,\"waitTime\":0},\"Info\":{\"taskInfo\":{\"relateTaskId\":0,\"args\":\"stringReplace\"},\"taskName\":\"stringReplace\",\"instanceId\":\"1\",\"platform\":\"stringReplace\",\"taskId\":0}}";
}
