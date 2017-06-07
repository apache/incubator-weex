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
 * Webview module
 */
import { isArray } from '../utils'
export default {
  goBack (vnode) {
    if (isArray(vnode)) {
      if (process.env.NODE_ENV === 'development') {
        console.warn('[vue-render] the ref passed to animation.transitionOnce is a array.')
      }
      vnode = vnode[0]
    }
    if (vnode && typeof vnode.goBack === 'function') {
      vnode.goBack()
    }
  },
  goForward (vnode) {
    if (isArray(vnode)) {
      if (process.env.NODE_ENV === 'development') {
        console.warn('[vue-render] the ref passed to animation.transitionOnce is a array.')
      }
      vnode = vnode[0]
    }
    if (vnode && typeof vnode.goForward === 'function') {
      vnode.goForward()
    }
  },
  reload (vnode) {
    if (isArray(vnode)) {
      if (process.env.NODE_ENV === 'development') {
        console.warn('[vue-render] the ref passed to animation.transitionOnce is a array.')
      }
      vnode = vnode[0]
    }
    if (vnode && typeof vnode.reload === 'function') {
      vnode.reload()
    }
  }
}
