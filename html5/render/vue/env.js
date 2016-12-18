import './styles/reset.css'
import './styles/components.css'

import '../../shared/arrayFrom'
import '../../shared/objectAssign'
import '../../shared/objectSetPrototypeOf'

import 'core-js/modules/es6.object.to-string'
import 'core-js/modules/es6.string.iterator'
import 'core-js/modules/web.dom.iterable'
import 'core-js/modules/es6.promise'

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
