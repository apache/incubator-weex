import './version'
import './params'

const lib = window.lib || (window.lib = {})
const env = lib.env || (lib.env = {})

const ua = window.navigator.userAgent
let match

/**
 * os
 */

match = ua.match(/Windows\sPhone\s(?:OS\s)?([\d.]+)/)
if (match) {
  /**
   * @type {Object}
   * @memberof lib.env
   * @property {String} name - os name, e.g. Android/AndroidPad/iPhone/iPod/iPad/Windows Phone/unknown, etc.
   * @property {lib.env~Version} version - os version.
   * @property {Boolean} isWindowsPhone
   * @property {Boolean} isIPhone - is iPhone/iTouch
   * @property {Boolean} isIPad
   * @property {Boolean} isIOS
   * @property {Boolean} isAndroid
   * @property {Boolean} isAndroidPad
   */
  env.os = {
    name: 'Windows Phone',
    isWindowsPhone: true,
    version: match[1]
  }
}
else if (!!ua.match(/Safari/) && (match = ua.match(/Android[\s/]([\d.]+)/))) {
  env.os = {
    version: match[1]
  }

  if ((ua.match(/Mobile\s+Safari/))) {
    env.os.name = 'Android'
    env.os.isAndroid = true
  }
  else {
    env.os.name = 'AndroidPad'
    env.os.isAndroidPad = true
  }
}
else if ((match = ua.match(/(iPhone|iPad|iPod)/))) {
  const name = match[1]

  match = ua.match(/OS ([\d_.]+) like Mac OS X/)

  env.os = {
    name: name,
    isIPhone: (name === 'iPhone' || name === 'iPod'),
    isIPad: name === 'iPad',
    isIOS: true,
    version: match[1].split('_').join('.')
  }
}
else {
  env.os = {
    name: 'unknown',
    version: '0.0.0'
  }
}

if (lib.version) {
  env.os.version = lib.version(env.os.version)
}

/**
 * browser
 */

match = ua.match(/(?:UCWEB|UCBrowser\/)([\d.]+)/)

if (match) {
  /**
   * @type {Object}
   * @memberof env
   * @property {String} name - browser name，e.g. UC/QQ/Firefox/Chrome/Android/Safari/iOS Webview/Chrome Webview/IE/IEMobile/unknown, etc.
   * @property {env~Version} version - browser version.
   * @property {Boolean} isUC
   * @property {Boolean} isQQ
   * @property {Boolean} isIE
   * @property {Boolean} isIEMobile
   * @property {Boolean} isIELikeWebkit
   * @property {Boolean} isChrome
   * @property {Boolean} isAndroid
   * @property {Boolean} isSafari
   * @property {Boolean} isWebview
   */
  env.browser = {
    name: 'UC',
    isUC: true,
    version: match[1]
  }
}
else if ((match = ua.match(/MQQBrowser\/([\d.]+)/))) {
  env.browser = {
    name: 'QQ',
    isQQ: true,
    version: match[1]
  }
}
else if ((match = ua.match(/Firefox\/([\d.]+)/))) {
  env.browser = {
    name: 'Firefox',
    isFirefox: true,
    version: match[1]
  }
}
else if ((match = ua.match(/MSIE\s([\d.]+)/))
  || (match = ua.match(/IEMobile\/([\d.]+)/))) {
  env.browser = {
    version: match[1]
  }

  if (ua.match(/IEMobile/)) {
    env.browser.name = 'IEMobile'
    env.browser.isIEMobile = true
  }
  else {
    env.browser.name = 'IE'
    env.browser.isIE = true
  }

  if (ua.match(/Android|iPhone/)) {
    env.browser.isIELikeWebkit = true
  }
}
else if ((match = ua.match(/(?:Chrome|CriOS)\/([\d.]+)/))) {
  env.browser = {
    name: 'Chrome',
    isChrome: true,
    version: match[1]
  }

  if (ua.match(/Version\/[\d+.]+\s*Chrome/)) {
    env.browser.name = 'Chrome Webview'
    env.browser.isWebview = true
  }
}
else if (!!ua.match(/Safari/) && (match = ua.match(/Android[\s/]([\d.]+)/))) {
  env.browser = {
    name: 'Android',
    isAndroid: true,
    version: match[1]
  }
}
else if (ua.match(/iPhone|iPad|iPod/)) {
  if (ua.match(/Safari/)) {
    match = ua.match(/Version\/([\d.]+)/)
    env.browser = {
      name: 'Safari',
      isSafari: true,
      version: match[1]
    }
  }
  else {
    match = ua.match(/OS ([\d_.]+) like Mac OS X/)
    env.browser = {
      name: 'iOS Webview',
      isWebview: true,
      version: match[1].replace(/_/g, '.')
    }
  }
}
else {
  env.browser = {
    name: 'unknown',
    version: '0.0.0'
  }
}

if (lib.version) {
  env.browser.version = lib.version(env.browser.version)
}
