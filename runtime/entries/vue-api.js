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

import { setGlobalAPI } from '../shared/utils'
import VueFactory from 'weex-vue-framework/factory'

function createVueModuleInstance () {
  const exports = {}
  VueFactory(exports, weex.document)
  const Vue = exports.Vue

  let instanceApp

  // patch reserved tag detection to account for dynamically registered
  // components
  const weexRegex = /^weex:/i
  const isReservedTag = Vue.config.isReservedTag || (() => false)
  const isRuntimeComponent = Vue.config.isRuntimeComponent || (() => false)
  Vue.config.isReservedTag = name => {
    return (!isRuntimeComponent(name) && weex.supports(`@component/${name}`)) ||
      isReservedTag(name) ||
      weexRegex.test(name)
  }
  Vue.config.parsePlatformTagName = name => name.replace(weexRegex, '')

  // expose weex-specific info
  Vue.prototype.$instanceId = weex['[[CurrentInstanceId]]']
  Vue.prototype.$document = weex.document

  // expose weex native module getter on subVue prototype so that
  // vdom runtime modules can access native modules via vnode.context
  Vue.prototype.$requireWeexModule = weex.requireModule

  // Hack `Vue` behavior to handle instance information and data
  // before root component created.
  Vue.mixin({
    beforeCreate () {
      const options = this.$options
      // root component (vm)
      if (options.el) {
        // set external data of instance
        const dataOption = options.data
        const internalData = (typeof dataOption === 'function' ? dataOption() : dataOption) || {}
        options.data = Object.assign(internalData, __weex_data__)
        instanceApp = this
      }
    },
    mounted () {
      const options = this.$options
      // root component (vm)
      if (options.el && weex.document && instanceApp === this) {
        try {
          // Send "createFinish" signal to native.
          weex.document.taskCenter.send('dom', { action: 'createFinish' }, [])
        } catch (e) {}
      }
    }
  })

  setGlobalAPI('Vue', Vue)

  __instance_lifecycle__.onDestory = () => {
    if (instanceApp instanceof Vue) {
      instanceAapp.$destroy()
    }
  }

  __instance_lifecycle__.onRefresh = ({ instance, data }) => {
    if (!instance || !(instanceApp instanceof Vue)) {
      return new Error(`refreshInstance: instance ${instanceId} not found!`)
    }
    for (const key in data) {
      Vue.set(instanceApp, key, data[key])
    }
  }
}

createVueModuleInstance()
