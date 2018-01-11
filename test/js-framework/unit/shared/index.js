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
import sinon from 'sinon'
import sinonChai from 'sinon-chai'
const { expect } = chai
chai.use(sinonChai)

import * as shared from '../../../../runtime/shared/'

describe('a polyfill of', () => {
  it('Promise', () => {
    expect(typeof Promise).to.be.equal('function')
    expect(() => {
      new Promise(sinon.spy()).then(sinon.spy())
      Promise.all()
      Promise.race()
      Promise.resolve()
      Promise.reject()
    }).to.not.throw
  })

  it('Object.assign', () => {
    expect(typeof Object.assign).to.be.equal('function')
  })

  it('setTimeout', () => {
    expect(typeof setTimeout).to.be.equal('function')
  })

  it('console.log', () => {
    expect(typeof console.log).to.be.equal('function')
  })
})

describe('freeze the prototype of build-in objects', function () {
  before(() => {
    shared.freezePrototype()
  })

  it('Object & Object.prototype', () => {
    expect(Object).to.be.frozen
    // expect(Object.prototype).to.be.frozen
  })
  it('Array & Array.prototype', () => {
    expect(Array).to.be.frozen
    expect(Array.prototype).to.be.frozen
  })

  it('String.prototype', () => { expect(String.prototype).to.be.frozen })
  it('Number.prototype', () => { expect(Number.prototype).to.be.frozen })
  it('Boolean.prototype', () => { expect(Boolean.prototype).to.be.frozen })
  it('Error.prototype', () => { expect(Error.prototype).to.be.frozen })
  it('Date.prototype', () => { expect(Date.prototype).to.be.frozen })
  it('Math.prototype', () => { expect(Math.prototype).to.be.frozen })
  it('RegExp.prototype', () => { expect(RegExp.prototype).to.be.frozen })
})
