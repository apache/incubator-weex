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
