/**
 * @fileOverview
 * Register framework(s) in JS runtime. Weex supply two layers for 3rd-party
 * framework(s): one is the instance management layer, another is the
 * virtual-DOM layer.
 */

const { Document, Element, Comment } = require('./vdom')
const Listener = require('./listener').default
const init = require('./init').default

const config = {
  Document, Element, Comment, Listener,
  sendTasks (...args) {
    return global.callNative(...args)
  }
}

Document.handler = config.sendTasks

exports.init = init
exports.config = config
