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

import Weex from './render/index'

import root from './base/root'
import div from './base/div'
import droot from './base/droot'

// install the extended apis and components.
import extensions from './extend'

Weex.install(root)
Weex.install(div)
Weex.install(droot)

Weex.install(extensions)

global.registerComponents([
  { type: 'input', methods: ['focus', 'blur'] }
])

export default Weex
