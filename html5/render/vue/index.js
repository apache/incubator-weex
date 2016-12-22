import semver from 'semver'
import * as components from './components'
import { setViewport } from './env'

function install (Vue) {
  setViewport()

  const htmlRegex = /^html:/i
  Vue.config.isReservedTag = tag => htmlRegex.test(tag)
  Vue.config.parsePlatformTagName = tag => tag.replace(htmlRegex, '')

  for (const name in components) {
    Vue.component(name, components[name])
  }

  /* istanbul ignore next */
  if (process.env.NODE_ENV === 'development') {
    if (semver.lt(Vue.version, '2.1.5')) {
      console.warn(`[Vue Renderer] The version of Vue should be ` +
        `greater than 2.1.5, current is ${Vue.version}.`)
    }
    console.log(`[Vue Renderer] Registered components: `
      + `[${Object.keys(components).join(', ')}].`)
  }
}

// auto install in dist mode
if (typeof window !== 'undefined' && window.Vue) {
  install(window.Vue)
}

export default {
  install
}
