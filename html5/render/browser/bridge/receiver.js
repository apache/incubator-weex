'use strict'

import config from '../render/config'
import protocol from './protocol'
import { isArray } from '../utils'
import Sender from './sender'

// callNative: jsFramework will call this method to talk to
// this renderer.
// params:
//  - instanceId: string.
//  - tasks: array of object.
//  - callbackId: number.
function callNative (instanceId, tasks, callbackId) {
  let calls = []
  if (typeof tasks === 'string') {
    try {
      calls = JSON.parse(tasks)
    }
    catch (e) {
      console.error('invalid tasks:', tasks)
    }
  }
  else if (isArray(tasks)) {
    calls = tasks
  }
  const len = calls.length
  calls[len - 1].callbackId = (!callbackId && callbackId !== 0)
                              ? -1
                              : callbackId
  for (let i = 0; i < len; i++) {
    processCall(instanceId, calls[i])
  }
}

function processCall (instanceId, call) {
  const isComponent = typeof call.module === 'undefined'
  isComponent ? componentCall(instanceId, call) : moduleCall(instanceId, call)

  const callbackId = call.callbackId
  if ((callbackId
    || callbackId === 0
    || callbackId === '0')
    && callbackId !== '-1'
    && callbackId !== -1) {
    performNextTick(instanceId, callbackId)
  }
}

function moduleCall (instanceId, call) {
  const moduleName = call.module
  const methodName = call.method
  let module, method
  const args = call.args || call.arguments || []

  if (!(module = protocol.apiModule[moduleName])) {
    return
  }
  if (!(method = module[methodName])) {
    return
  }

  method.apply(global.weex.getInstance(instanceId), args)
}

function componentCall (instanceId, call) {
  const componentName = call.component
  const ref = call.ref
  const methodName = call.method
  const args = call.args || call.arguments || []

  const elem = global.weex.getInstance(instanceId).getComponentManager().getComponent(ref)
  if (!elem) {
    return console.error(`[h5-render] component of ref ${ref} doesn't exist.`)
  }

  let method

  if (!(method = elem[methodName])) {
    return console.error(`[h5-render] component ${componentName} doesn't have a method named ${methodName}.`)
  }

  method.apply(elem, args)
}

function performNextTick (instanceId, callbackId) {
  Sender.getSender(instanceId).performCallback(callbackId)
}

function nativeLog () {
  if (config.debug) {
    if (arguments[0].match(/^perf/)) {
      console.info.apply(console, arguments)
      return
    }
    console.debug.apply(console, arguments)
  }
}

function exportsBridgeMethodsToGlobal () {
  global.callNative = callNative
  global.nativeLog = nativeLog
}

export default {
  init: function () {
    // exports methods to global(window).
    exportsBridgeMethodsToGlobal()
  }
}
