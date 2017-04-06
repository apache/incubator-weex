import './env'

/* istanbul ignore next */
import Vue from 'vue/dist/vue.esm.js'
// import { base, scrollable, style, inputCommon } from '../../../render/vue/mixins'
import { base, style } from '../../../../render/vue/mixins'
import weex from '../../../../render/vue/env/weex'
import { setVue } from '../../../../render/vue/env'

import * as utils from './utils'

/**
 * Describe tests for current versions of Vue.
 */
export function init (title, fn) {
  return describe(title, () => {
    let components = {}

    before(function () {
      const htmlRegex = /^html:/i
      Vue.config.isReservedTag = tag => htmlRegex.test(tag)
      Vue.config.parsePlatformTagName = tag => tag.replace(htmlRegex, '')

      Vue.mixin(base)
      Vue.mixin(style)

      setVue(Vue)

      window.global = window
      global.weex = weex

      window._no_remove_style_sheets = true
    })

    const helper = {

      utils,
      /**
       * register a component.
       * @param  {string} name,
       * @param  {object} component.
       */
      register (name, component) {
        global.weex.registerComponent(name, component)
        // components[name] = component
      },

      /**
       * create a vm instance of Vue.
       * @param  {Object} options.
       * @return {Vue} vue instance.
       */
      createVm (options = {}) {
        // options.components = components
        return new Vue(options).$mount()
      },

      /**
       * [compile description]
       * @param  {[type]} template [description]
       * @return {[type]}          [description]
       */
      compile (template) {
        return helper.createVm({ template })
      }
    }

    /**
     * describe a vue-render test for certain vue verson.
     */
    describe(`Vue ${Vue.version}`, () => {
      fn(Vue, helper)
    })
  })
}

// import vue200 from './vender/vue-2.0.0.js'
// import vue210 from './vender/vue-2.1.0.js'
// const Vues = [
//   vue200,
//   vue210
// ]
/**
 * Describe tests for multiple versions of Vue.
 */
// export function multiDescribe (title, fn) {
//   return describe(title, () => {
//     Vues.forEach(Vue => {
//       let components = {}
//       Vue.config.isReservedTag = function () { return false }
//       describe(`Vue ${Vue.version}`, () => fn(Vue, {
//         register (name, component) {
//           components[name] = component
//         },
//         reset () {
//           components = {}
//         },
//         createVm (options = {}) {
//           options.components = components
//           return new Vue(options).$mount()
//         },
//         compile (template) {
//           return this.createVm({ template })
//         }
//       }))
//     })
//   })
// }
