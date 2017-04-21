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
package com.alibaba.weex.commons.adapter;

import android.support.annotation.Nullable;

import com.squareup.okhttp.OkHttpClient;
import com.squareup.okhttp.Request;
import com.squareup.okhttp.Response;
import com.squareup.okhttp.ws.WebSocket;
import com.squareup.okhttp.ws.WebSocketCall;
import com.squareup.okhttp.ws.WebSocketListener;
import com.taobao.weex.appfram.websocket.IWebSocketAdapter;
import com.taobao.weex.appfram.websocket.WebSocketCloseCodes;

import java.io.EOFException;
import java.io.IOException;

import okio.Buffer;
import okio.BufferedSource;

/**
 * Created by moxun on 16/12/27.
 */

public class DefaultWebSocketAdapter implements IWebSocketAdapter {

    private WebSocket ws;
    private EventListener eventListener;

    @Override
    public void connect(String url, @Nullable String protocol, EventListener listener) {
        this.eventListener = listener;
        OkHttpClient okHttpClient = new OkHttpClient();

        Request.Builder builder = new Request.Builder();

        if (protocol != null) {
            builder.addHeader(HEADER_SEC_WEBSOCKET_PROTOCOL, protocol);
        }

        builder.url(url);

        WebSocketCall.create(okHttpClient, builder.build()).enqueue(new WebSocketListener() {
            @Override
            public void onOpen(WebSocket webSocket, Request request, Response response) throws IOException {
                ws = webSocket;
                eventListener.onOpen();
            }

            @Override
            public void onMessage(BufferedSource payload, WebSocket.PayloadType type) throws IOException {
                eventListener.onMessage(payload.readUtf8());
                payload.close();
            }

            @Override
            public void onPong(Buffer payload) {

            }

            @Override
            public void onClose(int code, String reason) {
                eventListener.onClose(code, reason, true);
            }

            @Override
            public void onFailure(IOException e) {
                e.printStackTrace();
                if (e instanceof EOFException) {
                    eventListener.onClose(WebSocketCloseCodes.CLOSE_NORMAL.getCode(), WebSocketCloseCodes.CLOSE_NORMAL.name(), true);
                } else {
                    eventListener.onError(e.getMessage());
                }
            }
        });
    }

    @Override
    public void send(String data) {
        if (ws != null) {
            try {
                Buffer buffer = new Buffer().writeUtf8(data);
                ws.sendMessage(WebSocket.PayloadType.TEXT, buffer.buffer());
                buffer.flush();
                buffer.close();
            } catch (Exception e) {
                e.printStackTrace();
                reportError(e.getMessage());
            }
        } else {
            reportError("WebSocket is not ready");
        }
    }

    @Override
    public void close(int code, String reason) {
        if (ws != null) {
            try {
                ws.close(code, reason);
            } catch (Exception e) {
                e.printStackTrace();
                reportError(e.getMessage());
            }
        }
    }

    @Override
    public void destroy() {
        if (ws != null) {
            try {
                ws.close(WebSocketCloseCodes.CLOSE_GOING_AWAY.getCode(), WebSocketCloseCodes.CLOSE_GOING_AWAY.name());
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void reportError(String message) {
        if (eventListener != null) {
            eventListener.onError(message);
        }
    }
}
