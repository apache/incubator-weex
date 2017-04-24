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
import './env'

/* istanbul ignore next */
import Vue from 'vue/dist/vue.runtime.esm.js'
// import { base, scrollable, style, inputCommon } from '../../../render/vue/mixins'
import { base, style } from '../../../../render/vue/mixins'
import weex from '../../../../render/vue/env/weex'
import { setVue } from '../../../../render/vue/env'
import helper from './main'
import { doneMixin } from './mixin'

/**
 * Describe tests for current versions of Vue.
 */
export function init (title, fn) {
  return describe(title, () => {
    // let components = {}

    before(function () {
      const htmlRegex = /^html:/i
      Vue.config.isReservedTag = tag => htmlRegex.test(tag)
      Vue.config.parsePlatformTagName = tag => tag.replace(htmlRegex, '')

      Vue.mixin(base)
      Vue.mixin(style)

      // for test only mixins.
      Vue.mixin(doneMixin)

      window.global = window
      global.weex = weex
      setVue(Vue)

      window._no_remove_style_sheets = true
    })

    /**
     * describe a vue-render test for certain vue verson.
     */
    describe(`Vue ${Vue.version}`, () => {
      fn(Vue, helper)
    })
  })
}
