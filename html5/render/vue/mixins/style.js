import { extend } from '../utils'
import { validateStyles } from '../validator'

let warned = false

function normalize (value) {
  // TODO: add more reliable check
  if (typeof value === 'number') {
    return value + 'px'
  }
  return value
}

function getStyleMap (component) {
  if (component && component.$vnode && component.$vnode.context) {
    const $options = component.$vnode.context.$options
    if ($options && $options.style) {
      if (!warned) {
        console.warn(`[Vue Render] Please use vue-loader to compile the .vue file.`)
        warned = true
      }
      return $options.style
    }
  }
}

function getStaticClass (component) {
  if (component && component.$vnode && component.$vnode.data) {
    return component.$vnode.data.staticClass
  }
}

function getComponentStyle (context) {
  const styleMap = getStyleMap(context)
  const staticClass = getStaticClass(context)

  if (styleMap && Array.isArray(staticClass)) {
    const styles = staticClass.reduce((res, name) => {
      return extend(res, styleMap[name])
    }, {})

    const realStyle = {}
    for (const key in styles) {
      realStyle[key] = normalize(styles[key])
    }
    return realStyle
  }
}

export default {
  mounted () {
    const styles = getComponentStyle(this)
    if (this.$el && styles) {
      validateStyles(this.$options && this.$options._componentTag, styles)
      for (const key in styles) {
        this.$el.style[key] = styles[key]
      }
    }
  }
}
