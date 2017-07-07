/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License") you may not use this file except in compliance
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
  collectStatPerf,
  tagFirstScreen,
  tagImg
} from '../../../../render/vue/utils/perf'
describe('utils', function () {
  describe('pref', function () {
    before(function () {
      this.clock = sinon.useFakeTimers()
    })
    after(function () {
      this.clock.restore()
    })
    it('collectStatPerf', function () {
      expect(collectStatPerf).to.be.a('function')
      collectStatPerf('totalTime', (new Date()).getTime())
    })
    it('tagFirstScreen', function () {
      expect(tagFirstScreen).to.be.a('function')
      tagFirstScreen()
      window._first_screen_detected = false
      tagFirstScreen()
    })
    it('tagImg', function () {
      tagImg()
      this.clock.tick(500)
      expect(tagImg).to.be.a('function')
    })
  })
})
