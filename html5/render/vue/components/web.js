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
import { extractComponentStyle, createEventMap } from '../core'
import { createEvent } from '../utils'
// import { validateStyles } from '../validator'

const _css = `
.weex-web {
  position: relative;
  width: 100%;
  height: 100%;
  border: none;
  box-sizing: border-box;
}
`

export default {
  props: {
    src: String
  },
  methods: {
    // TODO: check cross-origin
    goBack () {
      if (this.$el) {
        this.$el.contentWindow.history.back()
      }
    },
    goForward () {
      if (this.$el) {
        this.$el.contentWindow.history.forward()
      }
    },
    reload () {
      if (this.$el) {
        this.$el.contentWindow.history.reload()
      }
    }
  },

  mounted () {
    if (this.$el) {
      this.$emit('pagestart', createEvent(this.$el, 'pagestart', { url: this.src }))
      this.$el.addEventListener('load', event => {
        this.$emit('pagefinish', createEvent(this.$el, 'pagefinish', { url: this.src }))
      })
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('web', this.$vnode.data && this.$vnode.data.staticStyle)
    // }

    this._renderHook()
    return createElement('iframe', {
      attrs: {
        'weex-type': 'web',
        src: this.src
      },
      on: createEventMap(this, ['error']),
      staticClass: 'weex-web weex-el',
      staticStyle: extractComponentStyle(this)
    })
  },
  _css
}
