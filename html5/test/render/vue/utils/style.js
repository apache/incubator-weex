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
import {
  supportHairlines,
  supportSticky,
  autoPrefix,
  normalizeStyle,
  normalizeString,
  normalizeNumber,
  normalizeUnitsNum,
  trimComment
} from '../../../../render/vue/utils/style'
import { init } from '../../../../render/vue/env/viewport'

describe('style', function () {
  // const rect = document.documentElement.getBoundingClientRect()
  // const info = {}
  const { scale, dpr } = init()
  it('should support using 0.5px to paint 1px width border', () => {
    expect(supportHairlines()).to.be.false
    window.devicePixelRatio = 2
    // phantomjs didn't support using 0.5px to paint 1px width border
    expect(supportHairlines()).to.be.false
  })
  it('should support stick style', () => {
    // phantomjs support stick true
    expect(supportSticky()).to.be.true
  })

  it('should normalize units numbers', () => {
    expect(normalizeUnitsNum('100px')).to.equal(100 * scale + 'px')
    expect(normalizeUnitsNum('100')).to.equal(100 * scale + 'px')
    expect(normalizeUnitsNum('100wx')).to.equal(100 * scale * dpr + 'px')
    expect(normalizeUnitsNum('20wm')).to.equal('')
  })

  it('should normalize number style vals', () => {
    expect(normalizeNumber('width', 10)).to.equal(10 * scale + 'px')
    expect(normalizeNumber('width', 1.2)).to.equal(1.2 * scale + 'px')
  })

  it('should normalize string style vals', () => {
    expect(normalizeString('width', '100%')).to.equal('100%')
    expect(normalizeString('transform', 'translate3d(10px, 10px, 10px)'))
      .to.equal(`translate3d(${10 * scale}px, ${10 * scale}px, ${10 * scale}px)`)
    expect(normalizeString('border-bottom', '4px solid #112233'))
      .to.equal(`${4 * scale}px solid #112233`)
    expect(normalizeString('border-left', '4wx dotted red'))
      .to.equal(`${4 * scale * dpr}px dotted red`)
  })

  it('should normalize style object', () => {
    const style = {
      width: '200px',
      flexDirection: 'row',
      flex: 1,
      fontSize: 12,
      transform: 'translate3d(100px, 100px, 0)'
    }
    const res = normalizeStyle(style)
    console.log(res)
    expect(normalizeString('transform', style.transform)).to.equal(res.transform)
    expect(normalizeString('width', style.width)).to.equal(res.width)
    expect(normalizeString('flexDirection', style.flexDirection)).to.equal(res.flexDirection)
    expect(normalizeNumber('fontSize', style.fontSize)).to.equal(res.fontSize)
    // flex is in noUnitsNumberKeys array
    expect(style.flex).to.equal(res.flex)
  })

  it('should trim comment in style.', () => {
    const cssText = '.ani-point0[data-v-4c05cc1a] {\n  left: 88px;\n  top: 88px;\n}\n.ani-point1[data-v-4c05cc1a] {\n  /* left: 43px;\n  top: 71px;*/\n}'
    const trimmed = '.ani-point0[data-v-4c05cc1a] {\n  left: 88px;\n  top: 88px;\n}\n.ani-point1[data-v-4c05cc1a] {\n  \n}'
    expect(trimComment(cssText)).to.equal(trimmed)
  })
})
describe('autoPrefix', () => {
  const style = {
    width: '200px',
    flexDirection: 'row',
    flex: 1,
    transform: 'translate3d(100px, 100px, 0)'
  }

  it('should add prefix for styles.', () => {
    const res = autoPrefix(style)
    const {
      WebkitBoxDirection,
      WebkitBoxOrient,
      WebkitFlexDirection,
      flexDirection,
      WebkitTransform,
      transform,
      width
    } = res
    expect(WebkitBoxDirection).to.equal('normal')
    expect(WebkitBoxOrient).to.equal('horizontal')
    expect(WebkitFlexDirection).to.equal('row')
    expect(flexDirection).to.equal('row')
    expect(WebkitTransform).to.equal('translate3d(100px, 100px, 0)')
    expect(transform).to.equal('translate3d(100px, 100px, 0)')
    expect(width).to.equal('200px')
  })
})
