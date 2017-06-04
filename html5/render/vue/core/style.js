/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
import {
  camelizeKeys,
  // hyphenateKeys,
  extend,
  extendTruthy,
  trimComment,
  normalizeStyle,
  autoPrefix
} from '../utils'
import { tagBegin, tagEnd } from '../utils/perf'
/* istanbul ignore next */

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
       * another reason not to use cssRules directy:
       * @issue: https://stackoverflow.com/questions/21642277/security-error-the-operation-is-insecure-in-firefox-document-stylesheets
       */
      if ((styleSheet.ownerNode.tagName.toLowerCase() === 'link')
        || !styleSheet.ownerNode.textContent) {
        /**
         * css in a link. just ignore this. probably a link stylesheet.
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

// export function getScopeIds (context) {
//   const arr = []
//   let ctx = context
//   let scopeId
//   while (ctx) {
//     scopeId = ctx.$options._scopeId
//     scopeId && arr.push(scopeId)
//     ctx = ctx.$options.parent
//   }
//   return arr
// }

export function getScopeId (vnode) {
  return vnode.context.$options._scopeId
}

/**
 * get style in <style scoped> tags for this component.
 */
export function getScopeStyle (vnode, classNames) {
  const scopeId = getScopeId(vnode)
  const style = {}
  const styleMap = weex.styleMap || {}
  let clsNmsIdx = 0
  const clsNmsLen = classNames.length
  while (clsNmsIdx < clsNmsLen) {
    const cls = `.${classNames[clsNmsIdx]}[${scopeId}]`
    const map = styleMap[cls]
    map && extendTruthy(style, map)
    clsNmsIdx++
  }
  return camelizeKeys(style)
}

function getStyle (vnode, extract) {
  const data = vnode.data || {}
  const staticClassNames = (typeof data.staticClass === 'string') ? data.staticClass.split(' ') : (data.staticClass || [])
  const classNames = (typeof data.class === 'string') ? data.class.split(' ') : (data.class || [])
  const clsNms = staticClassNames.concat(classNames)
  const style = normalizeStyle(getScopeStyle(vnode, clsNms))
  /**
   * cache static style and bind style.
   * cached staticStyle (including style and staticStyle) has already been normalized
   * in $processStyle. So there's no need to normalize it again.
   */
  if (!data.cached) {
    // cache staticStyle once in the beginning.
    data.cached = extendTruthy({}, data.staticStyle)
  }
  // cache binding style every time since the binding style is variable.
  extendTruthy(data.cached, data.style)
  extend(style, data.cached)
  data.staticStyle = style
  if (extract) {
    delete data.staticStyle
    delete data.style
  }
  return style
}

/**
 * get style merged with static styles, binding styles, and scoped class styles,
 * with keys in camelcase.
 */
export function getComponentStyle (context, extract) {
  if (!context.$vnode) {
    if (process.env.NODE_ENV === 'development') {
      return console.error('[vue-render] getComponentStyle failed: no $vnode in context.')
    }
    return {}
  }
  let style = {}
  let vnode = context.$vnode
  while (vnode) {
    extend(style, getStyle(vnode, extract))
    vnode = vnode.parent
  }
  style = autoPrefix(style)
  /**
   * when prefixed value is a array, it should be applied to element
   * during the next tick.
   * e.g.
   *  background-image:  linear-gradient(to top,#f5fefd,#ffffff);
   *  will generate:
   *  {
   *    backgroundImage: [
   *      "-webkit-linear-gradient(to top,#f5fefd,#ffffff)",
   *      "-moz-linear-gradient(to top,#f5fefd,#ffffff)",
   *      "linear-gradient(to top,#f5fefd,#ffffff)"]
   *  }
   */
  for (const k in style) {
    if (Array.isArray(style[k])) {
      const vals = style[k]
      context.$nextTick(function () {
        const el = context.$el
        if (el) {
          for (let i = 0; i < vals.length; i++) {
            el.style[k] = vals[i]
          }
        }
      })
      delete style[k]
    }
  }
  return style
  // return addPrefix(normalizeStyle(style))
}

export function extractComponentStyle (context) {
  return getComponentStyle(context, true)
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
