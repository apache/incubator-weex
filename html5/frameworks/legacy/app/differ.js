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
export default class Differ {
  constructor (id) {
    this.id = id
    this.map = []
    this.hooks = []
  }
  isEmpty () {
    return this.map.length === 0
  }
  append (type, depth = 0, ref, handler) {
    if (!this.hasTimer) {
      this.hasTimer = true
      setTimeout(() => {
        this.hasTimer = false
        this.flush(true)
      }, 0)
    }
    const map = this.map
    if (!map[depth]) {
      map[depth] = {}
    }
    const group = map[depth]
    if (!group[type]) {
      group[type] = {}
    }
    if (type === 'element') {
      if (!group[type][ref]) {
        group[type][ref] = []
      }
      group[type][ref].push(handler)
    }
    else {
      group[type][ref] = handler
    }
  }
  flush (isTimeout) {
    const map = this.map.slice()
    this.map.length = 0
    map.forEach((group) => {
      callTypeMap(group, 'repeat')
      callTypeMap(group, 'shown')
      callTypeList(group, 'element')
    })

    const hooks = this.hooks.slice()
    this.hooks.length = 0
    hooks.forEach((fn) => {
      fn()
    })

    if (!this.isEmpty()) {
      this.flush()
    }
  }
  then (fn) {
    this.hooks.push(fn)
  }
}

function callTypeMap (group, type) {
  const map = group[type]
  for (const ref in map) {
    map[ref]()
  }
}

function callTypeList (group, type) {
  const map = group[type]
  for (const ref in map) {
    const list = map[ref]
    list.forEach((handler) => { handler() })
  }
}
