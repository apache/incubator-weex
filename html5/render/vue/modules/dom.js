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
import { camelToKebab, appendCss } from '../utils'

function getParentScroller (vnode) {
  if (!vnode) return null
  if (vnode.weexType === 'scroller' || vnode.weexType === 'list') {
    return vnode
  }
  return getParentScroller(vnode.$parent)
}

function now () {
  const now = window.performance && window.performance.now
      ? window.performance.now.bind(window.performance) : Date.now
  return now()
}

function scrollElement (dSuffix, position) {
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
  if (currentPosition === context.position) {
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

export default {
  /**
   * scrollToElement
   * @param  {String} vnode
   * @param  {Object} options {offset:Number}
   *   ps: scroll-to has 'ease' and 'duration'(ms) as options.
   */
  scrollToElement: function (vnode, options) {
    const scroller = getParentScroller(vnode)
    const scrollDirection = scroller.scrollDirection || 'vertical'

    if (scroller && scroller.$el && vnode.$el) {
      // if it's a list, then the listVnode.scrollDirection is undefined. just
      // assum it is the default value 'vertical'.
      const dSuffix = ({
        horizontal: 'Left',
        vertical: 'Top'
      })[scrollDirection]
      let offset = vnode.$el[`offset${dSuffix}`]

      if (options) {
        offset += Number(options.offset) || 0
        offset *= weex.config.env.scale /* adapt offset to different screen scales. */
      }
      else if (process.env.NODE_ENV === 'development') {
        console.warn('[Vue Render] The second parameter of "scrollToElement" is required, '
          + 'otherwise it may not works well on native.')
      }

      if (options && options.animated === false) {
        return scrollElement.call(scroller.$el, dSuffix, offset)
      }

      step({
        scrollable: scroller.$el,
        startTime: now(),
        frame: null,
        startPosition: scroller.$el[`scroll${dSuffix}`],
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
    appendCss(styleText, 'dom-added-rules')
  }
}
