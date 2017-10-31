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
import weex from './env'
import { setVue } from './env'
import { base, style, sticky } from './mixins'

import initEventMgr from './env/event-manager'

/**
 * init weex.
 * @param  {Vue$2} Vue: Vue Constructor.
 * @param  {object} options: extend weex plugins.
 *         - components.
 *         - modules.
 */
let _inited = false
function init (Vue/*, options = {}*/) {
  if (_inited) { return }
  _inited = true

  setVue(Vue)

  Vue.prototype.$getConfig = () => {
    console.warn('[Vue Render] "this.$getConfig" is deprecated, please use "weex.config" instead.')
    return weex.config
  }

  const htmlRegex = /^html:/i
  Vue.config.isReservedTag = tag => htmlRegex.test(tag)
  Vue.config.parsePlatformTagName = tag => tag.replace(htmlRegex, '')

  function isWeexTag (tag) {
    return typeof weex._components[tag] !== 'undefined'
  }
  const oldGetTagNamespace = Vue.config.getTagNamespace
  Vue.config.getTagNamespace = function (tag) {
    if (isWeexTag(tag)) {
      return
    }
    return oldGetTagNamespace(tag)
  }

  Vue.mixin(base)
  Vue.mixin(style)
  Vue.mixin(sticky)

  initEventMgr()
}

// auto init in dist mode.
if (typeof window !== 'undefined' && window.Vue) {
  init(window.Vue)
}

weex.init = init

export default weex
