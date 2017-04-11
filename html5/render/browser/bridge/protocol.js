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
'use strict'

export default {

  // weex instances
  _instances: {},

  // api meta info
  _meta: {},

  // Weex.registerApiModule used this to register and access apiModules.
  apiModule: {},

  // get the api method meta info array for the module.
  getApiModuleMeta: function (moduleName) {
    const metaObj = {}
    metaObj[moduleName] = this._meta[moduleName]
    return metaObj
  },

  // Set meta info for a api module.
  // If there is a same named api, just replace it.
  // opts:
  // - metaObj: meta object like
  // {
  //    dom: [{
  //      name: 'addElement',
  //      args: ['string', 'object']
  //    }]
  // }
  setApiModuleMeta: function (metaObj) {
    let moduleName
    for (const k in metaObj) {
      if (metaObj.hasOwnProperty(k)) {
        moduleName = k
      }
    }
    const metaArray = this._meta[moduleName]
    if (!metaArray) {
      this._meta[moduleName] = metaObj[moduleName]
    }
    else {
      const nameObj = {}
      metaObj[moduleName].forEach(function (api) {
        nameObj[api.name] = api
      })
      metaArray.forEach(function (api, i) {
        if (nameObj[api.name]) {
          metaArray[i] = nameObj[api.name]
          delete nameObj[api.name]
        }
      })
      for (const k in metaObj) {
        if (metaObj.hasOwnProperty(k)) {
          metaArray.push(metaObj[k])
        }
      }
    }
    this._meta[moduleName] = metaObj[moduleName]
  },

  // Set meta info for a single api.
  // opts:
  //  - moduleName: api module name.
  //  - meta: a meta object like:
  //  {
  //    name: 'addElement',
  //    args: ['string', 'object']
  //  }
  setApiMeta: function (moduleName, meta) {
    const metaArray = this._meta[moduleName]
    if (!metaArray) {
      this._meta[moduleName] = [meta]
    }
    else {
      let metaIdx = -1
      metaArray.forEach(function (api, i) {
        let name // todo
        if (meta.name === name) {
          metaIdx = i
        }
      })
      if (metaIdx !== -1) {
        metaArray[metaIdx] = meta
      }
      else {
        metaArray.push(meta)
      }
    }
  }
}

// _registerModules([{
//   modal: [{
//     name: 'toast',
//     args: ['object', 'function']
//   }, {
//     name: 'alert',
//     args: ['object', 'function']
//   }, {
//     name: 'confirm',
//     args: ['object', 'function']
//   }, {
//     name: 'prompt',
//     args: ['object', 'function']
//   }]
// }, {
//   animation: [{
//     name: 'transition',
//     args: ['string', 'object', 'function']
//   }]
// }])
