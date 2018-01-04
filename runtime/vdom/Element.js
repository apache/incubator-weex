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

import Node from './Node'
import {
  getDoc,
  getTaskCenter,
  linkParent,
  nextElement,
  previousElement,
  insertIndex,
  moveIndex,
  removeIndex
} from './operation'
import { uniqueId, isEmpty } from '../shared/utils'
import { getWeexElement, setElement } from './WeexElement'

const DEFAULT_TAG_NAME = 'div'
const BUBBLE_EVENTS = [
  'click', 'longpress', 'touchstart', 'touchmove', 'touchend',
  'panstart', 'panmove', 'panend', 'horizontalpan', 'verticalpan', 'swipe'
]

function registerNode (docId, node) {
  const doc = getDoc(docId)
  doc.nodeMap[node.nodeId] = node
}

export default class Element extends Node {
  constructor (type = DEFAULT_TAG_NAME, props, isExtended) {
    super()

    const WeexElement = getWeexElement(type)
    if (WeexElement && !isExtended) {
      return new WeexElement(type, props, true)
    }

    props = props || {}
    this.nodeType = 1
    this.nodeId = uniqueId()
    this.ref = this.nodeId
    this.type = type
    this.attr = props.attr || {}
    this.style = props.style || {}
    this.classStyle = props.classStyle || {}
    this.event = {}
    this.children = []
    this.pureChildren = []
  }

  /**
   * Append a child node.
   * @param {object} node
   * @return {undefined | number} the signal sent by native
   */
  appendChild (node) {
    if (node.parentNode && node.parentNode !== this) {
      return
    }
    /* istanbul ignore else */
    if (!node.parentNode) {
      linkParent(node, this)
      insertIndex(node, this.children, this.children.length, true)
      if (this.docId) {
        registerNode(this.docId, node)
      }
      if (node.nodeType === 1) {
        insertIndex(node, this.pureChildren, this.pureChildren.length)
        const taskCenter = getTaskCenter(this.docId)
        if (taskCenter) {
          return taskCenter.send(
            'dom',
            { action: 'addElement' },
            [this.ref, node.toJSON(), -1]
          )
        }
      }
    }
    else {
      moveIndex(node, this.children, this.children.length, true)
      if (node.nodeType === 1) {
        const index = moveIndex(node, this.pureChildren, this.pureChildren.length)
        const taskCenter = getTaskCenter(this.docId)
        if (taskCenter && index >= 0) {
          return taskCenter.send(
            'dom',
            { action: 'moveElement' },
            [node.ref, this.ref, index]
          )
        }
      }
    }
  }

  /**
   * Insert a node before specified node.
   * @param {object} node
   * @param {object} before
   * @return {undefined | number} the signal sent by native
   */
  insertBefore (node, before) {
    if (node.parentNode && node.parentNode !== this) {
      return
    }
    if (node === before || (node.nextSibling && node.nextSibling === before)) {
      return
    }
    if (!node.parentNode) {
      linkParent(node, this)
      insertIndex(node, this.children, this.children.indexOf(before), true)
      if (this.docId) {
        registerNode(this.docId, node)
      }
      if (node.nodeType === 1) {
        const pureBefore = nextElement(before)
        const index = insertIndex(
          node,
          this.pureChildren,
          pureBefore
            ? this.pureChildren.indexOf(pureBefore)
            : this.pureChildren.length
        )
        const taskCenter = getTaskCenter(this.docId)
        if (taskCenter) {
          return taskCenter.send(
            'dom',
            { action: 'addElement' },
            [this.ref, node.toJSON(), index]
          )
        }
      }
    }
    else {
      moveIndex(node, this.children, this.children.indexOf(before), true)
      if (node.nodeType === 1) {
        const pureBefore = nextElement(before)
        /* istanbul ignore next */
        const index = moveIndex(
          node,
          this.pureChildren,
          pureBefore
            ? this.pureChildren.indexOf(pureBefore)
            : this.pureChildren.length
        )
        const taskCenter = getTaskCenter(this.docId)
        if (taskCenter && index >= 0) {
          return taskCenter.send(
            'dom',
            { action: 'moveElement' },
            [node.ref, this.ref, index]
          )
        }
      }
    }
  }

