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
import App from '../app/index'
import { instanceMap } from './map'
import { init as initApp } from '../app/ctrl/index'
import { resetTarget } from '../core/dep'

/**
 * Create a Weex instance.
 *
 * @param  {string} id
 * @param  {string} code
 * @param  {object} options
 *         option `HAS_LOG` enable print log
 * @param  {object} data
 * @param  {object} info { created, ... services }
 */
export function createInstance (id, code, options, data, info) {
  const { services } = info || {}
  resetTarget()
  let instance = instanceMap[id]
  /* istanbul ignore else */
  options = options || {}
  let result
  /* istanbul ignore else */
  if (!instance) {
    instance = new App(id, options)
    instanceMap[id] = instance
    result = initApp(instance, code, data, services)
  }
  else {
    result = new Error(`invalid instance id "${id}"`)
  }
  return (result instanceof Error) ? result : instance
}
