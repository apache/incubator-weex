// import scroll from 'scroll-to'
import { camelToKebab, appendStyle } from '../utils'

export default {
  /**
   * scrollToElement
   * @param  {String} vnode
   * @param  {Object} options {offset:Number}
   *   ps: scroll-to has 'ease' and 'duration'(ms) as options.
   */
  scrollToElement: function (vnode, options) {
    // TODO
  },

  /**
   * getComponentRect
   * @param {String} vnode
   * @param {Function} callback
   */
  getComponentRect: function (vnode, callback) {
    const info = { result: false }

    if (vnode && vnode === 'viewport') {
      info.result = true
      info.size = {
        width: document.documentElement.clientWidth,
        height: document.documentElement.clientHeight,
        top: 0,
        left: 0,
        right: document.documentElement.clientWidth,
        bottom: document.documentElement.clientHeight
      }
    }
    else if (vnode && vnode.$el) {
      info.result = true
      info.size = vnode.$el.getBoundingClientRect()
    }

    const message = info.result ? info : {
      result: false,
      errMsg: 'Illegal parameter'
    }

    callback && callback(message)
    return message
  },

  /**
   * for adding fontFace
   * @param {string} key fontFace
   * @param {object} styles rules
   */
  addRule: function (key, styles) {
    key = camelToKebab(key)
    let stylesText = ''
    for (const k in styles) {
      if (styles.hasOwnProperty(k)) {
        stylesText += camelToKebab(k) + ':' + styles[k] + ';'
      }
    }
    const styleText = `@${key}{${stylesText}}`
    appendStyle(styleText, 'dom-added-rules')
  }
}
