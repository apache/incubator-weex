import {
  camelizeKeys,
  // hyphenateKeys,
  extend,
  trimComment,
  normalizeStyle
} from '../utils'
import { tagBegin, tagEnd } from '../utils/perf'
/* istanbul ignore next */
import addPrefix from 'inline-style-prefixer/static'

/**
 * get scoped class style map from stylesheets in <head>.
 */
export function getHeadStyleMap () {
  if (process.env.NODE_ENV === 'development') {
    tagBegin('getHeadStyleMap')
  }
  const needToRemoveStyleSheetNodes = []
  const res = Array.from(document.styleSheets || [])
    .reduce((pre, styleSheet) => {
      /**
       * why not using styleSheet.rules || styleSheet.cssRules to get css rules ?
       * because weex's components defined non-standard style attributes, which is
       * auto ignored when access rule.cssText.
       */
      if (!styleSheet.cssRules) {
        /**
         * no rules. just ignore this. probably a link stylesheet.
         */
        return pre
      }
      const strArr = trimComment(styleSheet.ownerNode.textContent.trim()).split(/}/)
      const len = strArr.length
      const rules = []
      for (let i = 0; i < len; i++) {
        const str = strArr[i]
        if (!str || str.match(/^\s*$/)) {
          continue
        }
        /**
         * should match these cases:
         * .a[data-v-xxx] { color: red }
         * .a[data-v-xxx], .b[data-v-xxx] { color: red; }
         *
         * should not match these cases:
         * .a { color: red; }
         * etc.
         */
        const match = str.match(/((?:,?\s*\.[\w-]+\[data-v-\w+\](?::\w+)?)+)\s*({[^}]+)/)
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
        const styleObj = trimComment(rule.cssText).split(';')
          .reduce((styleObj, statement) => {
            statement = statement.trim()
            if (statement && statement.indexOf('/*') <= -1) {
              const resArr = statement.split(':').map((part) => part.trim())
              styleObj[resArr[0]] = resArr[1]
            }
            return styleObj
          }, {})
        const res = pre[selector]
        if (!res) {
          pre[selector] = styleObj
        }
        else {
          extend(pre[selector], styleObj)
        }
      })
      /**
       * remove this styleSheet node since it's in the styleMap already. And this style
       * should only be fetched and used from styleMap to generate the final combined
       * component style, not from the stylesheet itself.
       */
      needToRemoveStyleSheetNodes.push(styleSheet.ownerNode)
      return pre
    }, {})
  if (!window._no_remove_style_sheets) {
    needToRemoveStyleSheetNodes.forEach(function (node) {
      node.parentNode.removeChild(node)
    })
  }
  else if (process.env.NODE_ENV === 'development') {
    console.warn(`[vue-render] you've defined '_no_remove_style_sheets' and the v-data-xx stylesheets will not be removed.`)
  }
  if (process.env.NODE_ENV === 'development') {
    tagEnd('getHeadStyleMap')
  }
  return res
}

export function getScopeIds (context) {
  const arr = []
  let ctx = context
  let scopeId
  while (ctx) {
    scopeId = ctx.$options._scopeId
    scopeId && arr.push(scopeId)
    ctx = ctx.$options.parent
  }
  return arr
}

/**
 * get style in <style scoped> tags for this component.
 */
export function getScopeStyle (context, classNames) {
  const scopeIds = getScopeIds(context)
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
}

/**
 * get style merged with static styles, binding styles, and scoped class styles,
 * with keys in camelcase.
 */
export function getComponentStyle (context) {
  if (!context.$vnode) {
    if (process.env.NODE_ENV === 'development') {
      return console.error('[vue-render] getComponentStyle failed: no $vnode in context.')
    }
    return {}
  }
  const cached = context.$vnode.data.cached
  const data = context.$vnode.data
  const staticClassNames = (typeof data.staticClass === 'string') ? data.staticClass.split(' ') : (data.staticClass || [])
  const classNames = (typeof data.class === 'string') ? data.class.split(' ') : (data.class || [])
  const clsNms = staticClassNames.concat(classNames)
  const style = getScopeStyle(context, clsNms)
  const res = extend(style, cached, data.staticStyle, data.style)
  context.$vnode.data.cached = res
  return addPrefix(normalizeStyle(camelizeKeys(res)))
}

export function extractComponentStyle (context) {
  const style = getComponentStyle(context)
  if (style) {
    delete context.$vnode.data.staticStyle
    delete context.$vnode.data.style
  }
  return style
}

/**
 * get { width, height } (size) of current component from components' styles.
 */
export function getSize (context) {
  if (!context.$vnode) {
    if (process.env.NODE_ENV === 'development') {
      return console.error('[vue-render] getComponentStyle failed: no $vnode in context.')
    }
    return {}
  }
  const data = context.$vnode.data
  const wh = {}
  const classes = typeof data.class === 'string' ? data.class.split(' ') : (data.class || [])
  const staticClass = typeof data.staticClass === 'string' ? data.staticClass.split(' ') : (data.class || [])
  const clsNms = staticClass.concat(classes)
  function extendWHFrom (to, from) {
    if (!from) { return }
    from.width && (to.width = from.width)
    from.height && (to.height = from.height)
  }
  extendWHFrom(wh, this._getScopeStyle(clsNms))
  extendWHFrom(wh, data.staticStyle)
  extendWHFrom(wh, data.style)
  return wh
}
