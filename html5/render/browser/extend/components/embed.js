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

const ID_PREFIX = 'weex_embed_'

let getRandom

function _generateId () {
  return ID_PREFIX + getRandom(10)
}

const proto = {
  create () {
    const node = document.createElement('div')
    node.id = this.id
    node.style.overflow = 'scroll'
    return node
  },

  initWeex () {
    this.id = _generateId()
    this.node.id = this.id
    const config = {
      appId: this.id,
      source: this.source,
      bundleUrl: this.source,
      loader: this.loader,
      jsonpCallback: this.jsonpCallback,
      width: this.node.getBoundingClientRect().width,
      rootId: this.id,
      embed: true
    }
    window.weex.init(config)
  },

  destroyWeex () {
    this.id && window.destroyInstance(this.id)
    // TODO: unbind events and clear doms.
    this.node.innerHTML = ''
  },

  reloadWeex () {
    if (this.id) {
      this.destroyWeex()
      this.id = null
      this.node.id = null
      this.node.innerHTML = ''
    }
    this.initWeex()
  }
}

// not recommended, because of the leak of memory.
const attr = {
  src: function (value) {
    this.source = value
    this.reloadWeex()
  }
}

function init (Weex) {
  const Component = Weex.Component
  const extend = Weex.utils.extend
  getRandom = Weex.utils.getRandom

  function Embed (data, nodeType) {
    const attr = data.attr
    if (attr) {
      this.source = attr.src
      this.loader = attr.loader || 'xhr'
      this.jsonpCallback = attr.jsonpCallback
    }
    Component.call(this, data, nodeType)
  }

  Embed.prototype = Object.create(Component.prototype)
  extend(Embed.prototype, proto)
  extend(Embed.prototype, { attr })

  Weex.registerComponent('embed', Embed)
}

export default { init }
