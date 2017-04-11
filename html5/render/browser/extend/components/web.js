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

let isArray

function handleMsg (web, evt) {
  let msg = evt.data
  if (typeof msg === 'string') {
    try {
      msg = JSON.parse(msg)
    }
    catch (e) {}
  }
  if (!msg) {
    return
  }
  if (msg.type === 'weex') {
    if (!isArray(msg.content)) {
      return console.error('[h5-render] weex msg received by web component.'
        + ' msg.content should be a array:', msg.content)
    }
    callNative(web.getComponentManager().instanceId, msg.content)
  }
}

function getProto (Weex) {
  const Atomic = Weex.Atomic
  return {
    create () {
      // Iframe's defect: can't use position:absolute and top, left, right,
      // bottom all setting to zero and use margin to leave specified
      // height for a blank area, and have to use 100% to fill the parent
      // container, otherwise it will use a unwanted default size instead.
      // Therefore a div as a iframe wrapper is needed here.
      const node = document.createElement('div')
      node.classList.add('weex-container')
      this.web = document.createElement('iframe')
      node.appendChild(this.web)
      this.web.classList.add('weex-element')
      this.web.style.width = '100%'
      this.web.style.height = '100%'
      this.web.style.border = 'none'
      return node
    },

    bindEvents (evts) {
      Atomic.prototype.bindEvents.call(this, evts)
      const that = this
      this.web.addEventListener('load', function (e) {
        that.dispatchEvent('pagefinish', {
          url: that.web.src
        })
      })
      window.addEventListener('message', handleMsg.bind(null, this))
    },

    goBack () {
      this.web.contentWindow.history.back()
    },

    goForward () {
      this.web.contentWindow.history.forward()
    },

    reload () {
      this.web.contentWindow.location.reload()
    }
  }
}

const attr = {
  src: function (val) {
    this.web.src = val
    setTimeout(function () {
      this.dispatchEvent('pagestart', { url: val })
    }.bind(this), 0)
  }
}

function init (Weex) {
  const Atomic = Weex.Atomic
  const extend = Weex.utils.extend
  isArray = Weex.utils.isArray

  // A component to import web pages, which works like
  // a iframe element or a webview.
  // attrs:
  //   - src
  // events:
  //   - pagestart
  //   - pagefinish
  //   - error
  function Web (data) {
    Atomic.call(this, data)
  }
  Web.prototype = Object.create(Atomic.prototype)
  extend(Web.prototype, getProto(Weex))
  extend(Web.prototype, { attr })

  Weex.registerComponent('web', Web)
}

export default { init }
