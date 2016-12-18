
export function computeWrapperSize (context) {
  const wrapper = context.$refs.wrapper
  if (wrapper) {
    const rect = wrapper.getBoundingClientRect()
    context.wrapperWidth = rect.width
    context.wrapperHeight = rect.height
  }
}

export function reachTop (context) {
  const wrapper = context.$refs.wrapper
  return (!!wrapper) && (wrapper.scrollTop <= 0)
}

export function reachBottom (context) {
  const wrapper = context.$refs.wrapper
  const inner = context.$refs.inner
  const offset = Number(context.loadmoreoffset) || 0

  if (wrapper && inner) {
    const innerHeight = inner.getBoundingClientRect().height
    const wrapperHeight = wrapper.getBoundingClientRect().height
    return wrapper.scrollTop >= innerHeight - wrapperHeight - offset
  }
  return false
}
