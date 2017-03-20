import { extend, trimComment, normalizeStyles } from '../utils'
import { tagBegin, tagEnd } from '../utils/perf'
// import { validateStyles } from '../validator'

// let warned = false

// function hyphenateExtend (to, from) {
//   if (!from) { return }
//   for (const k in from) {
//     to[hyphenate(k)] = from[k]
//   }
// }

function getHeadStyleMap () {
  if (process.env.NODE_ENV === 'development') {
    tagBegin('getHeadStyleMap')
  }
  const res = Array.from(document.styleSheets || [])
    .reduce((pre, styleSheet) => {
      // why not using styleSheet.rules || styleSheet.cssRules to get css rules ?
      // because weex's components defined non-standard style attributes, which is
      // auto ignored when access rule.cssText.
      const strArr = trimComment(styleSheet.ownerNode.textContent.trim()).split(/}/)
      const len = strArr.length
      const rules = []
      for (let i = 0; i < len; i++) {
        const str = strArr[i]
        if (!str || str.match(/^\s*$/)) {
          continue
        }

        const match = str.match(/((?:,?\s*\.[[\]\w-]+\s*)+)({[^}]+)/)
        if (!match) {
          // not the vue static class styles map. so acquire no rules for this styleSheet.
          // just jump through this styleSheet and go to analyzing next.
          return pre
        }
        const clsNms = match[1].split(',').map(n => n.trim())
        const cssText = match[2].replace(/[{}]/g, '').trim()
        let clsNmsIdx = 0
        const clsNmsLen = clsNms.length
        while (clsNmsIdx < clsNmsLen) {
          rules.push({
            selectorText: clsNms[clsNmsIdx],
            cssText
          })
          clsNmsIdx++
        }
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
  if (process.env.NODE_ENV === 'development') {
    tagEnd('getHeadStyleMap')
  }
  return res
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

    _getScopeStyle (classNames) {
      const scopeIds = this._getScopeIds()
      const style = {}
      const styleMap = weex.styleMap
      let map
      let cls
      let clsNmsIdx
      let scpIdsIdx
      const clsNmsLen = classNames.length
      const scpIdsLen = scopeIds.length
      if (clsNmsLen <= 0) {
        return {}
      }
      clsNmsIdx = 0
      while (clsNmsIdx < clsNmsLen) {
        scpIdsIdx = 0
        while (scpIdsIdx < scpIdsLen) {
          cls = `.${classNames[clsNmsIdx]}[${scopeIds[scpIdsIdx]}]`
          map = styleMap[cls]
          if (!map) {
            scpIdsIdx++
            continue
          }
          for (const k in map) {
            style[k] = map[k]
          }
          scpIdsIdx++
        }
        clsNmsIdx++
      }
      return style
    },

    _getSize (data) {
      const wh = {}
      const style = data.style
      const staticStyle = data.staticStyle
      const classes = typeof data.class === 'string' ? data.class.split(' ') : (data.class || [])
      const staticClass = typeof data.staticClass === 'string' ? data.staticClass.split(' ') : (data.class || [])
      const clsNms = staticClass.concat(classes)
      function extendWHFrom (to, from) {
        if (!from) { return }
        from.width && (to.width = from.width)
        from.height && (to.height = from.height)
      }
      extendWHFrom(wh, this._getScopeStyle(clsNms))
      extendWHFrom(wh, staticStyle)
      extendWHFrom(wh, style)
      return wh
    },

    // get style from class, staticClass, style and staticStyle.
    _getComponentStyle (data) {
      const staticClassNames = (typeof data.staticClass === 'string') ? data.staticClass.split(' ') : (data.staticClass || [])
      const classNames = (typeof data.class === 'string') ? data.class.split(' ') : (data.class || [])
      /**
       * merge styles. priority: high -> low
       *  1. data.style (bound style).
       *  2. data.staticStyle (inline styles).
       *  3. data.class style (bound class names).
       *  4. data.staticClass style (scoped styles or static classes).
       */
      const clsNms = staticClassNames.concat(classNames)
      const style = this._getScopeStyle(clsNms)
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
