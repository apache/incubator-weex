import 'envd'

import { init as initViewport } from './viewport'
import { extend } from '../utils'

/**
 * get WXEnvironment info.
 * @param  {object} viewportInfo: info about viewport.
 * @param  {object} envInfo: info parsed from lib.env.
 */
export function initEnv (viewportInfo, envInfo) {
  const env = {
    platform: 'Web',
    weexVersion: 'process.env.WEEX_VERSION',
    userAgent: navigator.userAgent,
    appName: envInfo.aliapp ? envInfo.aliapp.appname : navigator.appName,
    appVersion: envInfo.aliapp ? envInfo.aliapp.version.val : null,
    osName: envInfo.browser ? envInfo.browser.name : null,
    osVersion: envInfo.browser ? envInfo.browser.version.val : null,
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
