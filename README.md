# Weex

A framework for building Mobile cross-platform UI.

[![Build Status](https://travis-ci.org/apache/incubator-weex.svg?branch=master)](https://travis-ci.org/apache/incubator-weex/)

| platform | status |
| -------- | ------ |
| Android | [![Download](https://api.bintray.com/packages/alibabaweex/maven/weex_sdk/images/download.svg)](https://bintray.com/alibabaweex/maven/weex_sdk/_latestVersion) |
| iOS | [![Pod version](https://badge.fury.io/co/WeexSDK.svg)](https://cocoapods.org/pods/WeexSDK) [![Carthage compatible](https://img.shields.io/badge/Carthage-compatible-4BC51D.svg?style=flat)](https://github.com/Carthage/Carthage) |
| [Mobile Web](https://github.com/weexteam/weex-vue-render) | [![npm version](https://badge.fury.io/js/weex-html5.svg)](https://www.npmjs.com/package/weex-vue-render) |

Support Android 4.1 (API 16), iOS 8.0+ and WebKit 534.30+.

**Note: Please note the above download distribution is only for users' users' convenience. Weex Release is always in the format of [source code](https://weex.apache.org/download/download.html).**

## For Windows

Please ***INSTALL [Git for Windows](https://git-scm.com/download/win)*** and run all the following commands in git-bash.

## Meet Weex

* Install [Weex Playground App](https://weex.apache.org/tools/playground.html) to see examples we already written.
* If you want to write a demo, install [weex-toolkit](https://www.npmjs.com/package/weex-toolkit) in [Node.js](http://nodejs.org/) 8.0+ and
* Run `weex init` to generate & start a simple project in an empty folder.
* Follow the instructions in the project README.
* Enjoy it.

## Use Weex

* [Tutorial](http://weex.apache.org/guide/)
* [Documents](http://weex.apache.org/references)

### Android

* Prerequisites
  * Install [Node.js](http://nodejs.org/) 8.0+
  * Under project root
    * `npm install`, install project
    * `./start`
    * Install [Android Environment](http://developer.android.com/training/basics/firstapp/index.html)
    * Install [NDK](https://developer.android.com/ndk/) r16
    * Install [Cmake](https://cmake.org/download/) 3.9.0+
* Run playground, In Android Studio
    * Open `android/playground`
    * In `app/java/com.alibaba.weex/IndexActivity`, modify `CURRENT_IP` to your local IP
    * Click <img src="http://gtms04.alicdn.com/tps/i4/TB1wCcqMpXXXXakXpXX3G7tGXXX-34-44.png" height="16" > (`Run` button)
* [Add an example](./examples/README.md#add-an-example)

#### Runtime

On Android Platform , Weex code is executed in [weex_v8core](https://github.com/alibaba/weex_v8core) which is based on Google V8 JavaScript engine.

### iOS
* run playground
  * Prerequisites
    * Install [Node.js](http://nodejs.org/) 8.0+
      * Under project root
          * `npm install`, install project
          * `./start`
      * Install [iOS Environment](https://developer.apple.com/library/ios/documentation/IDEs/Conceptual/AppStoreDistributionTutorial/Setup/Setup.html)
      * Install [CocoaPods](https://guides.cocoapods.org/using/getting-started.html)
  * Run playground
      * `cd ios/playground`
      * `pod install`
      * Open `WeexDemo.xcworkspace` in Xcode
      * Click <img src="http://img1.tbcdn.cn/L1/461/1/5470b677a2f2eaaecf412cc55eeae062dbc275f9" height="16" > (`Run` button) or use default shortcut `cmd + r` in Xcode
      * If you want to run the demo on your device, don't need to modify `CURRENT_IP` manually. ~~In `DemoDefine.h`(you can search this file by Xcode default shortcut `cmd + shift + o`), modify `CURRENT_IP` to your local IP~~
  * [Add an example](./examples/README.md#add-an-example)
* integrate to your application

  - **[CocoaPods](https://cocoapods.org)**

     Add the following line to your Podfile:
   ```
     pod 'WeexSDK'
   ```
    run `pod install`

  - **[Carthage](https://github.com/carthage/carthage)**

    Add the following line to your Cartfile:
   ```
     github "apache/incubator-weex"
   ```
   Run `carthage update`, and you should now have the latest version of   `WeexSDK` in your `Carthage` folder.

### Mobile Web

see [weex-vue-render](https://github.com/weexteam/weex-vue-render).

## Scripts

See [SCRIPTS.md](./SCRIPTS.md) for more information.

## IDE Plugin & Syntax Highlight & DevTool

Weex team have developed a [DevTool](https://github.com/weexteam/weex-devtool) to help you to improve the debugging efficiency.

See more stuff on [this wiki page](https://github.com/alibaba/weex/wiki/Weex-Community)

## Weex Community
* [Mailing List](https://weex-project.io/contributing.html#join-in-discussions) Weex Mailing List, where most discussion happens.
* [StackOverflow](http://stackoverflow.com/questions/tagged/weex): Ask questions about Weex.
* [SegmentFault (cn)](https://segmentfault.com/t/weex): 中文交流与讨论
* [FAQ](https://weex.apache.org/faq.html)
* [Articles (cn)](https://github.com/weexteam/article/issues): Weex 相关文章集合
* [Telegram Russian Community Group](https://telegram.me/weex_ru)

### Contributing

See [Weex Contributing Guide](./CONTRIBUTING.md) for more information.
