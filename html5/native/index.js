import '../shared'
import runtime from '../runtime'
import { subversion } from '../../package.json'

const { native, transformer } = subversion

// register instance management APIs
for (const methodName in runtime) {
  global[methodName] = (...args) => {
    const ret = runtime[methodName](...args)
    if (ret instanceof Error) {
      console.error(ret.toString())
    }
    return ret
  }
}

// register framework meta info
global.frameworkVersion = native
global.transformerVersion = transformer
