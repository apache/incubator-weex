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
import 'envd'

import { setViewport } from './viewport'
import { extend } from '../utils'

const scaleInfo = setViewport()

const lib = window.lib
const env = {
  platform: 'Web',
  // weexVersion: '0.10.0', // TODO: get version from package.json (not sure)
  weexVersion: 'process.env.WEEX_VERSION',
  userAgent: navigator.userAgent,
  appName: lib.env.aliapp ? lib.env.aliapp.appname : navigator.appName,
  appVersion: lib.env.aliapp ? lib.env.aliapp.version.val : null,
  osName: lib.env.browser ? lib.env.browser.name : null,
  osVersion: lib.env.browser ? lib.env.browser.version.val : null,
  deviceModel: lib.env.os.name || null
}

/**
 * scaleInfo: scale, deviceWidth, deviceHeight.
 */
extend(env, scaleInfo)

// 750 by default currently
const scale = env.scale

const units = {
  REM: 12 * scale,
  VW: env.deviceWidth / 100,
  VH: env.deviceHeight / 100,
  VMIN: Math.min(env.deviceWidth, env.deviceHeight) / 100,
  VMAX: Math.max(env.deviceWidth, env.deviceHeight) / 100,
  CM: 96 / 2.54 * scale,
  MM: 96 / 25.4 * scale,
  Q: 96 / 25.4 / 4 * scale,
  IN: 96 * scale,
  PT: 96 / 72 * scale,
  PC: 96 / 6 * scale,
  PX: scale
}

Object.freeze(units)
// Object.freeze(env)

window.CSS_UNIT = units
window.WXEnvironment = env
