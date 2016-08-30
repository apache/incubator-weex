import Vm from '../../../../default/vm'
import { isPlainObject } from '../../../utils'
import {
  isWeexComponent,
  isNpmModule,
  removeWeexPrefix,
  removeJSSurfix
} from '../../../../default/app/bundle/misc'

export function bootstrap (app, name, config, data) {
  console.debug(`[JS Framework] bootstrap for ${name}`)

  let cleanName

  if (isWeexComponent(name)) {
    cleanName = removeWeexPrefix(name)
  }
  else if (isNpmModule(name)) {
    cleanName = removeJSSurfix(name)
    // check if define by old 'define' method
    /* istanbul ignore if */
    if (!app.customComponentMap[cleanName]) {
      return new Error(`It's not a component: ${name}`)
    }
  }
  else {
    return new Error(`Wrong component name: ${name}`)
  }

  config = isPlainObject(config) ? config : {}

  console.log(`transformerVersion: ${config.transformerVersion}, `
    + `available transformerVersion: ${global.transformVersion}`)

  app.vm = new Vm(cleanName, null, { _app: app }, null, data)
}
