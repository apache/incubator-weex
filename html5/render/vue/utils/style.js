import { normalize } from './flex'

/**
 * remove comments from a cssText.
 */
export function trimComment (cssText) {
  return cssText.replace(/(?:\/\*)[^*]*\*\//g, '')
}

/**
 * add flex prefixes for compatibility conisderation.
 */
export function normalizeStyles (style) {
  return normalize(style)
}
