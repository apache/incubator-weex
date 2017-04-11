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

import modal from 'modals'

const msg = {

  // duration: default is 0.8 seconds.
  toast: function (config) {
    modal.toast(config.message, config.duration)
  },

  // config:
  //  - message: string
  //  - okTitle: title of ok button
  //  - callback
  alert: function (config, callbackId) {
    const sender = this.sender
    config.callback = function () {
      sender.performCallback(callbackId)
    }
    modal.alert(config)
  },

  // config:
  //  - message: string
  //  - okTitle: title of ok button
  //  - cancelTitle: title of cancel button
  //  - callback
  confirm: function (config, callbackId) {
    const sender = this.sender
    config.callback = function (val) {
      sender.performCallback(callbackId, val)
    }
    modal.confirm(config)
  },

  // config:
  //  - message: string
  //  - okTitle: title of ok button
  //  - cancelTitle: title of cancel button
  //  - callback
  prompt: function (config, callbackId) {
    const sender = this.sender
    config.callback = function (val) {
      sender.performCallback(callbackId, val)
    }
    modal.prompt(config)
  }
}

const meta = {
  modal: [{
    name: 'toast',
    args: ['object']
  }, {
    name: 'alert',
    args: ['object', 'function']
  }, {
    name: 'confirm',
    args: ['object', 'function']
  }, {
    name: 'prompt',
    args: ['object', 'function']
  }]
}

export default {
  init: function (Weex) {
    Weex.registerApiModule('modal', msg, meta)
  }
}
