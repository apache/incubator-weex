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
/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2013-2015 Yuxi Evan You
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/
/* eslint-disable */


import Watcher from './watcher'
import Dep from './dep'
import {
  observe,
  proxy,
  unproxy
} from './observer'
import {
  isPlainObject,
  bind
} from '../util/index'

export function initState (vm) {
  vm._watchers = []
  initData(vm)
  initComputed(vm)
  initMethods(vm)
}

export function initData (vm) {
  let data = vm._data

  if (!isPlainObject(data)) {
    data = {}
  }
  // proxy data on instance
  const keys = Object.keys(data)
  let i = keys.length
  while (i--) {
    proxy(vm, keys[i])
  }
  // observe data
  observe(data, vm)
}

/* istanbul ignore next */
function noop () {
}

export function initComputed (vm) {
  const computed = vm._computed
  if (computed) {
    for (let key in computed) {
      const userDef = computed[key]
      const def = {
        enumerable: true,
        configurable: true
      }
      if (typeof userDef === 'function') {
        def.get = makeComputedGetter(userDef, vm)
        def.set = noop
      } else {
        def.get = userDef.get
          ? userDef.cache !== false
            ? makeComputedGetter(userDef.get, vm)
            : bind(userDef.get, vm)
          : noop
        def.set = userDef.set
          ? bind(userDef.set, vm)
          : noop
      }
      Object.defineProperty(vm, key, def)
    }
  }
}

function makeComputedGetter (getter, owner) {
  const watcher = new Watcher(owner, getter, null, {
    lazy: true
  })
  return function computedGetter () {
    if (watcher.dirty) {
      watcher.evaluate()
    }
    if (Dep.target) {
      watcher.depend()
    }
    return watcher.value
  }
}

export function initMethods (vm) {
  const methods = vm._methods
  if (methods) {
    for (let key in methods) {
      vm[key] = methods[key]
    }
  }
}
