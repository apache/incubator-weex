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
/*global Event*/
import * as globalEvent from '../../../../render/vue/modules/globalEvent'
describe('globalEvent module', () => {
  const callback = sinon.spy()
  it('should addEventListener be worked', () => {
    const {
      addEventListener
    } = globalEvent.default
    const event = 'click'
    expect(addEventListener).to.be.a('function')
    addEventListener(event, callback)
    expect(callback.called).to.be.false
    document.dispatchEvent(new Event(event))
    expect(callback.called).to.be.true
    expect(callback.callCount).to.be.equal(1)
  })
  it('should removeEventListener be worked', () => {
    const {
      removeEventListener
    } = globalEvent.default
    const event = 'click'
    expect(removeEventListener).to.be.a('function')
    removeEventListener(event)
    document.dispatchEvent(new Event(event))
    expect(callback.callCount).to.be.equal(1)
  })
})
