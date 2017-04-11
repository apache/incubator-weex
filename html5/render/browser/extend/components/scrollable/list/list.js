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

import scrollableModule from '../scrollable'

const availableChildrenTypes = ['cell', 'loading', 'refresh']

function getProto (Scrollable) {
  return {
    create () {
      const node = Scrollable.prototype.create.call(this)
      node.classList.add('list-wrap')
      this.scrollElement.classList.add('list-element')
      return node
    },

    createChildren () {
      const children = this.data.children || []
      children.forEach(function (data) {
        const type = data.type
        if (availableChildrenTypes.indexOf(type) === -1) {
          // throw new Error('[h5-render] invalid child type "'
          //   + type + '" for list.')
          console.warn('[h5-render] invalid child type "'
            + type + '" for list.')
        }
      })
      return Scrollable.prototype.createChildren.call(this)
    },

    appendChild (data) {
      const type = data.type
      if (availableChildrenTypes.indexOf(type) === -1) {
        // throw new Error('[h5-render] invalid child type "'
        //   + type + '" for list.')
        console.warn('[h5-render] invalid child type "'
          + type + '" for list.')
      }
      return Scrollable.prototype.appendChild.call(this, data)
    },

    insertBefore (child, before) {
      const type = child.data.type
      if (availableChildrenTypes.indexOf(type) === -1) {
        // throw new Error('[h5-render] invalid child type "'
        //   + type + '" for list.')
        console.warn('[h5-render] invalid child type "'
          + type + '" for list.')
      }
      return Scrollable.prototype.insertBefore.call(this, child, before)
    }
  }
}

function init (Weex) {
  const Scrollable = scrollableModule.init(Weex)
  function List (data, nodeType) {
    Scrollable.call(this, data, nodeType)
  }
  const extend = Weex.utils.extend

  List.prototype = Object.create(Scrollable.prototype)
  extend(List.prototype, getProto(Scrollable))

  return List
}

export default { init }
