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

const _css = `
.weex-switch {
  border: 0.013333rem solid #dfdfdf;
  cursor: pointer;
  display: inline-block;
  position: relative;
  vertical-align: middle;
  -webkit-user-select: none;
  -moz-user-select: none;
  -ms-user-select: none;
  user-select: none;
  box-sizing: content-box;
  background-clip: content-box;
  color: #64bd63;
  width: 1.333333rem;
  height: 0.8rem;
  background-color: white;
  border-color: #dfdfdf;
  box-shadow: #dfdfdf 0 0 0 0 inset;
  border-radius: 0.8rem;
  -webkit-transition: border 0.4s, box-shadow 0.4s, background-color 1.2s;
  -moz-transition: border 0.4s, box-shadow 0.4s, background-color 1.2s;
  transition: border 0.4s, box-shadow 0.4s, background-color 1.2s;
}

.weex-switch-checked {
  background-color: #64bd63;
  border-color: #64bd63;
  box-shadow: #64bd63 0 0 0 0.533333rem inset;
}

.weex-switch-checked.weex-switch-disabled {
  background-color: #A0CCA0;
  box-shadow: #A0CCA0 0 0 0 0.533333rem inset;
}

.weex-switch-disabled {
  background-color: #EEEEEE;
}

.weex-switch-inner {
  width: 0.8rem;
  height: 0.8rem;
  background: #fff;
  border-radius: 100%;
  box-shadow: 0 0.013333rem 0.04rem rgba(0, 0, 0, 0.4);
  position: absolute;
  top: 0;
  left: 0;
  -webkit-transition: background-color 0.4s, left 0.2s;
  -moz-transition: background-color 0.4s, left 0.2s;
  transition: background-color 0.4s, left 0.2s;
}

.weex-switch-checked > .weex-switch-inner {
  left: 0.533333rem;
}
`

function getSwitch (weex) {
  const { extractComponentStyle } = weex

  return {
    name: 'weex-switch',
    props: {
      checked: {
        type: [Boolean, String],
        default: false
      },
      disabled: {
        type: [Boolean, String],
        default: false
      }
    },
    data () {
      return {
        isChecked: (this.checked !== 'false' && this.checked !== false),
        isDisabled: (this.disabled !== 'false' && this.disabled !== false)
      }
    },
    computed: {
      wrapperClass () {
        const classArray = ['weex-switch']
        this.isChecked && classArray.push('weex-switch-checked')
        this.isDisabled && classArray.push('weex-switch-disabled')
        return classArray.join(' ')
      }
    },
    methods: {
      toggle () {
        // TODO: handle the events
        if (!this.isDisabled) {
          this.isChecked = !this.isChecked
          this.$emit('change', { value: this.isChecked })
        }
      }
    },

    mounted () {
      const el = this.$el
      if (el && el.nodeType === 1) {
        if (!this._removeClickHandler) {
          const handler = evt => {
            this.toggle()
          }
          this._removeClickHandler = el.removeEventListener.bind(el, 'click', handler)
          el.addEventListener('click', handler)
        }
      }
    },

    beforeDestroy () {
      const rm = this._removeClickHandler
      if (rm) {
        rm()
        delete this._removeClickHandler
      }
    },

    render (createElement) {
      /* istanbul ignore next */
      // if (process.env.NODE_ENV === 'development') {
      //   validateStyles('switch', this.$vnode.data && this.$vnode.data.staticStyle)
      // }
      return createElement('span', {
        attrs: { 'weex-type': 'switch' },
        staticClass: this.wrapperClass,
        staticStyle: extractComponentStyle(this)
      }, [createElement('small', { staticClass: 'weex-switch-inner' })])
    },
    _css
  }
}

export default {
  init (weex) {
    weex.registerComponent('switch', getSwitch(weex))
  }
}
