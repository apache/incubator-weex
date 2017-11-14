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
import config from '../config'
const utils = {}

function getParentScroller (vnode) {
  if (!vnode) return null
  const vm = vnode.$el
  ? vnode : vnode.elm
  ? vnode.componentInstance || vnode.context : null
  if (!vm) return null
  const type = vm.$el && vm.$el.getAttribute('weex-type')
  if (config.scrollableTypes.indexOf(type) > -1) {
    return vm
  }
  return getParentScroller(vm.$parent)
}

function now () {
  const now = window.performance && window.performance.now
  ? window.performance.now.bind(window.performance) : Date.now
  return now()
}

function scrollElement (dSuffix, position) {
  if ((this === document.body) || (this === window)
    && window.scrollTo) {
    return window.scrollTo(0, position)
  }
  this[`scroll${dSuffix}`] = position
}
/**
 * self invoked function that, given a context, steps through scrolling
 * @method step
 * @param {Object} context
 */
function step (context) {
  // call method again on next available frame
  context.frame = window.requestAnimationFrame(step.bind(window, context))
  const time = now()
  let elapsed = (time - context.startTime) / 468
  // avoid elapsed times higher than one
  elapsed = elapsed > 1 ? 1 : elapsed
  // apply easing to elapsed time
  const value = ease(elapsed)
  const currentPosition = context.startPosition + (context.position - context.startPosition) * value
  context.method.call(context.scrollable, context.dSuffix, currentPosition)
  // return when end points have been reached
  /**
    * NOTE: should use ~~ to parse position number into integer. Otherwise
    * this two float numbers maybe have a slicely little difference, which
    * will cause this function never to stop.
  */
  if (~~currentPosition === ~~context.position) {
    window.cancelAnimationFrame(context.frame)
    return
  }
}
/**
 * returns result of applying ease math function to a number
 * @method ease
 * @param {Number} k
 * @returns {Number}
 */
function ease (k) {
  return 0.5 * (1 - Math.cos(Math.PI * k))
}
const dom = {
  /**
   * scrollToElement
   * @param  {Vnode | VComponent} vnode
   * @param  {Object} options {offset:Number}
   *   ps: scroll-to has 'ease' and 'duration'(ms) as options.
   */
  scrollToElement: function (vnode, options) {
    const {
      isArray
    } = utils
    if (isArray(vnode)) {
      if (process.env.NODE_ENV === 'development') {
        console.warn('[vue-render] the ref passed to animation.transitionOnce is a array.')
      }
      vnode = vnode[0]
    }
    const scroller = getParentScroller(vnode)
    const scrollDirection = scroller && scroller.scrollDirection || 'vertical'
    const isWindow = !scroller
    const ct = isWindow ? document.body : scroller.$el
    const el = vnode.$el || vnode.elm
    if (ct && el) {
      // if it's a list, then the listVnode.scrollDirection is undefined. just
      // assum it is the default value 'vertical'.
      const dSuffix = ({
        horizontal: 'Left',
        vertical: 'Top'
      })[scrollDirection]
      const ctRect = ct.getBoundingClientRect()
      let elRect = el.getBoundingClientRect()
      /**
        * if it's a waterfall, and you want to scroll to a header, then just
        * scroll to the top.
      */
      if (scroller && scroller.weexType === 'waterfall' && scroller._headers && scroller._headers.indexOf(vnode.$vnode || vnode) > -1) {
        // it's in waterfall. just scroll to the top.
        elRect = ct.firstElementChild.getBoundingClientRect()
      }
      const dir = dSuffix.toLowerCase()
      let offset = (isWindow ? 0 : ct[`scroll${dSuffix}`]) + elRect[dir] - ctRect[dir]
      if (options) {
        offset += options.offset && options.offset * weex.config.env.scale || 0
        // offset *= weex.config.env.scale /* adapt offset to different screen scales. */
      }
      else if (process.env.NODE_ENV === 'development') {
        console.warn('[Vue Render] The second parameter of "scrollToElement" is required, '
        + 'otherwise it may not works well on native.')
      }
      if (options && options.animated === false) {
        return scrollElement.call(ct, dSuffix, offset)
      }
      step({
        scrollable: ct,
        startTime: now(),
        frame: null,
        startPosition: isWindow ? window.pageYOffset : ct[`scroll${dSuffix}`],
        position: offset,
        method: scrollElement,
        dSuffix: dSuffix
      })
    }
  },
  /**
   * getComponentRect
   * @param {String} vnode
   * @param {Function} callback
   */
  getComponentRect: function (vnode, callback) {
    const {
      isArray
    } = utils
    if (isArray(vnode)) {
      if (process.env.NODE_ENV === 'development') {
        console.warn('[vue-render] the ref passed to animation.transitionOnce is a array.')
      }
      vnode = vnode[0]
    }
    const scale = window.weex.config.env.scale
    const info = {
      result: false
    }
    const rectKeys = ['width', 'height', 'top', 'bottom', 'left', 'right']

    function recalc (rect) {
      const res = {}
      rectKeys.forEach(key => {
        if (rect[key]) {
          res[key] = rect[key] / scale
        }
      })
      return res
    }
    if (vnode && vnode === 'viewport') {
      info.result = true
      info.size = recalc({
        width: document.documentElement.clientWidth,
        height: document.documentElement.clientHeight,
        top: 0,
        left: 0,
        right: document.documentElement.clientWidth,
        bottom: document.documentElement.clientHeight
      })
      info.contentSize = recalc({
        width: document.documentElement.offsetWidth,
        height: document.documentElement.offsetHeight
      })
    }
    else if (vnode && vnode.$el) {
      info.result = true
      info.size = recalc(vnode.$el.getBoundingClientRect())
      if (vnode.$refs.inner) {
        info.contentSize = recalc({
          width: vnode.$refs.inner.offsetWidth,
          height: vnode.$refs.inner.offsetHeight
        })
      }
      else {
        info.contentSize = recalc({
          width: vnode.$el.offsetWidth,
          height: vnode.$el.offsetHeight
        })
      }
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
    const {
      camelToKebab,
      appendCss
    } = utils
    key = camelToKebab(key)
    let stylesText = ''
    for (const k in styles) {
      if (styles.hasOwnProperty(k)) {
        stylesText += camelToKebab(k) + ':' + styles[k] + ';'
      }
    }
    const styleText = `@${key}{${stylesText}}`
    appendCss(styleText, 'dom-added-rules')
  }
}
export default {
  init (weex) {
    const extendKeys = weex.utils.extendKeys
    extendKeys(utils, weex.utils, ['camelToKebab', 'appendCss', 'isArray'])
    weex.registerModule('dom', dom)
  }
}
