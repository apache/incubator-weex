/*
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
package com.taobao.weex.adapter;

import android.content.Context;

import com.taobao.weex.common.WXPerformance;

import java.io.Serializable;
import java.util.Map;

/**
 * Interface for commit log info. This interface works as an adapter for various log library.
 */
public interface IWXUserTrackAdapter {

  String MODULE_NAME = "weex";

  //Performance
  String LOAD = "load";

  //Alarm
  String JS_FRAMEWORK = "jsFramework";
  String JS_DOWNLOAD = "jsDownload";
  String DOM_MODULE = "domModule";
  String JS_BRIDGE = "jsBridge";
  String STREAM_MODULE = "streamModule";
  String INVOKE_MODULE = "invokeModule";
  String INIT_FRAMEWORK = "initFramework";

  /**
   * monitor keys
   */
  String MONITOR_ERROR_CODE = "errCode";
  String MONITOR_ARG = "arg";
  String MONITOR_ERROR_MSG = "errMsg";

  void commit(Context context, String eventId, String type, WXPerformance perf, Map<String, Serializable> params);
}
