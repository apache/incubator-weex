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
import '../lib/envd'

import { init as initViewport } from './viewport'
import { extend } from '../utils'

/**
 * get WXEnvironment info.
 * @param  {object} viewportInfo: info about viewport.
 * @param  {object} envInfo: info parsed from lib.env.
 */
export function initEnv (viewportInfo, envInfo) {
  const browserName = envInfo.browser ? envInfo.browser.name : navigator.appName
  const browserVersion = envInfo.browser ? envInfo.browser.version.val : null
  let osName = envInfo.os.name
  if (osName.match(/(iPhone|iPad|iPod)/i)) {
    osName = 'iOS'
  }
  else if (osName.match(/Android/i)) {
    osName = 'android'
  }
  const osVersion = envInfo.os.version.val
  const env = {
    platform: 'Web',
    weexVersion: 'process.env.WEEX_VERSION',
    userAgent: navigator.userAgent,
    appName: browserName,
    appVersion: browserVersion,
    osName,
    osVersion,
    deviceModel: envInfo.os.name || null
  }
  /**
   * viewportInfo: scale, deviceWidth, deviceHeight. dpr
   */
  return extend(env, viewportInfo)
}

// const viewportInfo = initViewport()

// 750 by default currently
// const scale = viewportInfo.scale

// const units = {
//   REM: 12 * scale,
//   VW: viewportInfo.deviceWidth / 100,
//   VH: viewportInfo.deviceHeight / 100,
//   VMIN: Math.min(viewportInfo.deviceWidth, viewportInfo.deviceHeight) / 100,
//   VMAX: Math.max(viewportInfo.deviceWidth, viewportInfo.deviceHeight) / 100,
//   CM: 96 / 2.54 * scale,
//   MM: 96 / 25.4 * scale,
//   Q: 96 / 25.4 / 4 * scale,
//   IN: 96 * scale,
//   PT: 96 / 72 * scale,
//   PC: 96 / 6 * scale,
//   PX: scale
// }

// Object.freeze(units)
// Object.freeze(env)

// window.CSS_UNIT = units
window.WXEnvironment = initEnv(initViewport(), window.lib.env)