  /**
   * Insert a node after specified node.
   * @param {object} node
   * @param {object} after
   * @return {undefined | number} the signal sent by native
   */
  insertAfter (node, after) {
    if (node.parentNode && node.parentNode !== this) {
      return
    }
    if (node === after || (node.previousSibling && node.previousSibling === after)) {
      return
    }
    if (!node.parentNode) {
      linkParent(node, this)
      insertIndex(node, this.children, this.children.indexOf(after) + 1, true)
      /* istanbul ignore else */
      if (this.docId) {
        registerNode(this.docId, node)
      }
      if (node.nodeType === 1) {
        const index = insertIndex(
          node,
          this.pureChildren,
          this.pureChildren.indexOf(previousElement(after)) + 1
        )
        const taskCenter = getTaskCenter(this.docId)
        /* istanbul ignore else */
        if (taskCenter) {
          return taskCenter.send(
            'dom',
            { action: 'addElement' },
            [this.ref, node.toJSON(), index]
          )
        }
      }
    }
    else {
      moveIndex(node, this.children, this.children.indexOf(after) + 1, true)
      if (node.nodeType === 1) {
        const index = moveIndex(
          node,
          this.pureChildren,
          this.pureChildren.indexOf(previousElement(after)) + 1
        )
        const taskCenter = getTaskCenter(this.docId)
        if (taskCenter && index >= 0) {
          return taskCenter.send(
            'dom',
            { action: 'moveElement' },
            [node.ref, this.ref, index]
          )
        }
      }
    }
  }

  /**
   * Remove a child node, and decide whether it should be destroyed.
   * @param {object} node
   * @param {boolean} preserved
   */
  removeChild (node, preserved) {
    if (node.parentNode) {
      removeIndex(node, this.children, true)
      if (node.nodeType === 1) {
        removeIndex(node, this.pureChildren)
        const taskCenter = getTaskCenter(this.docId)
        if (taskCenter) {
          taskCenter.send(
            'dom',
            { action: 'removeElement' },
            [node.ref]
          )
        }
      }
    }
    if (!preserved) {
      node.destroy()
    }
  }

  /**
   * Clear all child nodes.
   */
  clear () {
    const taskCenter = getTaskCenter(this.docId)
    /* istanbul ignore else */
    if (taskCenter) {
      this.pureChildren.forEach(node => {
        taskCenter.send(
          'dom',
          { action: 'removeElement' },
          [node.ref]
        )
      })
    }
    this.children.forEach(node => {
      node.destroy()
    })
    this.children.length = 0
    this.pureChildren.length = 0
  }

  /**
   * Set an attribute, and decide whether the task should be send to native.
   * @param {string} key
   * @param {string | number} value
   * @param {boolean} silent
   */
  setAttr (key, value, silent) {
    if (this.attr[key] === value && silent !== false) {
      return
    }
    this.attr[key] = value
    const taskCenter = getTaskCenter(this.docId)
    if (!silent && taskCenter) {
      const result = {}
      result[key] = value
      taskCenter.send(
        'dom',
        { action: 'updateAttrs' },
        [this.ref, result]
      )
    }
  }

  /**
   * Set batched attributes.
   * @param {object} batchedAttrs
   * @param {boolean} silent
   */
  setAttrs (batchedAttrs, silent) {
    if (isEmpty(batchedAttrs)) return
    const mutations = {}
    for (const key in batchedAttrs) {
      if (this.attr[key] !== batchedAttrs[key]) {
        this.attr[key] = batchedAttrs[key]
        mutations[key] = batchedAttrs[key]
      }
    }
    if (!isEmpty(mutations)) {
      const taskCenter = getTaskCenter(this.docId)
      if (!silent && taskCenter) {
        taskCenter.send(
          'dom',
          { action: 'updateAttrs' },
          [this.ref, mutations]
        )
      }
    }
  }

  /**
   * Set a style property, and decide whether the task should be send to native.
   * @param {string} key
   * @param {string | number} value
   * @param {boolean} silent
   */
  setStyle (key, value, silent) {
    if (this.style[key] === value && silent !== false) {
      return
    }
    this.style[key] = value
    const taskCenter = getTaskCenter(this.docId)
    if (!silent && taskCenter) {
      const result = {}
      result[key] = value
      taskCenter.send(
        'dom',
        { action: 'updateStyle' },
        [this.ref, result]
      )
    }
  }

