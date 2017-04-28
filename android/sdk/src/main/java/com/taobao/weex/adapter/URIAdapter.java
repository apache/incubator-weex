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

import android.net.Uri;
import android.support.annotation.NonNull;

import com.taobao.weex.WXSDKInstance;

/**
 * Created by sospartan on 15/11/2016.
 */

public interface URIAdapter {

  /**
   * Stream request url.
   */
  String REQUEST = "request";

  /**
   * URI for image src.
   */
  String IMAGE = "image";

  /**
   * Font file URI for text @font-face .
   */
  String FONT = "font";

  /**
   * Video URI.
   */
  String VIDEO = "video";

  /**
   * URI for a 'href' attribute.
   */
  String LINK = "link";

  /**
   * Bundle URI for Weex instance.
   */
  String BUNDLE = "bundle";

  /**
   * Web page src
   */
  String WEB = "web";

  /**
   * Unknown URIs.
   */
  String OTHERS = "others";

  /**
   *
   * @param type URI type, see {@link #IMAGE}/{@link #LINK}/{@link #FONT}/{@link #BUNDLE}/{@link #VIDEO}/{@link #OTHERS}
   * @param uri
   * @return
   */
  @NonNull Uri rewrite(WXSDKInstance instance, String type, Uri uri);
}
