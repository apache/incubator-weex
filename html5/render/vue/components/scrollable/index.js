import list from './list'
import scroller from './scroller'
import waterfall from './waterfall'
import cell from './cell'
import header from './header'
import loading from './loading'
import refresh from './refresh'
import loadingIndicator from './loading-indicator'

import './style.css'

const modules = [
  list,
  scroller,
  waterfall,
  cell,
  header,
  loading,
  refresh,
  loadingIndicator
]

export default {
  init (weex) {
    modules.forEach(function (mod) {
      weex.install(mod)
    })
  }
}
