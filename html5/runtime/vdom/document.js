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
 * @fileOverview
 * Virtual-DOM Document.
 */

import '../../shared/objectAssign'
import Comment from './comment'
import Element from './element'
import Listener from '../listener'
import { TaskCenter } from '../task-center'
import { createHandler } from '../handler'
import { addDoc, removeDoc, appendBody, setBody } from './operation'

export default function Document (id, url, handler) {
  id = id ? id.toString() : ''
  this.id = id
  this.URL = url

  addDoc(id, this)
  this.nodeMap = {}
  const L = Document.Listener || Listener
  this.listener = new L(id, handler || createHandler(id, Document.handler)) // deprecated
  this.taskCenter = new TaskCenter(id, handler ? (id, ...args) => handler(...args) : Document.handler)
  this.createDocumentElement()
}

// default task handler
Document.handler = null

/**
 * Update all changes for an element.
 * @param {object} element
 * @param {object} changes
 */
function updateElement (el, changes) {
  const attrs = changes.attrs || {}
  for (const name in attrs) {
    el.setAttr(name, attrs[name], true)
  }
  const style = changes.style || {}
  for (const name in style) {
    el.setStyle(name, style[name], true)
  }
}

Object.assign(Document.prototype, {
  /**
   * Get the node from nodeMap.
   * @param {string} reference id
   * @return {object} node
   */
  getRef (ref) {
    return this.nodeMap[ref]
  },

  /**
   * Turn on batched updates.
   */
  open () {
    this.listener.batched = false
  },

  /**
   * Turn off batched updates.
   */
  close () {
    this.listener.batched = true
  },

  /**
   * Create the document element.
   * @return {object} documentElement
   */
  createDocumentElement () {
    if (!this.documentElement) {
      const el = new Element('document')
      el.docId = this.id
      el.ownerDocument = this
      el.role = 'documentElement'
      el.depth = 0
      el.ref = '_documentElement'
      this.nodeMap._documentElement = el
      this.documentElement = el

      Object.defineProperty(el, 'appendChild', {
        configurable: true,
        enumerable: true,
        writable: true,
        value: (node) => {
          appendBody(this, node)
        }
      })

      Object.defineProperty(el, 'insertBefore', {
        configurable: true,
        enumerable: true,
        writable: true,
        value: (node, before) => {
          appendBody(this, node, before)
        }
      })
    }

    return this.documentElement
  },

  /**
   * Create the body element.
   * @param {string} type
   * @param {objct} props
   * @return {object} body element
   */
  createBody (type, props) {
    if (!this.body) {
      const el = new Element(type, props)
      setBody(this, el)
    }

    return this.body
  },

  /**
   * Create an element.
   * @param {string} tagName
   * @param {objct} props
   * @return {object} element
   */
  createElement (tagName, props) {
    return new Element(tagName, props)
  },

  /**
   * Create an comment.
   * @param {string} text
   * @return {object} comment
   */
  createComment (text) {
    return new Comment(text)
  },

  /**
   * Fire an event on specified element manually.
   * @param {object} element
   * @param {string} event type
   * @param {object} event object
   * @param {object} dom changes
   * @return {} anything returned by handler function
   */
  fireEvent (el, type, e, domChanges) {
    if (!el) {
      return
    }
    e = e || {}
    e.type = type
    e.target = el
    e.timestamp = Date.now()
    if (domChanges) {
      updateElement(el, domChanges)
    }
    return el.fireEvent(type, e)
  },

  /**
   * Destroy current document, and remove itself form docMap.
   */
  destroy () {
    delete this.listener
    delete this.nodeMap
    removeDoc(this.id)
  }
})
