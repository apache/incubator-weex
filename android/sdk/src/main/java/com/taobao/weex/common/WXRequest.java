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
package com.taobao.weex.common;

import com.taobao.weex.WXSDKInstance;

import java.util.Map;

public class WXRequest {

  /**
   * The request parameter
   */
  public Map<String, String> paramMap;

  /**
   * The request URL
   */
  public String url;
  /**
   * The request method
   */
  public String method;
  /**
   * The request body
   */
  public String body;

  /**
   * The request time out
   */
  public int timeoutMs = WXRequest.DEFAULT_TIMEOUT_MS;

  /**
   * The default timeout
   */
  public static final int DEFAULT_TIMEOUT_MS = 3000;

  /**
   * running weex instanceId
   * @see WXSDKInstance#mInstanceId
   */
  public String instanceId;;
}
