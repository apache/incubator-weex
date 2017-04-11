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

const supportGeolocation = 'geolocation' in navigator
const errorMsg = `[h5-render]: browser doesn't support geolocation.`

const geolocation = {
  // options:
  //   - enableHighAccuracy optional, value is true or false, false by default.
  //   - timeout [none-native] optional, value is a number (milliseconds), default vaule is FINFINITY.
  //   - maximumAge [none-native] optional, value is a number (milliseconds), default value is 0.
  getCurrentPosition (successCbId, errorCbId, options) {
    const successCb = pos => this.sender.performCallback(successCbId, pos)
    const errorCb = err => this.sender.performCallback(errorCbId, err)
    if (supportGeolocation) {
      navigator.geolocation.getCurrentPosition(successCb, errorCb, options)
    }
    else {
      console.warn(errorMsg)
      errorCb(new Error(errorMsg))
    }
  },

  // options: the same with `getCurrentPosition`.
  watchPosition (successCbId, errorCbId, options) {
    const successCb = pos => this.sender.performCallback(successCbId, pos, true)
    const errorCb = err => this.sender.performCallback(errorCbId, err)
    if (supportGeolocation) {
      const id = navigator.geolocation.watchPosition(pos => {
        pos.watchId = id
        successCb(pos)
      }, errorCb, options)
    }
    else {
      console.warn(errorMsg)
      errorCb(new Error(errorMsg))
    }
  },

  clearWatch (watchId) {
    if (supportGeolocation) {
      navigator.geolocation.clearWatch(watchId)
    }
    else {
      console.warn(errorMsg)
    }
  }
}

const meta = {
  geolocation: [{
    name: 'getCurrentPosition',
    args: ['function', 'function', 'object']
  }, {
    name: 'watchPosition',
    args: ['function', 'function', 'object']
  }, {
    name: 'clearWatch',
    args: ['string']
  }]
}

export default {
  init (Weex) {
    Weex.registerApiModule('geolocation', geolocation, meta)
  }
}
