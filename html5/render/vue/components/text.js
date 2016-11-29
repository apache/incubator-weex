import { validateText } from '../utils/validator'

/**
 * Get text styles
 */
function getTextStyle (props = {}) {
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

  return style
}

export default {
  functional: true,
  props: {
    lines: [Number, String],
    value: [String]
  },
  render (createElement, context) {
    if (process.env.NODE_ENV === 'development') {
      validateText(context)
    }
    const { props } = context
    return createElement(
      'div',
      {
        staticClass: 'weex-container',
        staticStyle: { fontSize: '32px' }
      },
      [createElement(
        'span',
        { staticStyle: getTextStyle(props) },
        context.children || [props.value]
      )]
    )
  }
}
