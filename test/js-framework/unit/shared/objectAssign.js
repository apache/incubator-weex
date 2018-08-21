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

import '../../../../runtime/shared/polyfill/objectAssign'

describe('object.assign', () => {
  it('is a function', () => {
    expect(Object.assign).is.an('function')
  })

  it('source to taget', () => {
    const target = {
      a: 'a',
      b: 'b'
    }
    const src1 = {
      b: 'bb',
      c: 'c'
    }
    const src2 = {
      d: 'd',
      a: 'aa'
    }
    Object.assign(target, src1, src2)
    expect(target).to.deep.equal({
      a: 'aa',
      b: 'bb',
      c: 'c',
      d: 'd'
    })
  })
})
