// import semver from 'semver'
import weex from './env'
import { setVue } from './env'
import * as components from './components'
import { base, style } from './mixins'
// import styleMixin from './mixins/style'

/**
 * init weex.
 * @param  {Vue$2} Vue: Vue Constructor.
 * @param  {object} options: extend weex plugins.
 *         - components.
 *         - modules.
 */
function init (Vue/*, options = {}*/) {
  setVue(Vue)

  Vue.prototype.$getConfig = () => {
    console.warn('[Vue Render] "this.$getConfig" is deprecated, please use "weex.config" instead.')
    return weex.config
  }

  const htmlRegex = /^html:/i
  Vue.config.isReservedTag = tag => htmlRegex.test(tag)
  Vue.config.parsePlatformTagName = tag => tag.replace(htmlRegex, '')

  // register sdk components.
  for (const name in components) {
    weex.registerComponent(name, components[name])
  }

  /* istanbul ignore next */
  // if (process.env.NODE_ENV === 'development') {
  //   if (semver.lt(Vue.version, '2.1.5')) {
  //     console.warn(`[Vue Render] The version of Vue should be ` +
  //       `greater than 2.1.5, current is ${Vue.version}.`)
  //   }
  //   console.info(`[Vue Render] Registered components: `
  //     + `[${Object.keys(components).join(', ')}].`)
  Vue.mixin(base)
  Vue.mixin(style)
  // }
}

// auto init in dist mode.
if (typeof window !== 'undefined' && window.Vue) {
  init(window.Vue)
}

export default weex
