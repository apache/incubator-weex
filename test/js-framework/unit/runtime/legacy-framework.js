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
const {
  expect
} = chai
chai.use(sinonChai)

import runtime from '../../../../runtime/api'
import frameworks from '../../../../runtime/frameworks'
import defaultConfig from '../../../../runtime/frameworks/legacy/config'
import { init as resetTaskHandler } from '../../../../runtime/bridge/TaskCenter'

const { init, config } = runtime
config.frameworks = frameworks
runtime.setNativeConsole()

import Vm from '../../../../runtime/frameworks/legacy/vm'
import { clearModules, getModule } from '../../../../runtime/frameworks/legacy/app/register'

const framework = init(config)

function clearRefs (json) {
  delete json.ref
  if (json.children) {
    json.children.forEach(clearRefs)
  }
}

describe('framework entry', () => {
  const oriCallNative = global.callNative
  const oriCallAddElement = global.callAddElement
  const oriDocumentHandler = config.Document.handler
  const callNativeSpy = sinon.spy()
  const callAddElementSpy = sinon.spy()
  const documentHandlerSpy = sinon.spy()
  const instanceId = Date.now() + ''

  before(() => {
    global.callNative = (id, tasks, callbackId) => {
      callNativeSpy(id, tasks, callbackId)
      /* istanbul ignore if */
      if (callbackId !== '-1') {
        framework.callJS(id, [{
          method: 'callback',
          args: [callbackId, null, true]
        }])
      }
    }
    global.callAddElement = (name, id, ref, json, index, callbackId) => {
      callAddElementSpy(name, ref, json, index, callbackId)
      /* istanbul ignore if */
      if (callbackId !== '-1') {
        framework.callJS(id, [{
          method: 'callback',
          args: [callbackId, null, true]
        }])
      }
    }
    config.Document.handler = oriDocumentHandler
    resetTaskHandler()
  })

  afterEach(() => {
    callNativeSpy.reset()
    callAddElementSpy.reset()
    documentHandlerSpy.reset()
  })

  after(() => {
    config.Document.handler = oriDocumentHandler
    global.callNative = oriCallNative
    global.callAddElement = oriCallAddElement
  })

  describe('createInstance', () => {
    it('a simple bundle', () => {
      const code = `
        define('@weex-component/main',
          function(require, exports, module) {
            module.exports = {
              data: function() {
                return {
                  text: 'Hello World',
                  showText: true
                };
              }
            }

            ;module.exports.style = {}

            ;module.exports.template = {
              'type': 'container',
              'children': [{
                'type': 'text',
                'shown': function() {
                  return this.showText
                },
                'attr': {
                  'value': function() {
                    return this.text
                  }
                }
              }]
            }
          }
        )

        bootstrap('@weex-component/main')
      `
      framework.createInstance(instanceId, code)
      expect(callNativeSpy.callCount).to.be.equal(2)
      expect(callAddElementSpy.callCount).to.be.equal(1)

      expect(callNativeSpy.firstCall.args[0]).to.be.equal(instanceId)
      expect(callNativeSpy.firstCall.args[1]).to.deep.equal([{
        module: 'dom',
        method: 'createBody',
        args: [{
          ref: '_root',
          type: 'container',
          attr: {},
          style: {}
        }]
      }])
      // expect(callNativeSpy.firstCall.args[2]).to.not.equal('-1')

      expect(callAddElementSpy.firstCall.args[0]).to.be.equal(instanceId)
      delete callAddElementSpy.firstCall.args[1].ref
      expect(callAddElementSpy.firstCall.args[1]).to.deep.equal({
        type: 'text',
        attr: { value: 'Hello World' },
        style: {}
      })

      // expect(callNativeSpy.secondCall.args[2]).to.not.equal('-1')

      expect(callNativeSpy.secondCall.args[0]).to.be.equal(instanceId)
      expect(callNativeSpy.secondCall.args[1]).to.deep.equal([{
        module: 'dom',
        method: 'createFinish',
        args: []
      }])
      // expect(callNativeSpy.thirdCall.args[2]).to.not.equal('-1')
    })

    it('with a exist instanceId', () => {
      const code = ''
      const result = framework.createInstance(instanceId, code)
      expect(result).to.be.an.instanceof(Error)
    })
  })

  describe('getRoot', () => {
    it('with a exist instanceId', () => {
      const json = framework.getRoot(instanceId)
      expect(json.ref).eql('_root')
      clearRefs(json)
      const expectJSON = {
        type: 'container',
        attr: {},
        style: {},
        children: [{
          type: 'text',
          attr: {
            value: 'Hello World'
          },
          style: {}
        }]
      }
      expect(json).to.be.deep.equal(expectJSON)
    })
  })

  describe('callJS', () => {
    it('fireEvent with no params', () => {
      framework.callJS()
    })

    it('fireEvent with a exist instanceId', () => {
      framework.callJS(instanceId, [{
        method: 'fireEvent',
        args: []
      }])
    })

    it('callback with a exist instanceId', () => {
      framework.callJS(instanceId, [{
        method: 'callback',
        args: []
      }])
    })

    it('with a non-exist instanceId', () => {
      const result = framework.callJS('123', [{
        method: 'fireEvent',
        args: []
      }])
      expect(result).to.be.an.instanceof(Error)
    })

    it('with a non-array tasks', () => {
      const result = framework.callJS(instanceId, {
        method: 'fireEvent',
        args: []
      })
      expect(result).to.be.an.instanceof(Error)
    })
  })

  describe('refreshInstance', () => {
    it('modify showText to false', () => {
      const json = framework.getRoot(instanceId)
      const textRef = json.children[0].ref
      framework.refreshInstance(instanceId, { showText: false })
      expect(callNativeSpy.callCount).to.be.equal(2)
      expect(callNativeSpy.firstCall.args[0]).to.be.equal(instanceId)
      expect(callNativeSpy.firstCall.args[1]).to.deep.equal([{
        module: 'dom',
        method: 'removeElement',
        args: [textRef]
      }])
      // expect(callNativeSpy.firstCall.args[2]).to.be.equal('-1')

      expect(callNativeSpy.secondCall.args[0]).to.be.equal(instanceId)
      expect(callNativeSpy.secondCall.args[1]).to.deep.equal([{
        module: 'dom',
        method: 'refreshFinish',
        args: []
      }])
      // expect(callNativeSpy.secondCall.args[2]).to.be.equal('-1')
    })

    it('with a non-exist instanceId', () => {
      const result = framework.refreshInstance('123', {})
      expect(result).to.be.an.instanceof(Error)
    })
  })

  describe('destroyInstance', () => {
    it('with no params', () => {
      framework.destroyInstance()
    })

    it('with a exist instanceId', () => {
      const result = framework.destroyInstance(instanceId)
      expect(result[instanceId]).to.be.undefined
    })

    it('with a non-exist instanceId', () => {
      const result = framework.destroyInstance('123')
      expect(result).to.be.an.instanceof(Error)
    })
  })

  describe('registerComponents', () => {
    it('with old format', () => {
      framework.registerComponents(['a', 'b', 'c'])
      expect(defaultConfig.nativeComponentMap).to.contain.keys('a', 'b', 'c')
    })

    it('with new format', () => {
      framework.registerComponents([{
        type: 'd',
        append: 'tree'
      }])
      expect(defaultConfig.nativeComponentMap).to.contain.keys('d')
      expect(defaultConfig.nativeComponentMap['d']).to.be.deep.equal({
        type: 'd',
        append: 'tree'
      })
    })

    it('with non-array', () => {
      framework.registerComponents({
        type: 'e'
      })
      expect(defaultConfig.nativeComponentMap).not.contain.keys('e')
    })

    it('with methods', () => {
      const components = [{
        type: 'x',
        methods: ['foo', 'bar']
      }, {
        type: 'y',
        methods: []
      }, {
        type: 'z',
        methods: null
      }]
      framework.registerComponents(components)
      expect(defaultConfig.nativeComponentMap).to.contain.keys('x', 'y', 'z')
    })
  })

  describe('register modules', () => {
    it('with object of modules', () => {
      clearModules()
      const modules = {
        a: [{
          name: 'b',
          args: ['string']
        }]
      }

      framework.registerModules(modules)
      expect(getModule('b')).an.object
      clearModules()
    })
  })

  describe('register methods', () => {
    it('with object of methods', () => {
      const methods = {
        a: sinon.spy()
      }

      framework.registerMethods(methods)
      expect(Vm.prototype.a).a.function
      delete Vm.prototype.a
    })
  })
})

describe('config', () => {
  it('config is an object', () => {
    init({})
  })
})
