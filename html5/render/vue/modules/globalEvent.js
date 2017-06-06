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
 * globalEvent API:
 * @doc http://weex.apache.org/cn/references/modules/globalevent.html
 */
// track varies kinds of events and listeners.
const handlerTraker = {}

export default {
  /**
   * addEventListener
   * NOTE: one callback can only be bound to the same event once. Bind a callback twice doesn't
   *  mean it will be called twice when the event fired once.
   * @param {string} evt - the event name to add a listener on.
   */
  addEventListener (evt, callback) {
    if (!callback) {
      if (process.env.NODE_ENV === 'development') {
        console.error(`[vue-render] missing callback arg in globalEvent.addEventListener.`)
      }
      return
    }
    let handlers = handlerTraker[evt]
    if (!handlers) {
      handlers = handlerTraker[evt] = []
    }
    const len = handlers.length
    for (let i = 0; i < len; i++) {
      if (handlers[i] === callback) {
        // this callback is already bound. no need to bind it again.
        return
      }
    }
    handlers.push(callback)
    document.addEventListener(evt, callback)
  },

  /**
   * removeEventListener
   * NOTE: remove all the event handlers for the specified event type.
   * @param {string} evt - the event name to remove a listener from.
   */
  removeEventListener (evt) {
    const handlers = handlerTraker[evt]
    if (!handlers) {
      // evt handlers has been already removed.
      return
    }
    handlers.forEach(cb => document.removeEventListener(evt, cb))
    delete handlerTraker[evt]
  }
}
