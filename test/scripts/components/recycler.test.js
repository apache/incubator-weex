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

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

const platform = process.env.platform.toLowerCase() || 'android';
const isAndroid = platform === 'android';

const isApproximate = (x, y) =>  {
  return Math.abs(x - y) <= (isAndroid ? 2 : 1.5)
}

describe('recycler @ignore-android @ignore-ios', function () {
  this.timeout(util.getTimeoutMills())
  const driver = util.createDriver(wd)

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/recycler.js'))
      .waitForElementById('waterfall',util.getGETActionWaitTimeMills(),1000)
  });

  afterEach(function () {
      return util.quit(driver);
  })

  let scaleFactor = 0
  let screenHeight = 0
  let recyclerWidth = 0
  let navBarHeight = 0
  let cell1Height = 0
  let cell2Height = 0
  it('#1 test recyler layout', () => {
    return driver
    .getWindowSize()
    .then(size=>{
      scaleFactor = size.width / 750
      screenHeight = size.height
      recyclerWidth = 750 * scaleFactor
      console.log(`screen size:${JSON.stringify(size)}`)
      console.log(`scale factor:${scaleFactor}`)
      console.log(`recyclerWidth:${recyclerWidth}`)
      console.log(`screenHeight:${screenHeight}`)
    })
    .sleep(2000)
    .elementById('waterfall')
    .getRect()
    .then((rect)=>{
      console.log(`recycler rect:${JSON.stringify(rect)}`)
      navBarHeight = rect.y
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, screenHeight - navBarHeight))
    })
    .elementById('header1')
    .getRect()
    .then((rect)=>{
      console.log(`header 1 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, 377 * scaleFactor))
    })
    .elementById('header2')
    .getRect()
    .then((rect)=>{
      console.log(`header 2 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 377 * scaleFactor))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
    .elementById('cell0')
    .getRect()
    .then((rect)=>{
      console.log(`cell 1 rect:${JSON.stringify(rect)}`)
      cell1Height = rect.height
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
    .elementById('cell1')
    .getRect()
    .then((rect)=>{
      console.log(`cell 2 rect:${JSON.stringify(rect)}`)
      cell2Height = rect.height
      assert.isOk(isApproximate(rect.x, 381 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
    .elementById('cell2')
    .getRect()
    .then((rect)=>{
      console.log(`cell 3 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 381 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell2Height))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
    .elementById('cell3')
    .getRect()
    .then((rect)=>{
      console.log(`cell 4 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell1Height))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
  })

  it('#2 test column count', () => {
    return driver
    .elementById('cell2')
    .click()
    .elementById('cell0')
    .getRect()
    .then((rect)=>{
      console.log(`cell 1 rect after changing column count to 3:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 242 * scaleFactor))
    })
    .elementById('cell1')
    .getRect()
    .then((rect)=>{
      console.log(`cell 2 rect after changing column count to 3:${JSON.stringify(rect)}`)
      cell2Height = rect.height
      assert.isOk(isApproximate(rect.x, 254 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 242 * scaleFactor))
    })
   .elementById('cell2')
    .getRect()
    .then((rect)=>{
      console.log(`cell 3 rect after changing column count to 3:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 508 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 242 * scaleFactor))
    })
   .elementById('cell3')
    .getRect()
    .then((rect)=>{
      console.log(`cell 4 rect after changing column count to 3:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 254 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell2Height))
      assert.isOk(isApproximate(rect.width, 242 * scaleFactor))
    })
    .elementById('cell2')
    .click()
  })

  it('#3 test column gap', () => {
    return driver
    .elementById('cell1')
    .click()
    .elementById('cell0')
    .getRect()
    .then((rect)=>{
      console.log(`cell 1 rect after changing column gap to normal:${JSON.stringify(rect)}`)
      cell1Height = rect.height
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 359 * scaleFactor))
    })
     .elementById('cell1')
    .getRect()
    .then((rect)=>{
      console.log(`cell 2 rect after changing column gap to normal:${JSON.stringify(rect)}`)
      cell2Height = rect.height
      assert.isOk(isApproximate(rect.x, 391 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 359 * scaleFactor))
    })
    .elementById('cell2')
    .getRect()
    .then((rect)=>{
      console.log(`cell 3 rect after changing column gap to normal:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 391 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell2Height))
      assert.isOk(isApproximate(rect.width, 359 * scaleFactor))
    })
     .elementById('cell3')
    .getRect()
    .then((rect)=>{
      console.log(`cell 4 rect after changing column gap to normal:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell1Height))
      assert.isOk(isApproximate(rect.width, 359 * scaleFactor))
    })
    .elementById('cell1')
    .click()
  })

  it('#4 test column width', () => {
    return driver
     .elementById('cell0')
    .click()
    .elementById('cell0')
    .getRect()
    .then((rect)=>{
      console.log(`cell 1 rect after changing column width to 600:${JSON.stringify(rect)}`)

      cell1Height = rect.height
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
    })
    .elementById('cell1')
    .getRect()
    .then((rect)=>{
      console.log(`cell 2 rect after changing column width to 600:${JSON.stringify(rect)}`)
      cell2Height = rect.height
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor + cell1Height))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
    })
    .elementById('cell0')
    .click()
  })

  it('#5 test deleting header', () => {
    return driver
    .elementById('header2')
    .click()
     .elementById('header1')
    .getRect()
    .then((rect)=>{
      console.log(`header 2 rect after deleting header 1:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
    })
   .elementById('header2')
    .click()
  })

  it('#6 test footer', () => {
    return driver
   .elementById('fixed1')
    .click()
    .sleep(500)
    .elementById('footer1')
    .getRect()
    .then((rect)=>{
      console.log(`footer rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, screenHeight - 94 * scaleFactor))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
  })

  it('#7 test sticky and fixed', () => {
    return driver
   .elementById('fixed1')
    .getRect()
    .then((rect)=>{
      console.log(`fixed rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 640 * scaleFactor))
      assert.isOk(isApproximate(rect.y, screenHeight - 110 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 78 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 78 * scaleFactor))
    })
    .elementById('header2')
    .getRect()
    .then((rect)=>{
      console.log(`sticky header rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight))
      assert.isOk(isApproximate(rect.width, recyclerWidth))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
  })

  it('#8 test removing cell', () => {
    return driver
    .elementById('cell28')
    .click()
    .elementById('cell27')
    .getRect()
    .then((rect)=>{
      console.log(`cell 28 rect after removing cell 29:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, screenHeight - 94 * scaleFactor - rect.height))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
  })

  it('#9 test moving cell', () => {
    return driver
    .elementById('cell29')
    .click()
    .elementById('fixed1')
    .click()
    .elementById('cell27')
    .getRect()
    .then((rect)=>{
      console.log(`cell 28 rect after moving cell 30 to 1:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.y, screenHeight - 94 * scaleFactor - rect.height))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
    })
  })

  it('#10 test recycler padding', () => {
    return driver
    .elementById('cell26')
    .click()
    .elementById('fixed1')
    .click()
    .elementById('header2')
    .getRect()
    .then((rect)=>{
      console.log(`sticking header rect after setting padding to 12:${JSON.stringify(rect)}`)

      assert.isOk(isApproximate(rect.x, 12 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + (isAndroid ? 12 * scaleFactor : 0)))
      assert.isOk(isApproximate(rect.width, recyclerWidth - 24 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
    .elementById('footer1')
    .getRect()
    .then((rect)=>{
      console.log(`footer rect after setting padding to 12:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 12 * scaleFactor))
      assert.isOk(isApproximate(rect.y, screenHeight - 106 * scaleFactor))
      assert.isOk(isApproximate(rect.width, recyclerWidth - 24 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
    .elementById((isAndroid ? 'cell26' : 'cell27'))
    .getRect()
    .then((rect)=>{
      console.log(`cell 27 rect after setting padding to 12:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 12 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 357 * scaleFactor))
    })
  })

  it('#11 test onscroll', () => {
    let originContentOffset = 0
    return driver
    .elementById((isAndroid ? 'cell27' : 'cell28'))
    .click()
    .elementById('stickyText1')
    .text()
    .then(text => {
      console.log(text)
      originContentOffset = Number.parseInt(text.replace('Content Offset:-',''))
    })
    .touch('drag', {fromX:recyclerWidth / 2, fromY:screenHeight / 5, toX:recyclerWidth / 2, toY: screenHeight * 4 / 5, duration: 2})
    .sleep(1000)
    .touch('drag', {fromX:recyclerWidth / 2, fromY:screenHeight / 5, toX:recyclerWidth / 2, toY: screenHeight * 4 / 5, duration: 2})
    .elementById('stickyText1')
    .text()
    .then(text => {
      console.log(text)
      const contentOffset = Number.parseInt(text.replace('Content Offset:-',''))
      assert.isOk(originContentOffset - contentOffset > screenHeight / scaleFactor)
    })
    .elementById('fixed1')
    .click()
  })

  it('#12 test scrollable', () => {
    let originContentOffset = 0
    return driver
    .elementById((isAndroid ? 'cell25' : 'cell26'))
    .click()
    .elementById('stickyText1')
    .text()
    .then(text => {
      console.log(text)
      originContentOffset = Number.parseInt(text.replace('Content Offset:-',''))
    })
    .touch('drag', {fromX:recyclerWidth / 2, fromY:screenHeight / 5, toX:recyclerWidth / 2, toY: screenHeight * 4 / 5,duration: 2})
    .elementById('stickyText1')
    .text()
    .then(text => {
      console.log(text)
      const contentOffset = Number.parseInt(text.replace('Content Offset:-',''))
      assert.equal(contentOffset, originContentOffset)
    })
  })
});


