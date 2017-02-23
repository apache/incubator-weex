/**
 * remove comments from a cssText.
 */
export function trimComment (cssText) {
  return cssText.replace(/(?:\/\*)[^*]*\*\//g, '')
}
