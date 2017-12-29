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
import chai from 'chai'
const { expect } = chai

import * as modules from '../../../../../runtime/frameworks/legacy/api/modules'
import { initModules, requireModule, clearModules } from '../../../../../runtime/frameworks/legacy/app/register'

describe('built-in modules', () => {
  before(() => {
    clearModules()
  })

  after(() => {
    clearModules()
  })

  it('have keys', () => {
    const app = {}
    initModules(modules)
    expect(requireModule(app, 'dom')).to.have.all.keys('scrollToElement')
    expect(requireModule(app, 'stream')).to.have.all.keys('sendHttp')
    expect(requireModule(app, 'event')).to.have.all.keys('openURL')
    expect(requireModule(app, 'pageInfo')).to.have.all.keys('setTitle')
    expect(requireModule(app, 'animation')).to.have.all.keys('transition')
  })
})
