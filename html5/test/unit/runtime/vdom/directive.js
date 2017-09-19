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
import { filterDirective } from '../../../../runtime/vdom/directive'

describe('filterDirective', () => {
  it('other type', () => {
    expect(filterDirective(0)).to.be.equal(0)
    expect(filterDirective(53)).to.be.equal(53)
    expect(filterDirective(null)).to.be.equal(null)
    const reg = /\w+/i
    expect(filterDirective(reg)).to.be.equal(reg)
  })

  it('normal string', () => {
    expect(filterDirective('')).to.be.equal('')
    expect(filterDirective('-1')).to.be.equal('-1')
    expect(filterDirective('abc')).to.be.equal('abc')
    expect(filterDirective(' a bc d')).to.be.equal(' a bc d')
    expect(filterDirective(' a {{ bc }}')).to.be.equal(' a {{ bc }}')
  })

  it('binding', () => {
    expect(filterDirective('[[abc]]')).to.deep.equal({ '@binding': 'abc' })
    expect(filterDirective('[[ xyz ]]')).to.deep.equal({ '@binding': 'xyz' })
    expect(filterDirective('[[ x y z ]]')).to.deep.equal({ '@binding': 'x y z' })
  })

  it('binding and normal string', () => {
    expect(filterDirective('xyz[[abc]]ttt')).to.deep.equal(['xyz', { '@binding': 'abc' }, 'ttt'])
    expect(filterDirective(' x [[ w ]] t ')).to.deep.equal([' x ', { '@binding': 'w' }, ' t '])
    expect(filterDirective('[[ wn]][xx{uur}]')).to.deep.equal([{ '@binding': 'wn' }, '[xx{uur}]'])
  })

  it('multi-binding', () => {
    expect(filterDirective('[[abc]][[wpc]]')).to.deep.equal([{ '@binding': 'abc' }, { '@binding': 'wpc' }])
    expect(filterDirective('abcd[[ xyz ]]ef[[w]]gh')).to.deep.equal([
      'abcd',
      { '@binding': 'xyz' },
      'ef',
      { '@binding': 'w' },
      'gh'
    ])
    expect(filterDirective(' a [[ b ]] [[c]] d [[e]][[f]]g')).to.deep.equal([
      ' a ',
      { '@binding': 'b' },
      ' ',
      { '@binding': 'c' },
      ' d ',
      { '@binding': 'e' },
      { '@binding': 'f' },
      'g'
    ])
  })

  it('parse object', () => {
    expect(filterDirective({ key: '[[x]]' })).to.deep.equal({ key: { '@binding': 'x' }})
    expect(filterDirective({ a: '[[A]]', b: '[[B]]' })).to.deep.equal({
      a: { '@binding': 'A' },
      b: { '@binding': 'B' }
    })
    expect(filterDirective({
      a: '[[A]]',
      x: 'X',
      y: {
        b: ' - [[B]] - ',
        z: {
          c: ['[[C]] + [[C]]', 'cc'],
          w: 24
        }
      }
    })).to.deep.equal({
      a: { '@binding': 'A' },
      x: 'X',
      y: {
        b: [' - ', { '@binding': 'B' }, ' - '],
        z: {
          c: [[{ '@binding': 'C' }, ' + ', { '@binding': 'C' }], 'cc'],
          w: 24
        }
      }
    })
  })
})
