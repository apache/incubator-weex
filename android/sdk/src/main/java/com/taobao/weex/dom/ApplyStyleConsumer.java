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
package com.taobao.weex.dom;

import java.util.Map;

/**
 * Created by sospartan on 22/02/2017.
 */
class ApplyStyleConsumer implements WXDomObject.Consumer {
  static ApplyStyleConsumer sInstance;

  public static ApplyStyleConsumer getInstance() {
    if (sInstance == null) {
      sInstance = new ApplyStyleConsumer();
    }
    return sInstance;
  }

  private ApplyStyleConsumer() {
  }

  ;

  @Override
  public void accept(WXDomObject dom) {
    WXStyle style = dom.getStyles();

    /** merge default styles **/
    Map<String, String> defaults = dom.getDefaultStyle();
    if (defaults != null) {
      for (Map.Entry<String, String> entry : defaults.entrySet()) {
        if (!style.containsKey(entry.getKey())) {
          style.put(entry.getKey(), entry.getValue());
        }
      }
    }

    if (dom.getStyles().size() > 0) {
      dom.applyStyleToNode();
    }
  }
}
