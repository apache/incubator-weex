import { extend, trimComment, normalizeStyles } from '../utils'
// import { validateStyles } from '../validator'

// let warned = false

// function hyphenateExtend (to, from) {
//   if (!from) { return }
//   for (const k in from) {
//     to[hyphenate(k)] = from[k]
//   }
// }

function getHeadStyleMap () {
  return Array.from(document.styleSheets || [])
    .reduce((pre, styleSheet) => {
      // why not using styleSheet.rules || styleSheet.cssRules to get css rules ?
      // because weex's components defined non-standard style attributes, which is
      // auto ignored when access rule.cssText.
      const strArr = trimComment(styleSheet.ownerNode.textContent.trim()).split(/\.(?!\d+)/)
      const len = strArr.length
      const rules = []
      for (let i = 0; i < len; i++) {
        const str = strArr[i]
        if (!str || str.match(/^\s*$/)) {
          continue
        }
        const match = str.match(/^([^{\s]+)\s*{\s*([^}]+)}\s*$/)
        if (!match) {
          // not the vue static class styles map. so acquire no rules for this styleSheet.
          // just jump through this styleSheet and go to analyzing next.
          return pre
        }
        rules.push({
          selectorText: `.${match[1]}`,
          cssText: match[2].trim()
        })
      }
      Array.from(rules).forEach(rule => {
        const selector = rule.selectorText || ''
        pre[selector] = trimComment(rule.cssText).split(';')
          .reduce((styleObj, statement) => {
            statement = statement.trim()
            if (statement && statement.indexOf('/*') <= -1) {
              const resArr = statement.split(':').map((part) => part.trim())
              styleObj[resArr[0]] = resArr[1]
            }
            return styleObj
          }, {})
      })
      return pre
    }, {})
}

export default {
  beforeCreate () {
    // get static class style map from document's styleSheets.
    if (!weex.styleMap) {
      weex.styleMap = getHeadStyleMap()
    }
  },

  methods: {

    _normalizeInlineStyles (data) {
      const style = extend({}, data.staticStyle, data.style)
      const res = normalizeStyles(style)
      return res
    },

    _getSize (data) {
      const wh = {}
      const _scopeId = this._getScopeId && this._getScopeId()
      const style = data.style
      const staticStyle = data.staticStyle
      const classes = typeof data.class === 'string' ? [data.class] : (data.class || [])
      const staticClass = typeof data.staticClass === 'string' ? [data.staticClass] : (data.staticClass || [])
      const styleMap = weex.styleMap
      function extendWHFrom (to, from) {
        if (!from) { return }
        to.width = from.width
        to.height = from.height
      }
      let i, len
      // merge from staticClass.
      i = 0
      len = staticClass.length
      while (i < len) {
        let cls = ''
        if (_scopeId) {
          cls = `.${staticClass[i]}[${_scopeId}]`
        }
        extendWHFrom(wh, styleMap[cls])
        i++
      }
      // merge from classes.
      i = 0
      len = classes.length
      while (i < len) {
        let cls = ''
        if (_scopeId) {
          cls = `.${classes[i]}[${_scopeId}]`
        }
        extendWHFrom(wh, styleMap[cls])
        i++
      }
      extendWHFrom(wh, staticStyle)
      extendWHFrom(wh, style)
      return wh
    },

    // get style from class, staticClass, style and staticStyle.
    _getComponentStyle (data) {
      const style = {}
      const _scopeId = this._getScopeId && this._getScopeId()
      const staticClassNames = (typeof data.staticClass === 'string') ? [data.staticClass] : (data.staticClass || [])
      const classNames = (typeof data.class === 'string') ? [data.class] : (data.class || [])
      const styleMap = weex.styleMap

      /**
       * merge styles. priority: high -> low
       *  1. data.style (bound style).
       *  2. data.staticStyle (inline styles).
       *  3. data.class style (bound class names).
       *  4. data.staticClass style (scoped styles or static classes).
       */
      let i, len
      i = 0
      len = staticClassNames.length
      while (i < len) {
        let cls = ''
        if (_scopeId) {
          cls = `.${staticClassNames[i]}[${_scopeId}]`
        }
        extend(style, styleMap[cls])
        i++
      }
      i = 0
      len = classNames.length
      while (i < len) {
        let cls = ''
        if (_scopeId) {
          cls = `.${classNames[i]}[${_scopeId}]`
        }
        extend(style, styleMap[cls])
        i++
      }
      const res = normalizeStyles(extend(style, data.staticStyle, data.style))
      return res
    },

    // merge static styles and static class styles into $vnode.data.mergedStyles.
    _mergeStyles () {
      const vnode = this.$options._parentVnode || {}
      const data = vnode.data
      if (!data) { return }
      this.$options._parentVnode.data.staticStyle = this._getComponentStyle(data)
    },

    _getParentRect () {
      const parentElm = this.$options._parentElm
      return parentElm && parentElm.getBoundingClientRect()
    },

    _getParentRectAsync (cb) {
      this.$nextTick(function () {
        return cb && cb.call(this, this.getParentRectSync())
      })
    }
  }
}
