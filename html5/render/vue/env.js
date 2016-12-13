import '../../shared/arrayFrom'
import '../../shared/objectAssign'
import '../../shared/objectSetPrototypeOf'
// TODO: Promise polyfill

// TODO: parse UA
const ua = navigator.userAgent

window.WXEnvironment = {
  platform: 'Web',
  userAgent: ua,
  appName: navigator.appName,
  appVersion: navigator.appVersion, // maybe too long
  weexVersion: '',
  osName: '',
  osVersion: '',
  deviceWidth: window.innerWidth,
  deviceHeight: window.innerHeight
}
