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
import ComponentManager from '../dom/componentManager'
import { registerLoader } from './loader'
import { protocol } from '../bridge'
import { extend } from '../utils'
import { addEventListener, removeAllEventListeners } from '../base/moduleEvent'

/**
 * register module event listener for every api module except 'globalEvent'.
 */
function registerModuleEventListener (name, module, meta) {
  if (name !== 'globalEvent') {
    module['addEventListener'] = function (evt, callbackId, options) {
      return addEventListener.call(this, name, evt, callbackId, options)
    }
    module['removeAllEventListeners'] = function (evt) {
      return removeAllEventListeners.call(this, name, evt)
    }
    ; [{
      name: 'addEventListener',
      args: ['string', 'function', 'object']
    }, {
      name: 'removeAllEventListeners',
      args: ['string']
    }].forEach(info => meta[name].push(info))
  }
}

const methods = {
  // Register a new component with the specified name.
  registerComponent (name, comp) {
    ComponentManager.registerComponent(name, comp)
  },

  // Register a new api module.
  // If the module already exists, just add methods from the
  // new module to the old one.
  registerApiModule (name, module, meta) {
    registerModuleEventListener(name, module, meta)
    if (!protocol.apiModule[name]) {
      protocol.apiModule[name] = module
    }
    else {
      for (const key in module) {
        if (module.hasOwnProperty(key)) {
          protocol.apiModule[name][key] = module[key]
        }
      }
    }
    // register API module's meta info to jsframework
    if (meta) {
      protocol.setApiModuleMeta(meta)
      global.registerModules(protocol.getApiModuleMeta(name), true)
    }
  },

  // Register a new api method for the specified module.
  // opts:
  //  - args: type of arguments the API method takes such
  //    as ['string', 'function']
  registerApi (moduleName, name, method, args) {
    if (typeof method !== 'function') {
      return
    }
    if (!protocol.apiModule[moduleName]) {
      protocol.apiModule[moduleName] = {}
      protocol._meta[moduleName] = []
    }
    protocol.apiModule[moduleName][name] = method
    if (!args) {
      return
    }
    // register API meta info to jsframework
    protocol.setApiMeta(moduleName, {
      name: name,
      args: args
    })
    global.registerModules(protocol.getApiModuleMeta(moduleName), true)
  },

  // Register a new weex-bundle-loader.
  registerLoader (name, loaderFunc) {
    registerLoader(name, loaderFunc)
  }

}

// To install components and plugins.
function install (mod) {
  mod.init(this)
}

export function bind (Weex) {
  Weex.install = install.bind(Weex)
  extend(Weex, methods)
}
