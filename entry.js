import weex from './packages/weex-vue-render/dist/index.core'
import plugins from './weex-vue-plugins'

// install plugins.
plugins.forEach(function (plugin) {
  weex.install(plugin)
})

weex.init()

export default weex
