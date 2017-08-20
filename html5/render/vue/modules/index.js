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

// modules from render/browesr

import event from '../../browser/extend/api/event'
import geolocation from '../../browser/extend/api/geolocation'
import pageInfo from '../../browser/extend/api/pageInfo'
import storage from '../../browser/extend/api/storage'
import stream from '../../browser/extend/api/stream'
import clipboard from '../../browser/extend/api/clipboard'

// custom modules
import animation from './animation'
import dom from './dom'
import globalEvent from './globalEvent'
import modal from './modal'
import navigator from './navigator'
import webview from './webview'
import websocket from './websocket'

const legacyModules = {
  event,
  geolocation,
  pageInfo,
  storage,
  stream,
  clipboard
}

const modules = {
  animation,
  dom,
  globalEvent,
  navigator,
  webview
}

export default {
  init (weex) {
    for (const k in legacyModules) {
      weex.install(legacyModules[k])
    }
    weex.install(modal)
    weex.install(websocket)
    for (const k in modules) {
      weex.registerModule(k, modules[k])
    }
  }
}
