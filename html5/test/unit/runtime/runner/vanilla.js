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

describe('Vanilla Examples', () => {
  it('execute vanilla code', (done) => {
    execute('Vanilla', `
      const body = document.createElement('div')
      document.documentElement.appendChild(body)
    `).then($root => {
      expect($root.type).to.be.equal('div')
    }).then(done)
  })

  it('instert child nodes', (done) => {
    execute('Vanilla', `
      const body = document.createElement('div')
      body.appendChild(document.createElement('text'))
      body.appendChild(document.createElement('image'))
      document.documentElement.appendChild(body)
    `).then($root => {
      expect($root.type).to.be.equal('div')
      expect($root.children[0].type).to.be.equal('text')
      expect($root.children[1].type).to.be.equal('image')
    }).then(done)
  })

  it('add text nodes', (done) => {
    execute('Vanilla', `
      const body = document.createElement('div')
      const text = document.createElement('text')
      text.setAttr('value', 'whatever')
      body.appendChild(text)
      document.documentElement.appendChild(body)
    `).then($root => {
      expect($root.type).to.be.equal('div')
      expect($root.children[0].type).to.be.equal('text')
      expect($root.children[0].attr.value).to.be.equal('whatever')
    }).then(done)
  })
})
