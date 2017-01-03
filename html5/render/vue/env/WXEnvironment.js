// TODO: parse UA
const ua = navigator.userAgent

const env = {
  platform: 'Web',
  userAgent: ua,
  appName: navigator.appName,
  appVersion: navigator.appVersion,
  weexVersion: '',
  osName: '',
  osVersion: '',
  deviceWidth: window.innerWidth,
  deviceHeight: window.innerHeight
}

// 750 by default currently
const scale = 2

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
Object.freeze(WXEnvironment)

window.CSS_UNIT = units
window.WXEnvironment = env
