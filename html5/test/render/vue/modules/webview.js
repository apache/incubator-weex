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
import * as webview from '../../../../render/vue/modules/webview'
describe('webview module', () => {
  const gobackSpy = sinon.spy()
  const reloadSpy = sinon.spy()
  const goforwardSpy = sinon.spy()
  const vnode = {
    goBack: gobackSpy,
    reload: reloadSpy,
    goForward: goforwardSpy
  }
  it('should goBack be worked', () => {
    const {
      goBack
    } = webview.default
    expect(goBack).to.be.a('function')
    goBack([vnode])
    expect(gobackSpy.callCount).to.be.equal(1)
  })
  it('should reload be worked', () => {
    const {
      reload
    } = webview.default
    expect(reload).to.be.a('function')
    reload([vnode])
    expect(reloadSpy.callCount).to.be.equal(1)
  })
  it('should goForward be worked', () => {
    const {
      goForward
    } = webview.default
    expect(goForward).to.be.a('function')
    goForward([vnode])
    expect(goforwardSpy.callCount).to.be.equal(1)
  })
})
