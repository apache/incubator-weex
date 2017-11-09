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
import dom from '../../../../render/vue/modules/dom'
describe('dom module', () => {
  const callback = sinon.spy()
  weex.install(dom)
  const domModule = weex.requireModule('dom')
  it('should scrollToElement be worked', (done) => {
    const {
      scrollToElement
    } = domModule
    const node = document.createElement('div')
    const vnode = {
      $el: node
    }
    const options = {
      offset: 100
    }
    node.style.height = '100px'
    node.style.width = '100px'
    node.style.backgroundColor = 'green'
    node.style['margin-top'] = '800px'
    expect(scrollToElement).to.be.a('function')
    scrollToElement([vnode], options)
    setTimeout(() => {
      expect(document.body.scrollTop).to.be.equal(0)
      scrollToElement(vnode, {
        animated: false
      })
      expect(document.body.scrollTop).to.be.equal(0)
      done()
    }, 500)
  })
  it('should getComponentRect be worked', () => {
    const {
      getComponentRect
    } = domModule
    const node = document.createElement('div')
    const vnode = {
      $el: node,
      $refs: {}
    }
    const rectKeys = ['width', 'height', 'top', 'bottom', 'left', 'right']
    let message
    const scale = window.weex.config.env.scale
    const recalc = (rect) => {
      const res = {}
      rectKeys.forEach(function (key) {
        res[key] = rect[key] / scale
      })
      return res
    }
    node.style.height = '100px'
    node.style.width = '100px'
    document.body.appendChild(node)
    expect(getComponentRect).to.be.a('function')
    // while node is an element
    message = getComponentRect([vnode], callback)
    expect(message.result).to.be.true
    expect(message.size.width).to.be.equal(recalc({
      width: 100
    }).width)
    expect(message.size.height).to.be.equal(recalc({
      height: 100
    }).height)
    expect(message.contentSize.width).to.be.equal(recalc({
      width: 100
    }).width)
    expect(message.contentSize.height).to.be.equal(recalc({
      height: 100
    }).height)
    expect(callback.callCount).to.be.equal(1)

    // while node is a viewport
    message = getComponentRect('viewport', callback)
    expect(message.result).to.be.true
    expect(message.size.width).to.be.equal(recalc({
      width: document.documentElement.clientWidth
    }).width)
    expect(message.size.right).to.be.equal(recalc({
      right: document.documentElement.clientWidth
    }).right)
    expect(message.size.height).to.be.equal(recalc({
      height: document.documentElement.clientHeight
    }).height)
    expect(message.size.bottom).to.be.equal(recalc({
      bottom: document.documentElement.clientHeight
    }).bottom)
    expect(message.contentSize.width).to.be.equal(recalc({
      bottom: document.documentElement.offsetWidth
    }).bottom)
    expect(message.contentSize.height).to.be.equal(recalc({
      bottom: document.documentElement.offsetHeight
    }).bottom)
    expect(callback.callCount).to.be.equal(2)
    document.body.removeChild(node)
  })
  it('should addRule be worked', () => {
    const {
      addRule
    } = domModule
    const key = 'font-face'
    const styles = {
      'font-family': 'iconfont'
    }
    expect(addRule).to.be.a('function')
    addRule(key, styles)
    const styleElement = document.getElementById('dom-added-rules')
    expect(styleElement.innerText).to.be.equal('@font-face{font-family:iconfont;}')
  })
})
