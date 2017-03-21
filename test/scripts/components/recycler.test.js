'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

const isApproximate = (x, y) =>  {
  return Math.abs(x - y) < 0.1
}

describe('recycler', function () {
  this.timeout(util.getTimeoutMills())
  const driver = util.createDriver(wd)

  before(function () {
    return util.init(driver)
      .get('wxpage://' + util.getDeviceHost() +'/components/recycler.js')
      .waitForElementByXPath('//div[1]/text[1]',util.getGETActionWaitTimeMills(),1000)
  });

  after(function () {
      return util.quit(driver);
  })

  let scaleFactor = 0
  let screenHeight = 0
  let navBarHeight = 0
  it('#1 test recyler layout', () => {
    return driver
    .getWindowSize()
    .then(size=>{
      scaleFactor = size.width / 750
      screenHeight = size.height
    })
    .sleep(2000)
    .elementByXPath('//recycler[1]')
    .getRect()
    .then((rect)=>{
      console.log(`scale factor:${scaleFactor}`)
      console.log(`recycler rect:${JSON.stringify(rect)}`)
      navBarHeight = rect.y
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.width, 750 * scaleFactor))
      assert.isOk(isApproximate(rect.height, screenHeight - navBarHeight))
    })
  })

  it('#2 test header 1 layout', () => {
    return driver
    .elementByXPath('//recycler[1]/div[1]/')
    .getRect()
    .then((rect)=>{
      console.log(`header 1 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight))
      assert.isOk(isApproximate(rect.width, 750 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 377 * scaleFactor))
    })
  })

  it('#3 test header 2 layout', () => {
    return driver
    .elementByXPath('//recycler[1]/div[2]/')
    .getRect()
    .then((rect)=>{
      console.log(`header 2 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 377 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 750 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 94 * scaleFactor))
    })
  })

  it('#4 test cell 1 layout', () => {
    return driver
    .elementByXPath('//recycler[1]/cell[1]/')
    .getRect()
    .then((rect)=>{
      console.log(`cell 1 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 0))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 502 * scaleFactor))
    })
  })

  it('#5 test cell 2 layout', () => {
    return driver
    .elementByXPath('//recycler[1]/cell[2]/')
    .getRect()
    .then((rect)=>{
      console.log(`cell 2 rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 381 * scaleFactor))
      assert.isOk(isApproximate(rect.y, navBarHeight + 471 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 369 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 472 * scaleFactor))
    })
  })

  it('#10 test fixed', () => {
    return driver
    .elementByXPath('//div[1]')
    .getRect()
    .then((rect)=>{
      console.log(`scale factor:${scaleFactor}`)
      console.log(`fixed rect:${JSON.stringify(rect)}`)
      assert.isOk(isApproximate(rect.x, 640 * scaleFactor))
      assert.isOk(isApproximate(rect.y, screenHeight - 110 * scaleFactor))
      assert.isOk(isApproximate(rect.width, 78 * scaleFactor))
      assert.isOk(isApproximate(rect.height, 78 * scaleFactor))
    })
  })

});


