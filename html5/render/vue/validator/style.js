
/**
 * Validate the CSS color value.
 */
export function isCSSColor (value) {
  return /^[a-z]+$/i.test(value) // match color name
    || /^#([a-f0-9]{3}){1,2}$/i.test(value) // match #ABCDEF
    || /^rgb\s*\((\s*[0-9.]+\s*,){2}\s*[0-9.]+\s*\)/i.test(value) // match rgb(0,0,0)
    || /^rgba\s*\((\s*[0-9.]+\s*,){3}\s*[0-9.]+\s*\)/i.test(value) // match rgba(0,0,0,0)
}

export const color = isCSSColor
export const backgroundColor = isCSSColor

export function position (value) {
  return ['absolute', 'relative'].indexOf(value) !== -1
}

export function opacity (value) {
  const count = parseFloat(value)
  return count >= 0 && count <= 1
}
