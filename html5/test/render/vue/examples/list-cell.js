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
import text from '../../../../render/vue/components/text'

init('list & cell example', (Vue, helper) => {
  const { utils, createVm } = helper

  before(() => {
    helper.register('list', list)
    helper.register('cell', cell)
    helper.register('text', text)
  })

  it('create simple list component', () => {
    const vm = createVm({
      template:
      `<list>
        <cell v-for="char in lists">
          <text>-- {{char}} --</text>
        </cell>
      </list>`,
      data: function () {
        return {
          lists: ['A', 'B', 'C']
        }
      }
    })

    expect(vm.$el.tagName).to.be.equal('MAIN')
    expect(utils.toArray(vm.$el.classList)).to.include.members(['weex-list', 'weex-list-wrapper'])
  })
})
