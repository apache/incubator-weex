/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
export function setViewport (app, configs = {}) {
  /* istanbul ignore if */
  if (process.env.NODE_ENV === 'development') {
    console.debug(`[JS Framework] Set viewport (width: ${configs.width}) for app#${app.id}.`)
    validateViewport(configs)
  }

  // Send viewport configs to native
  if (app && app.callTasks) {
    return app.callTasks([{
      module: 'meta',
      method: 'setViewport',
      args: [configs]
    }])
  }

  /* istanbul ignore next */
  else if (process.env.NODE_ENV === 'development') {
    console.warn(`[JS Framework] Can't find "callTasks" method on current app.`)
  }
}

/**
 * Validate the viewport config.
 * @param {Object} configs
 */
export function validateViewport (configs = {}) {
  const { width } = configs
  if (width) {
    if (typeof width !== 'number' && width !== 'device-width') {
      console.warn(`[JS Framework] Not support to use ${width} as viewport width.`)
      return false
    }
    return true
  }
  console.warn('[JS Framework] the viewport config should contain the "width" property.')
  return false
}
