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
import * as navigator from '../../../../render/vue/modules/navigator'

describe('navigator module', () => {
  const callback = sinon.spy()
  it('should push be worked', () => {
      const { push } = navigator.default
      const config = {
        url: '/'
      }
      expect(push).to.be.a('function')
      push(config, callback)
      expect(callback.callCount).to.be.equal(1)
  })
  it('should push be worked', () => {
      const { pop } = navigator.default
      const config = {
        url: '/'
      }
      expect(pop).to.be.a('function')
      pop(config, callback)
      expect(callback.callCount).to.be.equal(2)
  })
})