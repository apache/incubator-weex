// { "framework": "Vanilla" }

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

var body = document.createElement('div', {
  classStyle: { alignItems: 'center', marginTop: 120 }
})

var image = document.createElement('image', {
  attr: { src: 'https://alibaba.github.io/weex/img/weex_logo_blue@3x.png' },
  classStyle: { width: 360, height: 82 }
})

var text = document.createElement('text', {
  attr: { value: 'Hello World' },
  classStyle: { fontSize: 48 }
})

body.appendChild(image)
body.appendChild(text)
document.documentElement.appendChild(body)

body.addEvent('click', function () {
  text.setAttr('value', 'Hello Weex')
})
