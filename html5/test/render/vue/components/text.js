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
import semver from 'semver'
import { init } from '../helper'
import text from '../../../../render/vue/components/text'

init('<text> component', (Vue, helper) => {
  const { utils, compile } = helper

  before(() => {
    helper.register('text', text)
  })

  it('create simple text component', () => {
    const vm = compile(`<text>abc</text>`)

    expect(vm.$el.tagName.toLowerCase()).to.be.equal('p')
    expect(vm.$el.textContent).to.be.equal('abc')
  })

  it('empty text component', () => {
    const vm = compile(`<text></text>`)

    expect(vm.$el.tagName.toLowerCase()).to.be.equal('p')
    expect(vm.$el.innerHTML).to.be.equal('')
  })

  it('lines style', () => {
    const vm = compile(`<text style="lines:5;">abc</text>`)
    if (semver.gt(Vue.version, '2.0.8')) {
      expect(vm.$el.style.overflow).to.be.equal('hidden')
      expect(vm.$el.style.textOverflow).to.be.equal('ellipsis')
      expect(vm.$el.style.webkitLineClamp).to.be.equal('5')
    }
  })

  it('inline styles', () => {
    const vm = compile(`<text style="color:blue">abc</text>`)

    if (semver.gt(Vue.version, '2.0.8')) {
      expect(vm.$el.style.color).to.be.equal('blue')
    }
  })

  // Not sure about this feature.
  it('class property', () => {
    const vm = compile(`<text class="title"></text>`)
    expect(utils.toArray(vm.$el.classList)).to.include.members(['weex-text', 'weex-el', 'title'])
  })

  it('value property', () => {
    const vm = compile(`<text value="A"></text>`)
    expect(vm.$el.innerHTML).to.be.equal('A')
  })

  it('both have value and content', () => {
    const vm = compile(`<text value="A">B</text>`)
    expect(vm.$el.innerHTML).to.be.equal('B')
  })

  // describe.skip('error usage (on native)', () => {
  //   it('contain other tags', () => {
  //     const vm = compile(`<text><b>abc</b></text>`)
  //     const span = vm.$el.children[0]

  //     expect(span.tagName).to.be.equal('SPAN')
  //     expect(span.innerHTML).to.be.equal('')
  //   })
  // })
})
