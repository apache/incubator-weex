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
// import { validateStyles } from '../../validator'
// import indicator from './indicator'
import slideMixin from './slideMixin'

export default {
  mixins: [slideMixin],
  props: {
    index: {
      type: [String, Number],
      default: 0
    },
    'auto-play': {
      type: [String, Boolean],
      default: false
    },
    interval: {
      type: [String, Number],
      default: 3000
    },
    infinite: {
      type: [String, Boolean],
      default: true
    }
  },

  watch: {
    index () {
      this.currentIndex = this._normalizeIndex(this.index)
    }
  },

  data () {
    return {
      frameCount: 0,
      currentIndex: this.index
    }
  },

  beforeCreate () {
    this.weexType = 'slider'
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('slider', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    return this._renderSlides(createElement)
  }
}
