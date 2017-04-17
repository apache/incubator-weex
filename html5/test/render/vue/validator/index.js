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
import * as validator from '../../../../render/vue/validator'

describe('validator', function () {
  const { validateStyles, validateProps, configure } = validator
  const warn = function () {}

  before(() => {
    configure({ silent: false })
    configure({ silent: true, onfail: warn })
  })

  it('validateStyles', () => {
    const styles = {
      position: 'absolute',
      color: '#dddddd'
    }

    expect(validateStyles).to.be.a('function')
    expect(validateStyles('text', styles)).to.be.true
    expect(validateStyles('text', { opacity: 0.2 })).to.be.true
  })

  it('validateStyles (invalid)', () => {
    expect(validateStyles).to.be.a('function')
    expect(validateStyles('unknown', { color: 'blue' })).to.be.true
    expect(validateStyles('text', { unknown: 'nothing' })).to.be.false
    expect(validateStyles('text', { color: '#ABCC' })).to.be.false
    expect(validateStyles('text', { color: '#ABCC' })).to.be.false
  })

  it('validateProps', () => {
    const props = {
      isString: 'absolute',
      style: 'color: #dddddd'
    }

    expect(validateProps).to.be.a('function')
    expect(validateProps('unknown', { whatever: 'nothing' })).to.be.true
    expect(validateProps('unknown', props)).to.be.true
    expect(validateProps('unknown', { isString: false })).to.be.false
  })
})
