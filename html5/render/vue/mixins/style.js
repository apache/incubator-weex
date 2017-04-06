import {
  getHeadStyleMap,
  getComponentStyle,
  extractComponentStyle
} from '../core'

let warned = false
const warnInfo = `[vue-render] after v0.11.3 there's no need to add $processStyle in vue-loader config anymore.`

export default {
  beforeCreate () {
    // get static class style map from document's styleSheets.
    if (!weex.styleMap) {
      weex.styleMap = getHeadStyleMap()
    }
  },

  methods: {
    $processStyle (style) {
      if (!warned) {
        warned = true
        console.warn(warnInfo)
      }
      return style
    },

    _extractComponentStyle () {
      return extractComponentStyle(this)
    },

    /**
     * get style from class, staticClass, style and staticStyle.
     * merge styles priority: high -> low
     *  1. data.style (bound style).
     *  2. data.staticStyle (inline styles).
     *  3. data.class style (bound class names).
     *  4. data.staticClass style (scoped styles or static classes).
     */
    _getComponentStyle (data) {
      return getComponentStyle(this)
    },

    _getParentRect () {
      const parentElm = this.$options._parentElm
      return parentElm && parentElm.getBoundingClientRect()
    }
  }
}
