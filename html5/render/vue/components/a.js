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
import { extractComponentStyle, trimTextVNodes, createEventMap } from '../core'
// import { validateStyles } from '../validator'

const _css = `
.weex-a {
  text-decoration: none;
}
`
let cnt = 0

export default {
  name: 'weex-a',
  props: {
    href: String
  },
  mounted () {
    const $el = this.$el
    const id = $el.id

    /**
     * if there is a child component already triggered a click handler, then
     * this link jumping should be prevented.
     */
    $el.addEventListener('click', (e) => {
      const el = e._triggered && e._triggered.el
      if (el && (el !== $el) && !el.querySelector(`#${id}`)) {
        e.preventDefault()
      }
    })
  },
  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('a', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    this._renderHook()
    const id = cnt++
    return createElement('html:a', {
      attrs: {
        'weex-type': 'a',
        'id': `weex-a-${id}`,
        href: this.href
      },
      on: createEventMap(this),
      staticClass: 'weex-a weex-ct',
      staticStyle: extractComponentStyle(this)
    }, trimTextVNodes(this.$slots.default))
  },
  _css
}
