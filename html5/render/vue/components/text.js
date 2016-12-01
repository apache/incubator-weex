import { extend } from '../utils'
import { validateStyles } from '../validator'

/**
 * Get text styles
 */
function getTextStyle (context = {}) {
  const { data, props } = context
  const style = {
    whiteSpace: 'pre-wrap',
    wordWrap: 'break-word',
    display: '-webkit-box',
    webkitBoxOrient: 'vertical'
  }

  const lines = parseInt(props.lines) || 0
  if (lines > 0) {
    style.overflow = 'hidden'
    style.textOverflow = 'ellipsis'
    style.webkitLineClamp = lines
  }
  else {
    style.overflow = 'visible'
    style.textOverflow = ''
    style.webkitLineClamp = ''
  }

  return extend(style, data.staticStyle)
}

export default {
  functional: true,
  props: {
    lines: [Number, String],
    value: [String]
  },
  render (createElement, context) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV !== 'production') {
      validateStyles('text', context.data && context.data.staticStyle)
    }

    let className = 'weex-container'
    if (context.data.staticClass) {
      className += ' ' + context.data.staticClass
    }

    return createElement(
      'div',
      {
        staticClass: className,
        staticStyle: { fontSize: '32px' }
      },
      [createElement(
        'span',
        { staticStyle: getTextStyle(context) },
        context.children || [context.props.value]
      )]
    )
  }
}