  /**
   * Set batched style properties.
   * @param {object} batchedStyles
   * @param {boolean} silent
   */
  setStyles (batchedStyles, silent) {
    if (isEmpty(batchedStyles)) return
    const mutations = {}
    for (const key in batchedStyles) {
      if (this.style[key] !== batchedStyles[key]) {
        this.style[key] = batchedStyles[key]
        mutations[key] = batchedStyles[key]
      }
    }
    if (!isEmpty(mutations)) {
      const taskCenter = getTaskCenter(this.docId)
      if (!silent && taskCenter) {
        taskCenter.send(
          'dom',
          { action: 'updateStyle' },
          [this.ref, mutations]
        )
      }
    }
  }

  /**
   * Set style properties from class.
   * @param {object} classStyle
   */
  setClassStyle (classStyle) {
    // reset previous class style to empty string
    for (const key in this.classStyle) {
      this.classStyle[key] = ''
    }

    Object.assign(this.classStyle, classStyle)
    const taskCenter = getTaskCenter(this.docId)
    if (taskCenter) {
      taskCenter.send(
        'dom',
        { action: 'updateStyle' },
        [this.ref, this.toStyle()]
      )
    }
  }

  /**
   * Add an event handler.
   * @param {string} event type
   * @param {function} event handler
   */
  addEvent (type, handler, params) {
    if (!this.event) {
      this.event = {}
    }
    if (!this.event[type]) {
      this.event[type] = { handler, params }
      const taskCenter = getTaskCenter(this.docId)
      if (taskCenter) {
        taskCenter.send(
          'dom',
          { action: 'addEvent' },
          [this.ref, type]
        )
      }
    }
  }

  /**
   * Remove an event handler.
   * @param {string} event type
   */
  removeEvent (type) {
    if (this.event && this.event[type]) {
      delete this.event[type]
      const taskCenter = getTaskCenter(this.docId)
      if (taskCenter) {
        taskCenter.send(
          'dom',
          { action: 'removeEvent' },
          [this.ref, type]
        )
      }
    }
  }

  /**
   * Fire an event manually.
   * @param {string} type type
   * @param {function} event handler
   * @param {boolean} isBubble whether or not event bubble
   * @param {boolean} options
   * @return {} anything returned by handler function
   */
  fireEvent (type, event, isBubble, options) {
    let result = null
    let isStopPropagation = false
    const eventDesc = this.event[type]
    if (eventDesc && event) {
      const handler = eventDesc.handler
      event.stopPropagation = () => {
        isStopPropagation = true
      }
      if (options && options.params) {
        result = handler.call(this, ...options.params, event)
      }
      else {
        result = handler.call(this, event)
      }
    }

    if (!isStopPropagation
      && isBubble
      && (BUBBLE_EVENTS.indexOf(type) !== -1)
      && this.parentNode
      && this.parentNode.fireEvent) {
      event.currentTarget = this.parentNode
      this.parentNode.fireEvent(type, event, isBubble) // no options
    }

    return result
  }

  /**
   * Get all styles of current element.
   * @return {object} style
   */
  toStyle () {
    return Object.assign({}, this.classStyle, this.style)
  }

  /**
   * Convert current element to JSON like object.
   * @return {object} element
   */
  toJSON () {
    const result = {
      ref: this.ref.toString(),
      type: this.type,
      attr: this.attr,
      style: this.toStyle()
    }
    const event = []
    for (const type in this.event) {
      const { params } = this.event[type]
      if (!params) {
        event.push(type)
      }
      else {
        event.push({ type, params })
      }
    }
    if (event.length) {
      result.event = event
    }
    if (this.pureChildren.length) {
      result.children = this.pureChildren.map((child) => child.toJSON())
    }
    return result
  }

  /**
   * Convert to HTML element tag string.
   * @return {stirng} html
   */
  toString () {
    return '<' + this.type +
    ' attr=' + JSON.stringify(this.attr) +
    ' style=' + JSON.stringify(this.toStyle()) + '>' +
    this.pureChildren.map((child) => child.toString()).join('') +
    '</' + this.type + '>'
  }
}

setElement(Element)
