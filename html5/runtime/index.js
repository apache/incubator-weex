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
 * Register framework(s) in JS runtime. Weex supply two layers for 3rd-party
 * framework(s): one is the instance management layer, another is the
 * virtual-DOM layer.
 */

import * as shared from '../shared'

import init from './init'
import config from './config'

import {
  register,
  unregister,
  has
} from './service'

/* istanbul ignore next */
function freezePrototype () {
  shared.freezePrototype()

  Object.freeze(config.Element)
  Object.freeze(config.Comment)
  Object.freeze(config.Listener)
  Object.freeze(config.Document.prototype)
  Object.freeze(config.Element.prototype)
  Object.freeze(config.Comment.prototype)
  Object.freeze(config.Listener.prototype)
}

export default {
  setNativeConsole: shared.setNativeConsole,
  resetNativeConsole: shared.resetNativeConsole,
  setNativeTimer: shared.setNativeTimer,
  resetNativeTimer: shared.resetNativeTimer,
  service: { register, unregister, has },
  freezePrototype,
  init,
  config
}
