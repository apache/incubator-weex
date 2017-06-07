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
// import { validateStyles } from '../validator'
import { extractComponentStyle, trimTextVNodes, createEventMap } from '../core'

const _css = `
body > .weex-div {
  min-height: 100%;
}
`

export default {
  name: 'weex-div',
  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('div', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    this._renderHook()
    return createElement('html:div', {
      attrs: { 'weex-type': 'div' },
      on: createEventMap(this),
      staticClass: 'weex-div weex-ct',
      staticStyle: extractComponentStyle(this)
    }, trimTextVNodes(this.$slots.default))
  },
  _css
}
