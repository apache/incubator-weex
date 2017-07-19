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
import * as type from '../../../../render/vue/utils/type'
describe('utils', function () {
  describe('type', function () {
    it('isPlainObject', () => {
      const {
        isPlainObject
      } = type
      expect(isPlainObject).to.be.a('function')
      expect(isPlainObject({})).to.be.equal(true)
      expect(isPlainObject(1)).to.be.equal(false)
      expect(isPlainObject([1, 2, 3])).to.be.equal(false)
      expect(isPlainObject('1')).to.be.equal(false)
      expect(isPlainObject(new Date())).to.be.equal(false)
      expect(isPlainObject(() => {})).to.be.equal(false)
      expect(isPlainObject(new RegExp('/^.?/'))).to.be.equal(false)
    })
    it('isArray', () => {
      const {
        isArray
      } = type
      expect(isArray).to.be.a('function')
      expect(isArray([1, 2, 3])).to.be.equal(true)
      expect(isArray(1)).to.be.equal(false)
      expect(isArray('1')).to.be.equal(false)
      expect(isArray({})).to.be.equal(false)
      expect(isArray(new Date())).to.be.equal(false)
      expect(isArray(() => {})).to.be.equal(false)
      expect(isArray(new RegExp('/^.?/'))).to.be.equal(false)
    })
  })
})
