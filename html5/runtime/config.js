import { Document, Element, Comment } from './vdom'
import Listener from './listener'

const config = {
  Document, Element, Comment, Listener,
  sendTasks (...args) {
    return global.callNative(...args)
  }
}

Document.handler = config.sendTasks

export default config
