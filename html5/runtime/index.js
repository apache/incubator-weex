/**
 * @fileOverview
 * Register framework(s) in JS runtime. Weex supply two layers for 3rd-party
 * framework(s): one is the instance management layer, another is the
 * virtual-DOM layer.
 */

import * as shared from '../shared'
import { Document, Element, Comment } from './vdom'
import Listener from './listener'
import init from './init'

const config = {
  Document, Element, Comment, Listener,
  sendTasks (...args) {
    return global.callNative(...args)
  }
}

Document.handler = config.sendTasks

export default {
  setNativeConsole: shared.setNativeConsole,
  resetNativeConsole: shared.resetNativeConsole,
  setNativeTimer: shared.setNativeTimer,
  resetNativeTimer: shared.resetNativeTimer,
  init,
  config
}
