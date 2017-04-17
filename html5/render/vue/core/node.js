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
 * remove text nodes in the nodes array.
 * @param  {Array} nodes
 * @return {Array} nodes without text nodes.
 */
export function trimTextVNodes (vnodes) {
  if (Array.isArray(vnodes)) {
    return vnodes.filter(vnode => !!vnode.tag)
  }
  return vnodes
}

const supportedEvents = [
  'click', 'longpress', 'appear', 'disappear'
  // 'touchstart', 'touchmove', 'touchend'
]
/**
 * emit native events to enable v-on.
 * @param {VComponent} context: which one to emit a event on.
 * @param {array} extras: extra events.
 */
export function createEventMap (context, extras = []) {
  const eventMap = {}
  supportedEvents.concat(extras).forEach(name => {
    eventMap[name] = event => context.$emit(name, event)
  })
  return eventMap
}
