import { extend, hyphenate, trimComment } from '../utils'
// import { validateStyles } from '../validator'

// let warned = false

function getHeadStyleMap () {
  return Array.from(document.styleSheets || [])
    .reduce((pre, styleSheet) => {
      // why not using styleSheet.rules || styleSheet.cssRules to get css rules ?
      // because weex's components defined non-standard style attributes, which is
      // auto ignored when access rule.cssText.
      const strArr = styleSheet.ownerNode.textContent.trim().split('.')
      const len = strArr.length
      const rules = []
      for (let i = 0; i < len; i++) {
        const str = strArr[i]
        if (!str) {
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
              styleObj[hyphenate(resArr[0])] = resArr[1]
            }
            return styleObj
          }, {})
      })
      return pre
    }, {})
}

// function getWarnText (prop) {
//   return `[Vue Rneder] "${prop}" is not a standard CSS property,`
//     + 'it may not support very well on weex vue render.'
// }

// function normalize (styles) {
//   const realStyle = {}
//   for (const key in styles) {
//     let value = styles[key]

//     // TODO: add more reliable check
//     if (typeof value === 'number') {
//       value += 'px'
//     }

//     // warn for unsupported properties
//     switch (key) {
//       case 'lines':
//       case 'item-color':
//       case 'itemColor':
//       case 'item-selected-color':
//       case 'itemSelectedColor':
//       case 'item-size':
//       case 'itemSize': console.warn(getWarnText(key)); break
//     }

//     realStyle[key] = value
//   }
//   return realStyle
// }

// function getStyleMap (component) {
//   if (component && component.$vnode && component.$vnode.context) {
//     const $options = component.$vnode.context.$options
//     if ($options && $options.style) {
//       if (!warned) {
//         warned = true
//         console.error('[Invalid Bundle Format] This bundle format is '
//           + 'generated for Android and iOS platform, '
//           + 'please use "vue-loader" to compile the ".vue" file on the web.')
//       }
//       return $options.style
//     }
//   }
// }

// function getStaticClass (component) {
//   if (component && component.$vnode && component.$vnode.data) {
//     const data = component.$vnode.data
//     return [].concat(data.staticClass, data.class)
//   }
// }

// function getComponentStyle (context) {
  // const styleMap = getStyleMap(context)
  // const staticClass = getStaticClass(context)

  // if (styleMap && Array.isArray(staticClass)) {
  //   const styles = staticClass.reduce((res, name) => {
  //     return extend(res, styleMap[name])
  //   }, {})

  //   return normalize(styles)
  // }
// }

// function mergeStyles (context) {
//   const styles = getComponentStyle(context)
//   if (context.$el && styles) {
//     validateStyles(context.$options && context.$options._componentTag, styles)
//     for (const key in styles) {
//       context.$el.style[key] = styles[key]
//     }
//   }
// }

export default {
  beforeCreate () {
    // get static class style map from document's styleSheets.
    if (!weex.styleMap) {
      weex.styleMap = getHeadStyleMap()
      Object.freeze(weex)
    }
  },
  // mounted () {
  //   console.log('call mounted: merged styles')
  //   mergeStyles(this)
  // },
  // beforeUpdate () {
  //   console.log('call beforeUpdate: merged styles')
  //   mergeStyles(this)
  // },

  methods: {
    prerender () {
      this.mergeStyles()
    },

    // get style from staticClass and staticStyle.
    getComponentStyle () {
      const style = {}
      const data = this.$vnode && this.$vnode.data || {}
      const _scopeId = this.getScopeId && this.getScopeId()
      const hyphenatedStaticStyle = {}
      const staticStyle = data.staticStyle || {}
      const classNames = (data.staticClass || '').split(' ')

      Object.keys(staticStyle).forEach(name => {
        hyphenatedStaticStyle[hyphenate(name)] = staticStyle[name]
      })

      // apply static class styles. This relies on getHeadStyleMap
      // being already triggered once in the hook beforeCreate.
      if (weex.styleMap) {
        classNames.forEach(className => {
          const styleObj = weex.styleMap[`.${className}${_scopeId ? `[${_scopeId}]` : ''}`] || {}
          extend(style, styleObj)
        })
      }

      // apply static inline styles.
      extend(style, hyphenatedStaticStyle)

      return style
    },

    // merge static styles and static class styles into $vnode.data.mergedStyles.
    mergeStyles () {
      if (this.$vnode && this.$vnode.data) {
        this.$vnode.data.mergedStyle = this.getComponentStyle()
      }
    },

    getParentRect () {
      const parentElm = this.$options._parentElm
      return parentElm && parentElm.getBoundingClientRect()
    },

    getParentRectAsync (cb) {
      this.$nextTick(function () {
        return cb && cb.call(this, this.getParentRectSync())
      })
    }
  }
}
