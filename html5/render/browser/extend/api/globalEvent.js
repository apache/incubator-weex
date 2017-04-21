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
'use strict'

// track varies kinds of events and listeners.
const handlerTraker = {}

const globalEvent = {

  /**
   * addEventListener
   * @param {string} evt - the event name to add a listener on.
   */
  addEventListener (evt, callbackId) {
    const cb = e => this.sender.performCallback(callbackId, e, true)
    if (!handlerTraker[evt]) {
      handlerTraker[evt] = [cb]
    }
    else {
      handlerTraker[evt].push(cb)
    }
    document.addEventListener(evt, cb)
  },

  /**
   * removeEventListener
   * @param {string} evt - the event name to remove a listener from.
   */
  removeEventListener (evt) {
    handlerTraker[evt].forEach(cb => document.removeEventListener(evt, cb))
  }
}

const meta = {
  globalEvent: [{
    name: 'addEventListener',
    args: ['string', 'function']
  }, {
    name: 'removeEventListener',
    args: ['string']
  }]
}

export default {
  init (Weex) {
    Weex.registerApiModule('globalEvent', globalEvent, meta)
  }
}
