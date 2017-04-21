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

/* global XMLHttpRequest */

'use strict'

function loadByXHR (config, callback) {
  if (!config.source) {
    callback(new Error('xhr loader: missing config.source.'))
  }
  const xhr = new XMLHttpRequest()
  xhr.open('GET', config.source)
  xhr.onload = function () {
    callback(null, this.responseText)
  }
  xhr.onerror = function (error) {
    callback(error)
  }
  xhr.send()
}

function loadByJsonp (config, callback) {
  if (!config.source) {
    callback(new Error('jsonp loader: missing config.source.'))
  }
  const callbackName = config.jsonpCallback || 'weexJsonpCallback'
  window[callbackName] = function (code) {
    if (code) {
      callback(null, code)
    }
    else {
      callback(new Error('load by jsonp error'))
    }
  }
  const script = document.createElement('script')
  script.src = decodeURIComponent(config.source)
  script.type = 'text/javascript'
  document.body.appendChild(script)
}

function loadBySourceCode (config, callback) {
  // src is the jsbundle.
  // no need to fetch from anywhere.
  if (config.source) {
    callback(null, config.source)
  }
  else {
    callback(new Error('source code laoder: missing config.source.'))
  }
}

const callbackMap = {
  xhr: loadByXHR,
  jsonp: loadByJsonp,
  source: loadBySourceCode
}

export function load (options, callback) {
  const loadFn = callbackMap[options.loader]
  loadFn(options, callback)
}

export function registerLoader (name, loaderFunc) {
  if (typeof loaderFunc === 'function') {
    callbackMap[name] = loaderFunc
  }
}
