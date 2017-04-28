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
import * as check from '../../../../render/vue/validator/check'

describe('check', () => {
  const { flatten, checkSupported, isSupportedStyle, isSupportedProp } = check
  it('flatten', () => {
    expect(flatten([])).to.deep.equal([])
    expect(flatten(['a', 'b'])).to.deep.equal(['a', 'b'])
    expect(flatten([['a'], ['b']])).to.deep.equal(['a', 'b'])
    expect(flatten(['a', ['b'], 'c'])).to.deep.equal(['a', 'b', 'c'])
    expect(flatten([['a', 'b'], 'c', ['d', 'e', 'f']])).to.deep.equal(['a', 'b', 'c', 'd', 'e', 'f'])
    expect(flatten([['a', ['b']], 'c', [['d', ['e']], 'f']])).to.deep.equal(['a', 'b', 'c', 'd', 'e', 'f'])
  })

  it('checkSupported', () => {
    const list = {
      '@shared': ['a', 'b'],
      custom: ['@shared', 'c']
    }

    expect(checkSupported('custom', 'a', list)).to.be.true
    expect(checkSupported('custom', 'b', list)).to.be.true
    expect(checkSupported('custom', 'c', list)).to.be.true
    expect(checkSupported('custom', 'd', list)).to.be.false
    expect(checkSupported('unknown', 'a', list)).to.be.false
  })

  it('isSupportedStyle', () => {
    expect(isSupportedStyle('text', 'width')).to.be.true
    expect(isSupportedStyle('text', 'color')).to.be.true
    expect(isSupportedStyle('text', 'border')).to.be.false
  })

  it('isSupportedProp', () => {
    expect(isSupportedProp('text', 'width')).to.be.false
    expect(isSupportedProp('text', 'border')).to.be.false
  })
})
