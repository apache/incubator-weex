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

/**
 * viewport priority:
 *
 * 1. meta weex-viewport (developer custom)
 * 2. setViewport(config) := config.width (private code) @deprecated
 * 3. process.env.VIEWPORT_WIDTH (buid time)
 *
 */

let viewportWidth = process.env.VIEWPORT_WIDTH

const wxViewportMeta = document.querySelector('meta[name="weex-viewport"]')
const metaWidth = wxViewportMeta && parseInt(wxViewportMeta.getAttribute('content'))
if (metaWidth && !isNaN(metaWidth) && metaWidth > 0) { viewportWidth = metaWidth }

/**
 * set root font-size for rem units. If already been set, just skip this.
 */
function setRootFont (doc, width) {
  const rootFontSize = doc.documentElement.style.fontSize
  if (!rootFontSize) {
    doc.documentElement.style.fontSize = width / 10 + 'px'
  }
}

export function setViewport (config = {}) {
  const doc = window.document

  if (doc) {
    // set root font for rem.
    setRootFont(doc, viewportWidth)

    /**
     * why not to use window.screen.width to get screenWidth ? Because in some
     * old webkit browser on android system it get the device pixel width, which
     * is the screenWidth multiply by the device pixel ratio.
     */
    const deRect = document.documentElement.getBoundingClientRect()
    const screenWidth = deRect.width
    const screenHeight = deRect.height
    const scale = screenWidth / viewportWidth

    /**
     * if set initial/maximum/mimimum-scale some how the page will have a bounce
     * effect when user drag the page towards horizontal axis.
     */
    const contents = [
      `width=${viewportWidth}`,
      // `initial-scale=${scale}`,
      // `maximum-scale=${scale}`,
      // `minimum-scale=${scale}`,
      `user-scalable=no`
    ]

    let meta = doc.querySelector('meta[name="viewport"]')
    if (!meta) {
      meta = doc.createElement('meta')
      meta.setAttribute('name', 'viewport')
      document.querySelector('head').appendChild(meta)
    }
    meta.setAttribute('content', contents.join(','))

    const dpr = window.devicePixelRatio

    return {
      scale,
      deviceWidth: screenWidth * dpr,
      deviceHeight: screenHeight * dpr
    }
  }
}
