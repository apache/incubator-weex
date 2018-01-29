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

import { expect } from 'chai'
import { execute } from './helper'
import { compile } from 'weex-template-compiler'

function toFunction (content) {
  return `function () { ${content} }`
}

function compileAndExecute (template, additional) {
  const { render, staticRenderFns } = compile(template)
  return execute('Vue', `
    new Vue({
      el: '#root',
      render: ${toFunction(render)},
      staticRenderFns: [${staticRenderFns.map(toFunction).join(',')}],
      ${additional}
    })
  `)
}

describe('Vue Examples', () => {
  it('execute Vue code', (done) => {
    execute('Vue', `
      new Vue({
        el: 'body',
        render (h) {
          return h('div', {}, [h('text', {}, 'Hello World')])
        }
      })
    `).then($root => {
      expect($root.type).to.be.equal('div')
      expect($root.children[0].type).to.be.equal('text')
      expect($root.children[0].attr.value).to.be.equal('Hello World')
    }).then(done)
  })

  it('compile and execute', (done) => {
    compileAndExecute(`
      <div>
        <text>Hello World</text>
      </div>
    `).then($root => {
      expect($root.type).to.be.equal('div')
      expect($root.children[0].type).to.be.equal('text')
      expect($root.children[0].attr.value).to.be.equal('Hello World')
    }).then(done)
  })
})
