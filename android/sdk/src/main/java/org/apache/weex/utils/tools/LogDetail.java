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

import android.text.TextUtils;
import android.util.Log;

import com.alibaba.fastjson.annotation.JSONField;
import org.apache.weex.WXEnvironment;
import java.util.Locale;

public class LogDetail {
    public static final String KeyWrod_Init = "Weex_Init";
    public static final String KeyWords_Render = "Weex_Render";

    public LogDetail() {
        time = new Time();
        info = new Info();
    }

    @JSONField(name = "time")
    public Time time;

    @JSONField(name = "Info")
    public Info info;

    @Override
    public String toString() {
        return
                "taskName : " + info.taskName + " - LogDetail : {" +
                        "time = '" + time + '\'' +
                        ", info = '" + info + '\'' +
                        "}";
    }


    public String keyWords = KeyWords_Render;

    public void println() {
        if(WXEnvironment.isPerf())
        Log.e(TimeCalculator.TIMELINE_TAG," timeline " + this.keyWords + " java LogDetail: " + this.toString());
    }

    public void name(String name) {
        time.constructor();
        info.taskName = name;
        if(!TextUtils.isEmpty(name)) {
            String s = name.toLowerCase(Locale.ROOT);
            if(s.contains("module")
            || s.contains("component")
                    || s.contains("framework")) {
                this.keyWords = KeyWrod_Init;
            }
        }
    }

    public void keyWorkds(String keyWords) {
        this.keyWords = keyWords;
    }

    public void taskStart() {
        time.taskStart();
    }

    public void taskEnd() {
        time.taskEnd();
        println();
    }

}