import weex from './weex'
import * as core from '../core'

window.global = window
window.weex = weex

; ['getComponentStyle',
  'extractComponentStyle',
  'trimTextVNodes'].forEach(function (method) {
    weex[method] = core[method].bind(weex)
  })
