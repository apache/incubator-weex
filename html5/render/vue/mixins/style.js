import { extend } from '../utils'

function normalize (value) {
  // TODO: add more reliable check
  if (typeof value === 'number') {
    return value + 'px'
  }
  return value
}

function normalizeStyle (styles) {
  const realStyle = {}
  if (styles) {
    for (const key in styles) {
      realStyle[key] = normalize(styles[key])
    }
  }
  return realStyle
}

function getStyleMap (component) {
  if (component && component.$vnode) {
    const context = component.$vnode.context
    if (context && context.$options) {
      return context.$options.style || {}
    }
  }
  return {}
}

function getStaticClass (component) {
  if (component && component.$vnode && component.$vnode.data) {
    return component.$vnode.data.staticClass
  }
}

function getComponentStyle (context) {
  const styleMap = getStyleMap(context)
  const staticClass = getStaticClass(context)
  if (Array.isArray(staticClass)) {
    const styles = staticClass.reduce((res, name) => {
      return extend(res, styleMap[name])
    }, {})
    return normalizeStyle(styles)
  }
  return {}
}

function appendStyle (element, styles) {
  for (const key in styles) {
    element.style[key] = styles[key]
  }
}

export default {
  mounted () {
    appendStyle(this.$el, getComponentStyle(this))
  }
}
