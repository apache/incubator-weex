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

import runtime from '../../../../../runtime/api'
import frameworks from '../../../../../runtime/frameworks'
import services from '../../../../../runtime/services'

const defaultModules = {
  animation: ['transition'],
  clipboard: ['getString', 'setString'],
  dom: ['scrollToElement', 'getComponentRect', 'addRule'],
  modal: ['alert', 'confirm', 'prompt', 'toast'],
  navigator: ['push', 'pop'],
  picker: ['pick', 'pickDate', 'pickTime'],
  storage: ['setItem', 'getItem', 'removeItem', 'length', 'getAllKeys'],
  stream: ['fetch'],
  timer: ['setTimeout', 'setInterval'],
  WebSocket: ['send', 'close', 'onopen', 'onmessage', 'onclose', 'onerror'],
  webview: ['goBack', 'goForward', 'reload']
}

const defaultComponents = [
  'div', 'text', 'image', 'input', 'a', 'switch', 'scroller', 'list', 'cell',
  'slider', 'indicator', 'loading', 'refresh', 'textarea', 'web', 'video',
  'embed', 'richtext'
]

export function createContext () {
  const currentConfig = Object.assign({}, runtime.config)
  currentConfig.frameworks = frameworks

  for (const serviceName in services) {
    runtime.service.register(serviceName, services[serviceName])
  }

  const context = runtime.init(currentConfig)

  context.registerModules(defaultModules)
  context.registerComponents(defaultComponents)

  return context
}

// execute code and return the generated vdom tree
export function execute (type, code) {
  const context = createContext()
  return new Promise(resolve => {
    const id = String(Date.now() + Math.random())
    context.createInstance(id, `
      // { "framework": "${type}" }
      ${code}
    `)
    setTimeout(() => {
      resolve(context.getRoot(id))
    }, 10)
  })
}
