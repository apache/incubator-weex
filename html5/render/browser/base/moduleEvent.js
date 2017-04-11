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
 * save the original listener for 'addEventListener'.
 * this is the dipaching listener which is waiting for feed.
 * map structure: moduleName -> eventName -> [handlers].
 */
const handlerMap = {}

window.addEventListener('moduleevent', event => {
  const dt = event.detail || {}
  const module = dt.module
  const type = dt.type
  try {
    handlerMap[module][type].forEach(handler => handler(dt))
  }
  catch (e) {
    console.warn(`[h5-render] no such event ${type} for module ${module}.`)
  }
})

export function addEventListener (module, evt, callbackId, options) {
  const once = options && !!options.once
  let moduleMap = handlerMap[module]
  if (!moduleMap) {
    moduleMap = handlerMap[module] = {}
  }
  let handlers = moduleMap[evt]
  if (!handlers) {
    handlers = moduleMap[evt] = []
  }
  handlers.push((data) => this.sender.performCallback(callbackId, data, !once))
}

export function removeAllEventListeners (module, evt) {
  try {
    delete handlerMap[module][evt]
  }
  catch (e) {
    // do nothing.
  }
}
