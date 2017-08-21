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
'use strict';

var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var util = require("../util.js");
var assert = require('chai').assert

describe('border test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/css/border.js'))
      .waitForElementById('test-text',util.getGETActionWaitTimeMills() + 2000,1000)
  });

  afterEach(function () {
    return util.quit(driver);
  })


  it('#1 border screenshot diff', () => {
    return driver
    .sleep(10000)
    .takeScreenshot()
    .then(imgData => {
      var newImg = new Buffer(imgData, 'base64');
      var screenshotFolder = path.resolve(__dirname, '../../screenshot');
      var oldImgPath = path.join(screenshotFolder, process.env.platform === 'android' ? 'border-android.png' : 'border-ios.png');
      var diffImgPath = path.join(screenshotFolder, process.env.platform === 'android' ? 'border-android-diff.png' : 'border-ios-diff.png');
      return util.diffImage(oldImgPath, newImg, 100, diffImgPath);
    })
    .then(result => {
      console.log(result)
      assert.isOk(result)
    })
  })
});


