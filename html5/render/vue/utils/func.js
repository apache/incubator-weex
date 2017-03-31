// @flow

/**
 * Mix properties into target object.
 * the rightest object's value has the highest priority.
 */
export function extend (to: {}, ...args: Array<{}>): {} {
  if (!args || args.length <= 0) { return to }
  args.forEach(from => {
    if (typeof from !== 'object') { return }
    for (const key in from) {
      to[key] = from[key]
    }
  })
  return to
}

/**
 * Mix specified properties into target object.
 */
export function extendKeys (to: {}, from: {} = {}, keys: Array<string>): {} {
  (keys || []).forEach(key => {
    from && (to[key] = from[key])
  })
  return to
}

/**
 * Extract specified properties from src to target object.
 */
export function extractKeys (to: {}, from: {} = {}, keys: Array<string>) {
  if (!from) { return to }
  (keys || []).forEach(key => {
    from && (to[key] = from[key])
    from && (delete from[key])
  })
  return to
}

/**
 * Simple bind, faster than native
 *
 * @param {Function} fn
 * @param {Object} ctx
 * @return {Function}
 */
export function bind (fn: Function, ctx: mixed) {
  return function (a: mixed) {
    const l = arguments.length
    return l
      ? l > 1
        ? fn.apply(ctx, arguments)
        : fn.call(ctx, a)
      : fn.call(ctx)
  }
}

/**
 * Only call the func the last time before it's not that frequently called.
 */
export function debounce (func: Function, wait: number) {
  let timerId
  function later () {
    timerId = null
    func.apply(null)
  }
  return function () {
    clearTimeout(timerId)
    timerId = setTimeout(later, wait)
  }
}

/**
 * Only call the func the first time before a series frequently function calls happen.
 */
export function depress (func: Function, wait: number) {
  let timerId
  function later () {
    timerId = null
  }
  return function () {
    if (!timerId) {
      func.apply()
    }
    clearTimeout(timerId)
    timerId = setTimeout(later, wait)
  }
}

/**
 * Only call the func every time after a wait milliseconds if it's too frequently called.
 */
export function throttle (func: Function, wait: number, callLastTime: boolean) {
  let last = 0
  let lastTimer = null
  const lastTimeDuration = wait + (wait > 25 ? wait : 25)  // plus half wait time.
  return function (...args: Array<mixed>) {
    const context = this
    const time = new Date().getTime()
    if (time - last > wait) {
      if (callLastTime) {
        lastTimer && clearTimeout(lastTimer)
        lastTimer = setTimeout(function () {
          lastTimer = null
          func.apply(context, args)
        }, lastTimeDuration)
      }
      func.apply(context, args)
      last = time
    }
  }
}
