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
const TestUrl = 'ws://echo.websocket.org';
/** @test {webSocket module} */
describe('webSocket module', function() {
  after(function(done) {
    const ws = new WebSocket(TestUrl);
    ws.addEventListener('open', function() {
      ws.send('shutdown');
      done();
    });
  });
  describe('extends Standard WebSocket API', function() {
    context('should inherit', function() {
      let ws = websocket.WebSocket(TestUrl);
      it('methods', function() {
        expect(websocket, 'should close method to be defined').to.have.property('close');
        expect(websocket, 'should send method to be defined').to.have.property('send');
      });
      it('attributes', function() {
        expect(websocket, 'should onerror to be defined').to.have.property('onerror');
        expect(websocket, 'should onmessage to be defined').to.have.property('onmessage');
        expect(websocket, 'should onopen to be defined').to.have.property('onopen');
        expect(websocket, 'should onclose to be defined').to.have.property('onclose');
        expect(ws, 'should binaryType to be defined').to.have.property('binaryType');
        expect(ws, 'should bufferdAmount to be defined').to.have.property('bufferedAmount');
        expect(ws, 'should extensions to be defined').to.have.property('extensions');
        expect(ws, 'should protocol to be defined').to.have.property('protocol');
        expect(ws, 'should readyState to be defined').to.have.property('readyState');
        expect(ws, 'should url to be defined').to.have.property('url');
      });
 
      it('constants', function() {
        expect(websocket.INSTANCE, 'to be defined');
      });
    });
 
    context('should forward native events', function() {
      let ws;
      this.timeout(8000)
      beforeEach(function() {
        ws = websocket.WebSocket(TestUrl);
      });
 
      afterEach(function() {
        websocket.close();
      });
 
      it('open', function(done) {
        websocket.onopen = function(){
          done();
        }
      });
      it('close', function(done) {
        let closed = false;
        ws.onclose = function(){
          if (!closed) {
            done();
            closed = true;
          }
        }
        websocket.onopen = function(){
          websocket.close()
        }
      });
 
      it('message', function(done) {
        const message = 'Test';
        websocket.onmessage = function(e){
          expect(e.data).to.be.equal(message);
          done();
        }
        websocket.onopen = function(){
          websocket.send(message);
        }
      });
    });
    describe('should ignore',function(){
        it('protocol is undefined', function(done) {
          let ws = websocket.WebSocket(TestUrl)
          expect(websocket.INSTANCE).not.to.be.null;
          done();
        });
 
        it('url is undefined', function(done) {
          let ws = websocket.WebSocket('')
          expect(websocket.INSTANCE).to.be.null;
          done();
        });

        it('both url and protocol is defined', function(done) {
          let ws = websocket.WebSocket(TestUrl,'ws')
          expect(websocket.INSTANCE).not.to.be.null;
          done();
        });
    })
  });
});