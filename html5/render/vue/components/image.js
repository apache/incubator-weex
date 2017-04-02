/**
 * get resize (stetch|cover|contain) related styles.
 */
function getResizeStyle (context) {
  const stretch = '100%'
  const resize = context.resize || stretch
  const bgSize = ['cover', 'contain', stretch].indexOf(resize) > -1 ? resize : stretch
  // compatibility: http://caniuse.com/#search=background-size
  return { 'background-size': bgSize }
}

function preProcessSrc (context, url, mergedStyle) {
  // somehow the merged style in _prerender hook is gone.
  // just return the original src.
  if (!mergedStyle || !mergedStyle.width || !mergedStyle.height) {
    return url
  }
  const { width, height } = mergedStyle
  return context.processImgSrc && context.processImgSrc(url, {
    width: parseFloat(width),
    height: parseFloat(height),
    quality: context.quality,
    sharpen: context.sharpen,
    original: context.original
  }) || url
}

export default {
  props: {
    src: String,
    placeholder: String,
    resize: String,
    quality: String,
    sharpen: String,
    original: [String, Boolean]
  },

  updated () {
    this._fireLazyload()
  },

  mounted () {
    this._fireLazyload()
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('image', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    // const style = this._normalizeInlineStyles(this.$vnode.data)
    const wh = this._getSize(this.$vnode.data)
    return createElement('figure', {
      attrs: {
        'weex-type': 'image',
        'img-src': preProcessSrc(this, this.src, wh),
        'img-placeholder': preProcessSrc(this, this.placeholder, wh)
      },
      on: this._createEventMap(['load', 'error']),
      staticClass: 'weex-image weex-el',
      staticStyle: getResizeStyle(this)
    })
  }
}
