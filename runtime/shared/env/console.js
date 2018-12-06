/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * @fileOverview
 * This file will hack `console` methods by `WXEnvironment.logLevel`.
 *
 * So we can control how many and which messages will be sent by change the log level.
 * Additionally in native platform the message content must be primitive values and
 * using `nativeLog(...args, logLevelMark)` so we create a new `console` object in
 * global add a format process for its methods.
 */

import { checkLevel, generateLevelMap, resetLevelMap } from '../utils'

const originalConsole = global.console

/**
 * Hack console for native environment.
 */
export function setNativeConsole () {
  generateLevelMap()

  /* istanbul ignore next */
  // mock console in native environment
  if (global.WXEnvironment && global.WXEnvironment.platform !== 'Web') {
    global.console = {
      debug: (...args) => {
        if (checkLevel('debug')) { global.nativeLog(...format(args), '__DEBUG') }
      },
      log: (...args) => {
        if (checkLevel('log')) { global.nativeLog(...format(args), '__LOG') }
      },
      info: (...args) => {
        if (checkLevel('info')) { global.nativeLog(...format(args), '__INFO') }
      },
      warn: (...args) => {
        if (checkLevel('warn')) { global.nativeLog(...format(args), '__WARN') }
      },
      error: (...args) => {
        if (checkLevel('error')) { global.nativeLog(...format(args), '__ERROR') }
      }
    }
  }

  // Web or Node
  else {
    const { debug, log, info, warn, error } = console
    console.__ori__ = { debug, log, info, warn, error }
    console.debug = (...args) => {
      if (checkLevel('debug')) { console.__ori__.debug.apply(console, args) }
    }
    console.log = (...args) => {
      if (checkLevel('log')) { console.__ori__.log.apply(console, args) }
    }
    console.info = (...args) => {
      if (checkLevel('info')) { console.__ori__.info.apply(console, args) }
    }
    console.warn = (...args) => {
      if (checkLevel('warn')) { console.__ori__.warn.apply(console, args) }
    }
    console.error = (...args) => {
      if (checkLevel('error')) { console.__ori__.error.apply(console, args) }
    }
  }
}

/**
 * Reset hacked console to original.
 */
/* istanbul ignore next */
export function resetNativeConsole () {
  resetLevelMap()
  global.console = originalConsole
}

/**
 * Convert all log arguments into primitive values.
 * @param  {array} args
 * @return {array}
 */
/* istanbul ignore next */
function format (args) {
  return args.map((v) => {
    const type = Object.prototype.toString.call(v)
    if (type.toLowerCase() === '[object object]') {
      v = JSON.stringify(v)
    }
    else {
      v = String(v)
    }
    return v
  })
}
