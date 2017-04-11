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

// there will be only one instance of modal.
const MODAL_WRAP_CLASS = 'weex-modal-wrap'
const MODAL_NODE_CLASS = 'weex-modal-node'

export default function Modal () {
  this.wrap = document.querySelector(MODAL_WRAP_CLASS)
  this.node = document.querySelector(MODAL_NODE_CLASS)
  if (!this.wrap) {
    this.createWrap()
  }
  if (!this.node) {
    this.createNode()
  }
  this.clearNode()
  this.createNodeContent()
  this.bindEvents()
}

Modal.prototype = {

  show: function () {
    this.wrap.style.display = 'block'
    this.node.classList.remove('hide')
  },

  destroy: function () {
    document.body.removeChild(this.wrap)
    document.body.removeChild(this.node)
    this.wrap = null
    this.node = null
  },

  createWrap: function () {
    this.wrap = document.createElement('div')
    this.wrap.className = MODAL_WRAP_CLASS
    document.body.appendChild(this.wrap)
  },

  createNode: function () {
    this.node = document.createElement('div')
    this.node.classList.add(MODAL_NODE_CLASS, 'hide')
    document.body.appendChild(this.node)
  },

  clearNode: function () {
    this.node.innerHTML = ''
  },

  createNodeContent: function () {

    // do nothing.
    // child classes can override this method.
  },

  bindEvents: function () {
    this.wrap.addEventListener('click', function (e) {
      e.preventDefault()
      e.stopPropagation()
    })
  }
}
