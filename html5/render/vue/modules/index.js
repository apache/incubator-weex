// modules from render/browesr
import Event from '../../browser/extend/api/event'
import Geolocation from '../../browser/extend/api/geolocation'
import Modal from '../../browser/extend/api/modal'
import PageInfo from '../../browser/extend/api/pageInfo'
import Storage from '../../browser/extend/api/storage'
import Stream from '../../browser/extend/api/stream'
// import Clipboard from '../../browser/extend/api/clipboard'
// import GlobalEvent from '../../browser/extend/api/globalEvent'

// custom modules
import animation from './animation'
import dom from './dom'
import navigator from './navigator'
import webview from './webview'

const modules = {
  animation,
  dom,
  navigator,
  webview
}

export function requireModule (name) {
  if (modules[name]) {
    return modules[name]
  }
  console.log(`[Vue Render] Can't find "${name}" module.`)
  return null
}

export function init (weex) {
  weex.install(Event)
  weex.install(Geolocation)
  weex.install(Modal)
  weex.install(PageInfo)
  weex.install(Storage)
  weex.install(Stream)
  // weex.install(Clipboard)
  // weex.install(GlobalEvent)
}
