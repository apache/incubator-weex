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
package com.taobao.weex.appfram.websocket;

/**
 * Created by moxun on 17/1/3.
 * @see <a href="https://developer.mozilla.org/en-US/docs/Web/API/CloseEvent">CloseEvent</a>
 */

public enum WebSocketCloseCodes {
    CLOSE_NORMAL(1000),
    CLOSE_GOING_AWAY(1001),
    CLOSE_PROTOCOL_ERROR(1002),
    CLOSE_UNSUPPORTED(1003),
    CLOSE_NO_STATUS(1005),
    CLOSE_ABNORMAL(1006),
    UNSUPPORTED_DATA(1007),
    POLICY_VIOLATION(1008),
    CLOSE_TOO_LARGE(1009),
    MISSING_EXTENSION(1010),
    INTERNAL_ERROR(1011),
    SERVICE_RESTART(1012),
    TRY_AGAIN_LATER(1013),
    TLS_HANDSHAKE(1015);

    private int code;

    WebSocketCloseCodes(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }
}
