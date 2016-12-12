// import Vue from 'vue'
import semver from 'semver'
import * as components from './components'
import { updateMeta } from './prepare'

export function install (Vue) {
  const htmlRegex = /^html:/i
  Vue.config.isReservedTag = tag => htmlRegex.test(tag)
  Vue.config.parsePlatformTagName = tag => tag.replace(htmlRegex, '')

  for (const name in components) {
    Vue.component(name, components[name])
  }

  /* istanbul ignore next */
  if (process.env.NODE_ENV === 'development') {
    console.log(`[Vue Renderer] Registered components: `
      + `[${Object.keys(components).join(', ')}].`)
  }
}

/* eslint-disable no-undef */
if (typeof Vue === 'undefined') {
  console.error('[Vue Renderer] `Vue` is not defined!')
}
else {
  /* istanbul ignore next */
  if (process.env.NODE_ENV === 'development'
    && semver.lt(Vue.version, '2.1.5')) {
    console.warn(`[Vue Renderer] The version of Vue should be ` +
      `greater than 2.1.5, current is ${Vue.version}.`)
  }

  updateMeta()

  Vue.use({ install })
}
