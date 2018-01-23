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

import { expect } from 'chai'
import * as utils from '../../../../runtime/shared/utils'

describe('utils', () => {
  it('isEmpty', () => {
    expect(utils.isEmpty()).to.be.true
    expect(utils.isEmpty(0)).to.be.true
    expect(utils.isEmpty(null)).to.be.true
    expect(utils.isEmpty(undefined)).to.be.true
    expect(utils.isEmpty(false)).to.be.true
    expect(utils.isEmpty(true)).to.be.true
    expect(utils.isEmpty(NaN)).to.be.true
    expect(utils.isEmpty([])).to.be.true
    expect(utils.isEmpty({})).to.be.true
    expect(utils.isEmpty(Object.create(null))).to.be.true
    expect(utils.isEmpty(Object.create({}))).to.be.true
    expect(utils.isEmpty(Object.create({ x: '' }))).to.be.true

    expect(utils.isEmpty({ abc: '' })).to.be.false
    expect(utils.isEmpty([0])).to.be.false
  })
})
