import '../shared'
import { subversion } from '../../package.json'
import { init, config } from '../runtime'
import frameworks from '../config'
config.frameworks = frameworks
const { native, transformer } = subversion

// register framework meta info
global.frameworkVersion = native
global.transformerVersion = transformer

// init frameworks
const globalMethods = init(config)

// set global methods
for (const methodName in globalMethods) {
  global[methodName] = (...args) => {
    const ret = globalMethods[methodName](...args)
    if (ret instanceof Error) {
      console.error(ret.toString())
    }
    return ret
  }
}
