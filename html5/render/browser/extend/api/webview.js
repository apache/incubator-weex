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

const webview = {

  // ref: ref of the web component.
  goBack: function (ref) {
    const webComp = this.getComponentManager().getComponent(ref)
    if (!webComp.goBack) {
      console.error('error: the specified component has no method of'
          + ' goBack. Please make sure it is a webview component.')
      return
    }
    webComp.goBack()
  },

  // ref: ref of the web component.
  goForward: function (ref) {
    const webComp = this.getComponentManager().getComponent(ref)
    if (!webComp.goForward) {
      console.error('error: the specified component has no method of'
          + ' goForward. Please make sure it is a webview component.')
      return
    }
    webComp.goForward()
  },

  // ref: ref of the web component.
  reload: function (ref) {
    const webComp = this.getComponentManager().getComponent(ref)
    if (!webComp.reload) {
      console.error('error: the specified component has no method of'
          + ' reload. Please make sure it is a webview component.')
      return
    }
    webComp.reload()
  }

}

const meta = {
  webview: [{
    name: 'goBack',
    args: ['string']
  }, {
    name: 'goForward',
    args: ['string']
  }, {
    name: 'reload',
    args: ['string']
  }]
}

export default {
  init: function (Weex) {
    Weex.registerApiModule('webview', webview, meta)
  }
}
