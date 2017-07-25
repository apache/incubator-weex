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
/*global Event*/
import * as animations from '../../../../render/vue/modules/animation'

describe('animation module', () => {
  it('should transition be worked', (done) => {
    const { transition } = animations.default
    const config = {
      duration: 100,
      timingFunction: 'ease',
      delay: 100,
      styles: {
        position: 'absolute',
        'align-items': 'center',
        flex: 3,
        width: '100%',
        transform: 'translate(10px, 6px, 0)',
        'border-top-width': '1px',
        top: 0,
        'margin-left': '-10px'
      }
    }
    const vnode = { $el: document.createElement('div') }
    const callback = sinon.spy()
    const transitionValue = `all ${config.duration}ms ${config.timingFunction} ${config.delay}ms`
    transition([vnode], config, callback)
    expect(vnode.$el.style.transition).to.be.equal(transitionValue)
    vnode.$el.dispatchEvent(new Event('transitionend'))
    expect(vnode.$el.style.transition).to.be.equal('')
    expect(callback.called).to.be.true
    setTimeout(() => {
        expect(vnode.$el.style.position).to.be.equal(config.styles.position)
        expect(vnode.$el.style['-webkit-box-align']).to.be.equal(config.styles['align-items'])
        expect(vnode.$el.style['-webkit-align-items']).to.be.equal(config.styles['align-items'])
        expect(vnode.$el.style['-webkit-box-flex']).to.be.equal(config.styles.flex+'')
        expect(vnode.$el.style.top).to.be.equal(config.styles.top + 'px')
        expect(vnode.$el.style['margin-left']).to.be.equal('0px')
        done()
    },100)
  })
})
