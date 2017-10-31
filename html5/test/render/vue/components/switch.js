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
import { init } from '../helper'
import _switch from '../../../../render/vue/components/switch'

init('<switch> component', (Vue, helper) => {
  const { utils, compile } = helper

  before(() => {
    helper.register('switch', _switch)
  })

  it('simple <switch> component', () => {
    const vm = compile(`<switch></switch>`)
    expect(vm.$el.tagName.toLowerCase()).to.be.equal('span')
    expect(utils.toArray(vm.$el.classList)).to.include('weex-switch')
  })

  it('disabled <switch>', () => {
    const vmA = compile(`<switch disabled="true"></switch>`)
    const vmB = compile(`<switch disabled></switch>`)
    const vmC = compile(`<switch disabled="disabled"></switch>`)
    expect(vmA.$el.className).to.match(/weex\-switch\-disabled/)
    expect(vmB.$el.className).to.match(/weex\-switch\-disabled/)
    expect(vmC.$el.className).to.match(/weex\-switch\-disabled/)
  })

  it('enabled <switch>', () => {
    const vmA = compile(`<switch></switch>`)
    const vmB = compile(`<switch disabled="false"></switch>`)

    expect(vmA.$el.className).to.match(/weex-switch/)
    expect(vmB.$el.className).to.match(/weex-switch/)
  })

  it('checked <switch>', () => {
    const vmA = compile(`<switch checked="true"></switch>`)
    const vmB = compile(`<switch checked></switch>`)
    const vmC = compile(`<switch checked="checked"></switch>`)

    expect(vmA.$el.className).to.match(/weex\-switch\-checked/)
    expect(vmB.$el.className).to.match(/weex\-switch\-checked/)
    expect(vmC.$el.className).to.match(/weex\-switch\-checked/)
  })

  it('toggle <switch>', () => {
    const vm = compile(`<switch ref="switch"></switch>`).$refs.switch
    expect(vm.isChecked).to.not.be.true
    vm.toggle()
    expect(vm.isChecked).to.be.true
    vm.toggle()
    expect(vm.isChecked).to.not.be.true
  })

  it('toggle & disabled <switch>', () => {
    const vm = compile(`<switch ref="switch"></switch>`).$refs.switch
    vm.isDisabled = true

    expect(vm.isChecked).to.not.be.true
    vm.toggle()
    expect(vm.isChecked).to.not.be.true
  })

  it('unchecked <switch>', () => {
    const vmA = compile(`<switch></switch>`)
    const vmB = compile(`<switch checked="false"></switch>`)

    expect(vmA.$el.className).to.match(/weex-switch/)
    expect(vmB.$el.className).to.match(/weex-switch/)
  })

  it('disabled & checked <switch>', () => {
    const vmA = compile(`<switch disabled checked></switch>`)
    const vmB = compile(`<switch disabled="disabled" checked="checked"></switch>`)

    expect(vmA.$el.className).to.match(/weex\-switch\-checked/)
    expect(vmA.$el.className).to.match(/weex\-switch\-disabled/)
    expect(vmB.$el.className).to.match(/weex\-switch\-checked/)
    expect(vmB.$el.className).to.match(/weex\-switch\-disabled/)
  })
})
