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
import websocket from '../../../../render/vue/modules/websocket/websocket'
const TestUrl = 'ws://echo.websocket.org'
  /** @test {webSocket module} */
describe('webSocket module', function () {
  describe('extends Standard WebSocket API', function () {
    context('should inherit', function () {
      let ws = null
      before(() => {
        ws = websocket.WebSocket(TestUrl)
      })
      after(() => {
        websocket.close()
      })
      it('methods', () => {
        expect(websocket, 'should close method to be defined').to.have.property('close')
        expect(websocket, 'should send method to be defined').to.have.property('send')
      })
      it('attributes', () => {
        expect(websocket, 'should onerror to be defined').to.have.property('onerror')
        expect(websocket, 'should onmessage to be defined').to.have.property('onmessage')
        expect(websocket, 'should onopen to be defined').to.have.property('onopen')
        expect(websocket, 'should onclose to be defined').to.have.property('onclose')
        expect(ws, 'should binaryType to be defined').to.have.property('binaryType')
        expect(ws, 'should protocol to be defined').to.have.property('protocol')
        expect(ws, 'should readyState to be defined').to.have.property('readyState')
        expect(ws, 'should url to be defined').to.have.property('url')
      })
      it('constants', () => {
        expect(websocket, 'should INSTANCE to be defined').to.have.property('INSTANCE')
      })
    })
    context('should forward native events', function () {
      let ws = null
      before(() => {
        ws = websocket.WebSocket(TestUrl, 'ws')
      })
      after(() => {
        websocket.close()
      })
      it('open', function () {
        const open = () => {}
        websocket.onopen = open
        expect(ws.onopen).to.be.deep.equal(open)
      })
      it('close', function () {
        const close = () => {}
        websocket.onclose = close
        expect(ws.onclose).to.be.deep.equal(close)
      })
      it('error', function () {
        const error = () => {}
        websocket.onerror = error
        expect(ws.onerror).to.be.deep.equal(error)
      })
      it('message', function () {
        const message = () => {}
        websocket.onmessage = message
        expect(ws.onmessage).to.be.deep.equal(message)
      })
    })
  })
})
