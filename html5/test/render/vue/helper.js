import '../../../render/vue/styles/reset.css'
import '../../../render/vue/styles/components.css'

import '../../../render/browser/render/gesture'

/* istanbul ignore next */
import '../../../shared/arrayFrom'
/* istanbul ignore next */
import '../../../shared/objectAssign'
/* istanbul ignore next */
import '../../../shared/objectSetPrototypeOf'

/* istanbul ignore next */
import 'core-js/modules/es6.object.to-string'
/* istanbul ignore next */
import 'core-js/modules/es6.string.iterator'
/* istanbul ignore next */
import 'core-js/modules/web.dom.iterable'
/* istanbul ignore next */
import 'core-js/modules/es6.promise'

/* istanbul ignore next */
import Vue from 'vue/dist/vue.esm.js'
// import { base, scrollable, style, inputCommon } from '../../../render/vue/mixins'
import { base, style } from '../../../render/vue/mixins'
import weex from '../../../render/vue/env/weex'

/**
 * functional tools exported by helper.utils.
 */
const utils = {
  toArray (list) {
    if (!list) return []
    return Array.prototype.slice.call(list)
  }
}

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

      window.global = window
      global.weex = weex
    })

    const helper = {

      utils,
      /**
       * register a component.
       * @param  {string} name,
       * @param  {object} component.
       */
      register (name, component) {
        components[name] = component
      },

      /**
       * reset registered components with empty object.
       */
      reset () {
        components = {}
      },

      /**
       * create a vm instance of Vue.
       * @param  {Object} options.
       * @return {Vue} vue instance.
       */
      createVm (options = {}) {
        options.components = components
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
      after(function () {
        helper.reset()
      })

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
