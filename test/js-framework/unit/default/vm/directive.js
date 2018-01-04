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
  applyNaitveComponentOptions,
  bindSubVm,
  bindSubVmAfterInitialized
} from '../../../../../runtime/frameworks/legacy/vm/directive'

import {
  initState
} from '../../../../../runtime/frameworks/legacy/core/state'
import config from '../../../../../runtime/frameworks/legacy/config'

const { nativeComponentMap } = config
const directive = {}

function extendVm (vm, methodNames) {
  methodNames.forEach((name) => {
    vm[name] = directive[name]
  })
  initState(vm)
}

function initElement (el) {
  el.setAttr = function (k, v) { this.attr[k] = v }
  el.setStyle = function (k, v) { this.style[k] = v }
  el.setClassStyle = function (style) { this.classStyle = style }
  el.addEvent = function (t, h) {
    this.event[t] = h
  }
}

describe('apply component options', () => {
  it('apply top prop', () => {
    nativeComponentMap['test-apply'] = {
      type: 'test-apply1',
      append: 'tree'
    }
    const template = {
      type: 'test-apply'
    }
    applyNaitveComponentOptions(template)
    expect(template.type).to.be.equal('test-apply')
    expect(template.append).to.be.equal('tree')

    delete nativeComponentMap['test-apply']
  })

  it('apply a object', () => {
    nativeComponentMap['test-apply'] = {
      classList: ['c'],
      attr: {
        a: 'a',
        b: 'b'
      }
    }
    const spy = sinon.spy()
    const template = {
      type: 'test-apply',
      classList: spy,
      attr: {
        b: '2'
      }
    }
    applyNaitveComponentOptions(template)

    expect(template).to.deep.equal({
      type: 'test-apply',
      classList: spy,
      attr: {
        a: 'a',
        b: '2'
      }
    })

    delete nativeComponentMap['test-apply']
  })
})

// exports._bindSubVm(subVm, template)
describe('bind external infomations to sub vm', () => {
  let vm, subVm
  // const methodNames = [
  //   '_watch', '_bindKey', '_bindDir',
  //   '_setId', '_setAttr', '_setClass', '_setStyle',
  //   '_setEvent', '_bindEvents', '_bindElement',
  //   '_bindSubVm', '_bindSubVmAfterInitialized']
  beforeEach(() => {
    vm = {
      _data: { a: 1, b: 2, c: 'class-style1' },
      _watchers: [],
      _app: { eventManager: { add: () => {} }},
      _options: {
        style: {
          'class-style1': {
            aaa: 1,
            bbb: 2
          },
          'class-style2': {
            aaa: 2,
            ccc: 3
          }
        }
      },
      foo: function () {}
    }
    extendVm(vm, [])
    subVm = {
      _options: {
        props: {
          a: String,
          b: String
        }
      }
    }
  })

  it('bind to no-root-element sub vm', () => {
    bindSubVm(vm, subVm, {
      attr: { a: 3, c: 4 },
      style: { a: 2 },
      events: { click: 'foo' }
    })
    expect(subVm.a).eql(3)
    expect(subVm.c).is.undefined
    expect(subVm._rootEl).is.undefined
  })

  it('bind props with external data', () => {
    bindSubVm(vm, subVm, {
      attr: { a: function () { return this.b } }
    })
    expect(subVm.a).eql(2)
  })

  it('bind styles to a sub vm with root element', () => {
    subVm._rootEl = {
      attr: {},
      style: {},
      event: []
    }
    const template = {
      style: { aaa: 2, bbb: function () { return this.a } }
    }
    initElement(subVm._rootEl)
    bindSubVm(vm, subVm, template)
    bindSubVmAfterInitialized(vm, subVm, template)
    expect(subVm._rootEl.style.aaa).eql(2)
    expect(subVm._rootEl.style.bbb).eql(1)
    vm.a = 3
    expect(subVm._rootEl.style.bbb).eql(3)
  })

  it('bind simply classlist to a sub vm with root element', () => {
    subVm._rootEl = {
      attr: {},
      style: {},
      event: []
    }
    const template = {
      classList: ['class-style1']
    }
    initElement(subVm._rootEl)
    bindSubVm(vm, subVm, template)
    bindSubVmAfterInitialized(vm, subVm, template)
    expect(subVm._rootEl.classStyle.aaa).eql(1)
    expect(subVm._rootEl.classStyle.bbb).eql(2)
  })

  it('bind classlist to a sub vm with root element', () => {
    subVm._rootEl = {
      attr: {},
      style: {},
      event: []
    }
    const template = {
      classList: function () {
        return [this.c]
      }
    }
    initElement(subVm._rootEl)
    bindSubVm(vm, subVm, template)
    bindSubVmAfterInitialized(vm, subVm, template)
    expect(subVm._rootEl.classStyle.aaa).eql(1)
    expect(subVm._rootEl.classStyle.bbb).eql(2)
    vm.c = 'class-style2'
    expect(subVm._rootEl.classStyle.aaa).eql(2)
    expect(subVm._rootEl.classStyle.bbb).to.be.undefined
    expect(subVm._rootEl.classStyle.ccc).eql(3)
  })

  it('bind events to a sub vm with root element', () => {
    subVm._rootEl = {
      attr: {},
      style: {},
      event: {}
    }
    const template = {
      events: { click: 'foo' }
    }
    initElement(subVm._rootEl)
    bindSubVm(vm, subVm, template)
    bindSubVmAfterInitialized(vm, subVm, template)
    // expect(subVm._rootEl.event.click).a('function')
  })
})
