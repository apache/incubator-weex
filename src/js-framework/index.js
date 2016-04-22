import './polyfill'
import * as framework from './lib/framework'
import {version, optionalDependencies} from './package.json'

for (let methodName in framework) {
    global[methodName] = function (...args) {
        const ret = framework[methodName](...args)
        if (ret instanceof Error) {
            console.error(ret.toString())
        } else if (ret instanceof Array) {
            ret.forEach(r => {
                if (ret instanceof Error) {
                    console.error(r.toString())
                }
            })
        }
        return ret
    }
}

Object.assign(global, {
    frameworkVersion: version,
    needTransformerVersion: optionalDependencies['weex-transformer']
})

/**
 * register methods
 */
const methods = require('./lib/api/methods')
const {registerMethods} = global
registerMethods(methods)
