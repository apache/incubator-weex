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
/**
 * websocket module
 */
/*global WebSocket*/
export default (function () {
  const registerListeners = ['onopen', 'onmessage', 'onerror', 'onclose']
  const ws = {
    INSTANCE: null,
    WebSocket: function (url, protocol) {
      if (!url) {
        ws.INSTANCE = null
        return
      }
      if (!protocol) {
        ws.INSTANCE = new WebSocket(url)
      }
      else {
        ws.INSTANCE = new WebSocket(url, protocol)
      }
      return ws.INSTANCE
    },
    send: function (messages) {
      ws.INSTANCE && ws.INSTANCE.send(messages)
    },
    close: function () {
      ws.INSTANCE && ws.INSTANCE.close()
    }
  }
  for (const i in registerListeners) {
    if (registerListeners.hasOwnProperty(i)) {
      Object.defineProperty(ws, registerListeners[i], {
        get: function () {
          return ws.INSTANCE && ws.INSTANCE[registerListeners[i]]
        },
        set: function (fn) {
          if (ws.INSTANCE) {
            ws.INSTANCE[registerListeners[i]] = fn
          }
        }
      })
    }
  }
  return ws
})()
