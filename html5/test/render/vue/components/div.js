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
import div from '../../../../render/vue/components/div'

init('<div> component', (Vue, helper) => {
  const { compile, utils } = helper

  before(() => {
    helper.register('div', div)
  })

  it('simple <div> component', () => {
    const vm = compile(`<div>abc</div>`)
    const el = vm.$el
    expect(el.tagName.toLowerCase()).to.equal('div')
    const classListArr = utils.toArray(el.classList)
    expect('weex-div').to.be.oneOf(classListArr)
    expect('weex-ct').to.be.oneOf(classListArr)
    expect(el.getAttribute('weex-type')).to.be.equal('div')
    expect(el.innerHTML).to.be.equal('')
  })

  it('<div> with children <div>', () => {
    const vm = compile(`<div><div></div></div>`)
    const el = vm.$el
    expect(el.children.length).to.be.equal(1)
    expect(el.children[0].tagName).to.match(/^(?:html:)?div$/i)
    expect(el.children[0].getAttribute('weex-type')).to.be.equal('div')
  })
})
