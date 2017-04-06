import css from '../styles/reset.css'
import '../styles/base.css'

// import 'lazyimg'
import '../../browser/render/gesture'

import '../../../shared/arrayFrom'
import '../../../shared/objectAssign'
import '../../../shared/objectSetPrototypeOf'

import 'core-js/modules/es6.object.to-string'
import 'core-js/modules/es6.string.iterator'
import 'core-js/modules/web.dom.iterable'
import 'core-js/modules/es6.promise'

import modules from '../modules'
import weex from './weex'

// register built-in modules.
weex.install(modules)

export function setVue (vue) {
  if (!vue) {
    throw new Error('[Vue Render] Vue not found. Please make sure vue 2.x runtime is imported.')
  }
  weex.__vue__ = vue
  console.log(`[Vue Render] install Vue ${vue.version}.`)
}

window.weex = weex
window.global = window

export default weex
