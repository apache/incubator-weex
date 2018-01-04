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

import App from '../../../../../runtime/frameworks/legacy/app'
import { Element, Document } from '../../../../../runtime/vdom'

describe('App Instance', () => {
  const oriDocumentHandler = Document.handler
  const sendTasksSpy = sinon.spy()
  let app

  beforeEach(() => {
    Document.handler = sendTasksSpy
    const id = Date.now() + ''
    app = new App(id, {})
  })

  afterEach(() => {
    Document.handler = oriDocumentHandler
  })

  describe('normal check', () => {
    it('is a class', () => {
      expect(App).to.be.an('function')
    })

    it('being created', () => {
      expect(app).to.be.an('object')
      expect(app).to.be.instanceof(App)
    })

    it('with some apis', () => {
      expect(app.requireModule).a.function
      expect(app.updateActions).a.function
      expect(app.callTasks).a.function
    })

    it('run apis', () => {
      expect(app.requireModule('stream')).to.deep.equal({})
      expect(app.updateActions()).to.be.undefined
      expect(app.callTasks([])).to.be.undefined
    })
  })

  describe('call tasks', () => {
    it('with no args & callback', () => {
      const tasks = [{
        module: 'dom',
        method: 'createBody',
        args: []
      }]

      app.callTasks(tasks)
      expect(sendTasksSpy.lastCall.args[1]).to.deep.equal(tasks)
    })

    it('with callback', (done) => {
      const tasks = [{
        module: 'dom',
        method: 'createBody',
        args: []
      }]

      app.callTasks(tasks)
      expect(sendTasksSpy.lastCall.args[1]).to.deep.equal(tasks)
      done()
    })

    it('with function arg', (done) => {
      const callbackId = '1'

      const tasks = [{
        module: 'dom',
        method: 'createBody',
        args: [() => {}]
      }]

      app.callTasks(tasks)
      expect(sendTasksSpy.lastCall.args[1]).to.deep.equal([{
        module: 'dom',
        method: 'createBody',
        args: [callbackId]
      }])
      done()
    })

    it('with node arg', (done) => {
      const node = new Element()
      node.ref = '1'

      const tasks = [{
        module: 'dom',
        method: 'createBody',
        args: [node]
      }]

      app.callTasks(tasks)
      expect(sendTasksSpy.lastCall.args[1]).to.deep.equal([{
        module: 'dom',
        method: 'createBody',
        args: [node.ref]
      }])
      done()
    })

    it('with callback after close', (done) => {
      const callbackId = '1'

      const tasks = [{
        module: 'dom',
        method: 'createBody',
        args: [() => {}]
      }]

      app.doc.close()

      app.callTasks(tasks)
      expect(sendTasksSpy.lastCall.args[1]).to.deep.equal([{
        module: 'dom',
        method: 'createBody',
        args: [callbackId]
      }])
      done()
    })
  })
})
