'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('weex text', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  before(function () {
    return util.init(driver)
      .get(util.getPage('/components/text-layout.js'))
      .waitForElementById("lines", util.getGETActionWaitTimeMills(),1000)
  });

  after(function () {
      // return util.quit(driver);
  })


  let scaleFactor = 0 
  let screenWidth = 0
  it ('#1 Window size', () =>{
    return driver
    .getWindowSize()
    .then(size=>{
      screenWidth =  size.width
      scaleFactor = screenWidth/ 750
    })
  })

  it('#2 Text Content', () => {
    return driver
    .elementById('text-only')
    .text()
    .then((text)=>{
      assert.equal(text,'Hello World')
    })
  })

  it ('#3 Line Height', () =>{
    return driver
    .elementById('lines')
    .getRect()
    .then(rect => {
       console.log(rect)
       console.log(screenWidth)
       assert.equal(rect.width, screenWidth)
       assert.equal(rect.height, Math.floor(2 * 32 * scaleFactor))
       scroll(rect.height)
    })
  })

   it ('#4 Font Size', () =>{
    return driver
    .elementById('font')
    .getRect()
    .then(rect => {
       assert.equal(rect.width, screenWidth)
       assert.isAtLeast(rect.height, Math.floor(2 * 48 * scaleFactor))
       scroll(rect.height)
    })
  })

   it ('#5 Fixed-Size', () =>{
    return driver
    .elementById('fixed-size')
    .getRect()
    .then(rect => {
       assert.equal(rect.width, Math.floor(300 * scaleFactor))
       assert.equal(rect.height, Math.floor(100 * scaleFactor))
       scroll(rect.height)
    })
  })

   it ('#6 flex:1; align-Items: stretch; flex-direction:row', () =>{
    return driver
    .elementById('flexgrow-alignitems')
    .getRect()
    .then(rect => {
       assert.equal(rect.width, Math.floor(500 * scaleFactor))
       assert.equal(rect.height, Math.floor(300 * scaleFactor))
       scroll(rect.height)
    })
   })
  
   it ('#7 flex:1; align-Items: center; flex-direction:row', () =>{
    return driver
    .elementById('flexgrow')
    .getRect()
    .then(rect => {
       assert.equal(rect.width, Math.floor(500 * scaleFactor))
       assert.equal(rect.height, Math.floor(40 * scaleFactor))
       scroll(rect.height)
    })
   })

   it ('#8 flex:1; align-Items: stretch; flex-direction:column', () =>{
    return driver
    .elementById('flexgrow-alignitems-coloumn')
    .getRect()
    .then(rect => {
       assert.equal(rect.width, Math.floor(500 * scaleFactor))
       assert.equal(rect.height, Math.floor(300 * scaleFactor))
       scroll(rect.height)
    })
   })
  
   it ('#9 flex:1; align-Items: auto; flex-direction:column', () =>{
    return driver
    .elementById('flexgrow-column')
    .getRect()
    .then(rect => {
       assert.isBelow(rect.width, 500 * scaleFactor/2)
       assert.equal(rect.height, Math.floor(300 * scaleFactor))
       scroll(rect.height)
    })
   })

   function scroll(height){
      driver.touch('drag', {
        fromX:100,
        fromY:300+height,
        toX:  100,
        toY: 300,
        duration: 0.5
       })
   }
});
