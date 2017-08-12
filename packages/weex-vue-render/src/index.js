import weex from '../../../html5/render/vue'

import components from '../../../html5/render/vue/components'
import modules from '../../../html5/render/vue/modules'

const preInit = weex.init

weex.init = function () {
  preInit.apply(weex, arguments)
  const plugins = components.concat(modules)

  plugins.forEach(function (plugin) {
    weex.install(plugin)
  })
}

if (global.Vue) {
  weex.init(global.Vue)
}

export default weex
