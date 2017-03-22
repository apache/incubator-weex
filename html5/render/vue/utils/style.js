/**
 * remove comments from a cssText.
 */
export function trimComment (cssText) {
  return cssText.replace(/(?:\/\*)[^*]*\*\//g, '')
}

let support = null

export function supportSticky () {
  if (support !== null) {
    return support
  }
  const element = window.document.createElement('div')
  const elementStyle = element.style
  elementStyle.cssText = 'position:-webkit-sticky;position:sticky;'
  support = elementStyle.position.indexOf('sticky') !== -1
  return support
}
