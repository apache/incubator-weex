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

import {
  getModule,
  clearModules,
  initModules,
  initMethods,
  requireModule,
  requireCustomComponent,
  registerCustomComponent
} from '../../../../../runtime/frameworks/legacy/app/register'

function Ctx () {
  this.customComponentMap = {}
  this.callTasks = sinon.spy()
}

describe('register', () => {
  let ctx

  before(() => {
    clearModules()
    ctx = new Ctx()
  })

  after(() => {
    clearModules()
  })

  describe('component', () => {
    it('with exports', () => {
      const def = {
        a: 'b'
      }
      registerCustomComponent(ctx, 'componentA', def)
      expect(requireCustomComponent(ctx, 'componentA')).to.deep.equal(def)
    })

    it('with a existing name', () => {
      const def = {
        a: 'b'
      }
      sinon.stub(console, 'error')
      registerCustomComponent(ctx, 'componentA', def)
      expect(console.error).callCount(1)
      console.error.restore()
    })
  })

  describe('module', () => {
    it('with a old format', () => {
      initModules({
        dom: [
          'createBody',
          'addElement'
        ]
      })
      expect(requireModule(ctx, 'dom')).to.have.any.keys('createBody', 'addElement')
    })

    it('with a new format', () => {
      initModules({
        dom: [
          {
            name: 'moveElement',
            args: ['string', 'string']
          }
        ],
        stream: [
          {
            name: 'sendMtop',
            args: ['object', 'function']
          }
        ]
      })
      expect(requireModule(ctx, 'dom')).to.have.all.keys('createBody', 'addElement', 'moveElement')
      expect(requireModule(ctx, 'stream')).to.have.all.keys('sendMtop')
    })

    it('with a existed module.method', () => {
      initModules({
        dom: [
          {
            name: 'moveElement',
            args: ['string', 'string', 'string']
          }
        ]
      }, true)
      initModules({
        stream: [
          {
            name: 'sendMtop',
            args: ['object']
          }
        ]
      })
      expect(getModule('dom').moveElement).to.deep.equal({
        name: 'moveElement',
        args: ['string', 'string', 'string']
      })

      expect(getModule('stream').sendMtop).to.deep.equal({
        name: 'sendMtop',
        args: ['object', 'function']
      })
    })

    it('run registered module', () => {
      initModules({
        event: [{
          name: 'openURL',
          args: ['string']
        }]
      })
      const event = requireModule(ctx, 'event')
      expect(event).to.have.keys('openURL')

      event.openURL('http://test.com')
      expect(ctx.callTasks.callCount).to.be.equal(1)
    })
  })

  describe('api', () => {
    it('a common api', () => {
      initMethods(Ctx, {
        $test1: function () {
          return {
            ctx: this,
            value: 'test1'
          }
        },
        $test2: function () {
          return {
            ctx: this,
            value: 'test2'
          }
        }
      })

      expect(ctx.$test1).to.be.a('Function')
      expect(ctx.$test1()).to.deep.equal({
        ctx: ctx,
        value: 'test1'
      })
      expect(ctx.$test2).to.be.an('Function')
      expect(ctx.$test2()).to.deep.equal({
        ctx: ctx,
        value: 'test2'
      })
    })

    it('override api', () => {
      initMethods(Ctx, {
        $override: function () {
          return {
            ctx: this,
            value: 'first'
          }
        }
      })

      initMethods(Ctx, {
        $override: function () {
          return {
            ctx: this,
            value: 'ignored'
          }
        }
      })

      expect(ctx.$override).to.be.a('Function')
      expect(ctx.$override()).to.deep.equal({
        ctx: ctx,
        value: 'first'
      })
    })
  })
})
