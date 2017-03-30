// @flow

/**
 * remove comments from a cssText.
 */
export function trimComment (cssText: string): string {
  return cssText.replace(/(?:\/\*)[^*]*\*\//g, '')
}

let support: boolean | null = null

export function supportSticky (): boolean {
  if (support !== null) {
    return support
  }
  const element = window.document.createElement('div')
  const elementStyle = element.style
  elementStyle.cssText = 'position:-webkit-sticky;position:sticky;'
  support = elementStyle.position.indexOf('sticky') !== -1
  return support
}
