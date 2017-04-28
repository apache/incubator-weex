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
import { getTaskCenter } from './operation'

let Element

export function setElement (El) {
  Element = El
}

/**
 * A map which stores all type of elements.
 * @type {Object}
 */
export const elementTypes = {}

/**
 * Register an extended element type with component methods.
 * @param  {string} type    component type
 * @param  {array}  methods a list of method names
 */
export function registerElement (type, methods) {
  // Skip when no special component methods.
  if (!methods || !methods.length) {
    return
  }

  // Init constructor.
  const XElement = function (props) {
    Element.call(this, type, props, true)
  }

  // Init prototype.
  XElement.prototype = Object.create(Element.prototype)
  Object.defineProperty(XElement.prototype, 'constructor', {
    configurable: false,
    enumerable: false,
    writable: false,
    value: Element
  })

  // Add methods to prototype.
  methods.forEach(methodName => {
    XElement.prototype[methodName] = function (...args) {
      const taskCenter = getTaskCenter(this.docId)
      if (taskCenter) {
        return taskCenter.send('component', {
          ref: this.ref,
          component: type,
          method: methodName
        }, args)
      }
    }
  })

  // Add to element type map.
  elementTypes[type] = XElement
}

/**
 * Clear all element types. Only for testing.
 */
export function clearElementTypes () {
  for (const type in elementTypes) {
    delete elementTypes[type]
  }
}
