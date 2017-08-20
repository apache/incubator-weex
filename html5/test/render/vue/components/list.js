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
import list from '../../../../render/vue/components/scrollable/list'
import cell from '../../../../render/vue/components/scrollable/list/cell'

init('<list> component', (Vue, helper) => {
  const { utils, compile } = helper

  before(() => {
    helper.register('list', list)
    helper.register('cell', cell)
  })

  it('create simple list component', () => {
    const vm = compile(`<list><cell></cell></list>`)
    expect(vm.$el.tagName.toLowerCase()).to.be.equal('main')
    expect(utils.toArray(vm.$el.classList)).to.include.members(['weex-list', 'weex-list-wrapper', 'weex-ct'])
  })
})
