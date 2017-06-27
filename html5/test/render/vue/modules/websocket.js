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
        expect(websocket.close, 'to be defined');
        expect(websocket.send, 'to be defined');
      });
      it('attributes', function() {
        expect(websocket.onerror, 'to be defined');
        expect(websocket.onmessage, 'to be defined');
        expect(websocket.onopen, 'to be defined');
        expect(websocket.onclose, 'to be defined');
        expect(ws.binaryType, 'to be defined');
        expect(ws.bufferedAmount, 'to be defined');
        expect(ws.extensions, 'to be defined');
        expect(ws.protocol, 'to be defined');
        expect(ws.readyState, 'to be defined');
        expect(ws.url, 'to be defined');
      });
 
      it('constants', function() {
        expect(websocket.INSTANCE, 'to be defined');
      });
    });
 
    context('should forward native events', function() {
      let ws;
 
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