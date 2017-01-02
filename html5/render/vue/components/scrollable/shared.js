import refresh from './refresh'
import loading from './loading'

export function createLoading (context, createElement) {
  return createElement(loading, {
    on: {
      loading: () => {
        context.$emit('loading', context.createCustomEvent('loading'))
      }
    }
  })
}

export function createRefresh (context, createElement) {
  return createElement(refresh, {
    on: {
      refresh: () => {
        context.$emit('refresh', context.createCustomEvent('loading'))
      }
    }
  })
}
