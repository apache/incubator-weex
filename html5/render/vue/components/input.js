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

/**
 * @fileOverview Input component.
 * Support v-model only if vue version is large than 2.2.0
 */
import { extractComponentStyle, createEventMap } from '../core'
import { inputCommon } from '../mixins'
import { extend, mapFormEvents } from '../utils'
// import { validateStyles } from '../validator'

const _css = `
.weex-input, .weex-textarea {
  font-size: 0.426667rem;
}
.weex-input:focus, .weex-textarea:focus {
  outline: none;
}
`

export default {
  mixins: [inputCommon],
  props: {
    type: {
      type: String,
      default: 'text',
      validator (value) {
        return [
          'email', 'number', 'password', 'search', 'tel', 'text', 'url'
          // unsupported type:
          // button, checkbox, color, date, datetime, file, hidden, image,
          // month, radio, range, reset, submit, time, week,
        ].indexOf(value) !== -1
      }
    },
    value: String,
    placeholder: String,
    disabled: {
      type: [String, Boolean],
      default: false
    },
    autofocus: {
      type: [String, Boolean],
      default: false
    },
    maxlength: [String, Number],
    returnKeyType: String
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('input', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    const events = extend(createEventMap(this), mapFormEvents(this))
    return createElement('html:input', {
      attrs: {
        'weex-type': 'input',
        type: this.type,
        value: this.value,
        disabled: (this.disabled !== 'false' && this.disabled !== false),
        autofocus: (this.autofocus !== 'false' && this.autofocus !== false),
        placeholder: this.placeholder,
        maxlength: this.maxlength,
        'returnKeyType': this.returnKeyType
      },
      domProps: {
        value: this.value
      },
      on: this.createKeyboardEvent(events),
      staticClass: 'weex-input weex-el',
      staticStyle: extractComponentStyle(this)
    })
  },
  _css
}
