/**
 * Create a cached version of a pure function.
 */
export function cached (fn) {
  const cache = Object.create(null)
  return function cachedFn (str) {
    const hit = cache[str]
    return hit || (cache[str] = fn(str))
  }
}

/**
 * Camelize a hyphen-delmited string.
 */
const camelizeRE = /-(\w)/g
export const camelize = cached(str => {
  return str.replace(camelizeRE, (_, c) => c.toUpperCase())
})

/**
 * Capitalize a string.
 */
export const capitalize = cached(str => {
  return str.charAt(0).toUpperCase() + str.slice(1)
})

/**
 * Hyphenate a camelCase string.
 */
const hyphenateRE = /([^-])([A-Z])/g
export const hyphenate = cached(str => {
  return str
    .replace(hyphenateRE, '$1-$2')
    .replace(hyphenateRE, '$1-$2')
    .toLowerCase()
})

/**
 * Mix properties into target object.
 */
export function extend (to, _from) {
  for (const key in _from) {
    to[key] = _from[key]
  }
  return to
}

/**
 * Simple bind, faster than native
 *
 * @param {Function} fn
 * @param {Object} ctx
 * @return {Function}
 */
export function bind (fn, ctx) {
  return function (a) {
    const l = arguments.length
    return l
      ? l > 1
        ? fn.apply(ctx, arguments)
        : fn.call(ctx, a)
      : fn.call(ctx)
  }
}

export function debounce (func, wait) {
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

export function throttle (func, wait) {
  let last = 0
  return function (...args) {
    const context = this
    const time = new Date().getTime()
    if (time - last > wait) {
      func.apply(context, args)
      last = time
    }
  }
}

export function createMixin (...mixins) {
  const mixinMethods = {}
  mixins.forEach(methods => {
    for (const key in methods) {
      mixinMethods[key] = function (...args) {
        return methods[key].apply(this, [this, ...args])
      }
    }
  })
  return {
    methods: mixinMethods
  }
}
