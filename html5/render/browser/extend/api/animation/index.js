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

import { transitionOnce } from './lib'

const _data = {}

const animation = {

  /**
   * transition
   * @param  {string} ref        [description]
   * @param  {obj} config     [description]
   * @param  {string} callbackId [description]
   */
  transition: function (ref, config, callbackId) {
    let refData = _data[ref]
    const stylesKey = JSON.stringify(config.styles)
    const weexInstance = this
    // If the same component perform a animation with exactly the same
    // styles in a sequence with so short interval that the prev animation
    // is still in playing, then the next animation should be ignored.
    if (refData && refData[stylesKey]) {
      return
    }
    if (!refData) {
      refData = _data[ref] = {}
    }
    refData[stylesKey] = true

    const component = this.getComponentManager().getComponent(ref)
    return transitionOnce(component, config, function () {
      // Remove the stylesKey in refData so that the same animation
      // can be played again after current animation is already finished.
      delete refData[stylesKey]
      weexInstance.sender.performCallback(callbackId)
    })
  }
}

const meta = {
  animation: [{
    name: 'transition',
    args: ['string', 'object', 'function']
  }]
}

export default {
  init: function (Weex) {
    Weex.registerApiModule('animation', animation, meta)
  }
}
