import { extend } from '../../utils'
// import header from './header'
import refresh from './refresh'
import loading from './loading'

// export function createHeader (context, createElement) {
//   return createElement(header)
// }

export function createLoading (context, createElement, vnode) {
  const options = vnode.componentOptions
  return createElement(loading, extend(vnode.data, {
    on: options.listeners
  }), options.children)
}

export function createRefresh (context, createElement, vnode) {
  const options = vnode.componentOptions
  return createElement(refresh, extend(vnode.data, {
    on: options.listeners
  }), options.children)
}
