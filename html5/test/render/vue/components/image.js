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
import image from '../../../../render/vue/components/image'

init('<image> component', (Vue, helper) => {
  const { compile, utils } = helper

  before(() => {
    helper.register('image', image)
  })

  it('simple <image> component', () => {
    const vm = compile(`<image>abc</image>`)
    const el = vm.$el
    expect(el.tagName.toLowerCase()).to.be.equal('figure')
    expect(utils.toArray(el.classList)).to.include.members(['weex-image', 'weex-el'])
    expect(el.getAttribute('weex-type')).to.be.equal('image')
  })

  it('<image> with src', () => {
    const src = '//vuejs.org/images/logo.png'
    const vm = compile(`<image src="${src}"></image>`)
    expect(vm.$el.getAttribute('img-src')).to.be.equal(`${src}`)
  })

  it('<image> with placeholder', () => {
    const placeholder = '//vuejs.org/images/logo.png'
    const vm = compile(`<image placeholder="${placeholder}"></image>`)
    expect(vm.$el.getAttribute('img-placeholder')).to.be.equal(`${placeholder}`)
  })

  it('<image> resize="cover"', () => {
    const vm = compile(`<image resize="cover"></image>`)
    expect(vm.$el.style.backgroundSize).to.be.equal('cover')
  })

  it('<image> resize="contain"', () => {
    const vm = compile(`<image resize="contain"></image>`)
    expect(vm.$el.style.backgroundSize).to.be.equal('contain')
  })

  it('<image> resize="stretch"', () => {
    const vm = compile(`<image resize="stretch"></image>`)
    expect(vm.$el.style.backgroundSize).to.contain('100%')
  })

  it('<image> unknown resize', () => {
    const vm = compile(`<image resize="unknown"></image>`)
    expect(vm.$el.style.backgroundSize).to.be.contain('100%')
  })
})
