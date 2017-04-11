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

import Sticky from './sticky'

// Set positon to fixed, with a placeholder if it's in a
// scrollable component.
export function setFixed () {
  // delay processing in case the node is not appended yet.
  setTimeout(function () {
    this.node.style.position = 'fixed'
    if (!this.isInScrollable()) {
      return
    }
    const parent = this.node.parentNode
    if (parent) {
      // For the elements who are fixed: this fixedPlaceholder
      // shoud be inserted, and the fixed element itself should
      // be moved to the root container.
      this.fixedPlaceholder = document.createElement('div')
      this.fixedPlaceholder.classList.add('weex-fixed-placeholder')
      this.fixedPlaceholder.style.cssText = [
        'display:none;',
        'width:0px;',
        'height:0px;'
      ].join('')
      parent.insertBefore(this.fixedPlaceholder, this.node)
      this.getRootContainer().appendChild(this.node)
    }
  }.bind(this), 0)
}

// unset a fixed node to the pecified 'position' or 'relative' by default.
export function unsetFixed (position) {
  // For the elements who are fixed elements before, now
  // are not fixed: the fixedPlaceholder has to be replaced
  // by this element.
  position = position ? position + '' : 'relative'
  if (this.fixedPlaceholder) {
    const parent = this.fixedPlaceholder.parentNode
    parent.insertBefore(this.node, this.fixedPlaceholder)
    parent.removeChild(this.fixedPlaceholder)
    this.fixedPlaceholder = null
    this.node.style.position = position
  }
}

export function setSticky () {
  this.node.style.zIndex = 100
  setTimeout(function () {
    this.sticky = new Sticky(this)
  }.bind(this), 0)
}

export function unsetSticky () {
  if (this.sticky) {
    this.sticky.destroy()
    this.sticky = null
  }
}

// usally used to unset sticky and fixed
export function unsetPosition (position) {
  this.style.position.call(this, position)
}

export const style = {
  position (value) {
    // This is a peace of hacking to fix the problem about
    // mixing fixed and transform. See 'http://stackoverflo
    // w.com/questions/15194313/webkit-css-transform3d-posi
    // tion-fixed-issue' for more info.
    value !== 'fixed' && this.unsetFixed()
    value !== 'sticky' && this.unsetSticky()
    if (value === 'fixed') {
      return this.setFixed()
    }
    if (value === 'sticky') {
      return this.setSticky()
    }
    this.node.style.position = value
  }
}
