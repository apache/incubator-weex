---
title: Release Note
type: releasenote
layout: post
---

# Release Note

## v0.17
-----

* `feature` `Android/iOS` Support `writing direction style:direction=rtl`([#782](https://github.com/apache/incubator-weex/pull/782)[#886](https://github.com/apache/incubator-weex/pull/886))
* `feature` `Android/iOS` Support scroll start and scroll end event on scroller and list ([#858](https://github.com/apache/incubator-weex/pull/858)[856](https://github.com/apache/incubator-weex/pull/856))
* `feature` `iOS` support text max-width （[#834](https://github.com/apache/incubator-weex/pull/834)）
* `feature` `Android` CSS Transiton Animation Supported component ([#851](https://github.com/apache/incubator-weex/pull/851))
* `feature` `Android` New `local` module ([#781](https://github.com/apache/incubator-weex/pull/781))
* `feature` `Android` Support ripple background on Android 5.0 and higher ([#792](https://github.com/apache/incubator-weex/pull/792))
* `feature` `Android` Support multi language on dialog ([#831](https://github.com/apache/incubator-weex/pull/831))
* `feature` `H5` Support lazyload and appear watcher when body height set to 100% ([#827](https://github.com/apache/incubator-weex/pull/827)).
* `feature` `H5` Add try catch for storage accessing incase user disabled the related function in a browser ([#827](https://github.com/apache/incubator-weex/pull/827)).
* `feature` `H5` image support css sprite (sprite-src, sprite-position, sprite-width) ([#827](https://github.com/apache/incubator-weex/pull/827)).
* `feature` `JSFM` Support batch update styles and attributes in Vue.js ([#819](https://github.com/apache/incubator-weex/pull/819) [#7046](https://github.com/vuejs/vue/pull/7046))
* `feature` `JSFM` Stop trimming CSS units in richtext component. ([#6927](https://github.com/vuejs/vue/pull/6927))
* `feature` `JSFM` Stop rethrow the captured error on Weex platform. ([#7024](https://github.com/vuejs/vue/pull/7024))
* `feature` `JSFM` Upgrade weex-vue-framework to 2.5.3 ([release nodes](https://github.com/vuejs/vue/releases/tag/v2.5.3))
* `feature` `JSFM` Adjust the behavior of `nextTick` to improve compatibility.
* `bugfix` `iOS` bugfix boxshadow render abnormal （[#791](https://github.com/apache/incubator-weex/pull/791)）
* `bugfix` `iOS` bugfix timer exposed on JSContxt （[#839](https://github.com/apache/incubator-weex/pull/839)）
* `bugfix` `iOS` fix iOS8 scrollview’s assign delegate crash （[#838](https://github.com/apache/incubator-weex/pull/838)）
* `bugfix` `iOS` fix setViewport：sometimes doesn’t work（[#843](https://github.com/apache/incubator-weex/pull/843)）
* `bugfix` `iOS` fix addEvent lead to generate a new view while it as been recycled （[#837](https://github.com/apache/incubator-weex/pull/837)）
* `bugfix` `iOS` fix about setting nan frame crash （[#853](https://github.com/apache/incubator-weex/pull/853)）
* `bugfix` `iOS` disable tableview estimation row or section height which make list component behavior abnormal （[#867](https://github.com/apache/incubator-weex/pull/867)）
* `bugfix` `Android` Fix that moveElement doesn’t work when parent is not a list ([#805](https://github.com/apache/incubator-weex/pull/805))
* `bugfix` `Android` Fix flicker caused by coexistence of box-shadow and border-radius (#[780](https://github.com/apache/incubator-weex/pull/780))
* `bugfix` `Android` Fix android new Date() cannot get accuracy time ([#753](https://github.com/apache/incubator-weex/pull/753))
* `bugfix` `H5` Fix scroll event listenning and scrollToElement on chrome for the latest version ([#827](https://github.com/apache/incubator-weex/pull/827)).

## v0.16
-----
* + support 3d rotate ([#532](https://github.com/apache/incubator-weex/pull/532) [#418](https://github.com/apache/incubator-weex/pull/418))
* + new feature support perspective function in transform ([#551](https://github.com/apache/incubator-weex/pull/551)[#532](https://github.com/apache/incubator-weex/pull/532))
* + new feature support save image to photo album ([547](https://github.com/apache/incubator-weex/pull/547) [575](https://github.com/apache/incubator-weex/pull/575) [544](https://github.com/apache/incubator-weex/pull/544))
* + support `image.save` ([#575](https://github.com/apache/incubator-weex/pull/575)).
* + optimize event binding and support mobile firefox, and also fix a lot of other things ([#606](https://github.com/apache/incubator-weex/pull/606)).
* + Support js service in Rax DSL.
* + Partial support of sending `ArrayBuffer` between js and native.
* + Add basic support of `<recycle-list>`, both in Vue and Rax DSL.
* + Support saving image to photo alubm in `image` [#547](https://github.com/apache/incubator-weex/pull/547)
* + Support perspective features [#551](https://github.com/apache/incubator-weex/pull/551)
* + New interface to performance tracing [#586](https://github.com/apache/incubator-weex/pull/586)
* + Add the ability of FlatGUI, it can reduce the view hierarchy in `cell` [#643](https://github.com/apache/incubator-weex/pull/643)
* + Support the `box-shadow` style for Android 4.3 and higher [#685](https://github.com/apache/incubator-weex/pull/685)
* + Support float interval/delay in timer [#699](https://github.com/apache/incubator-weex/pull/699)
* + New `recycle-list` compoent with hight performance and low memory cost [#726](https://github.com/apache/incubator-weex/pull/726)
* * remove dependency about socketRocket dependency in iOS.
* * fix coretext crash in iOS.
* * fix toast view still pop while the page was destroyed in iOS.
* * separate weex-vue-render into two parts: render core and plugins ([#533](https://github.com/apache/incubator-weex/pull/533)).
* * Fix Jni crash due to emoji [#574](https://github.com/apache/incubator-weex/pull/574)
* * Fix the lost refresh header of `list` in viewpager [#601](https://github.com/apache/incubator-weex/pull/601)
* * Fix draw iconfont fail when first download iconfont [#625](https://github.com/apache/incubator-weex/pull/625)
* * Fix the problem of 'text-overflow:clip' [#718](https://github.com/apache/incubator-weex/pull/718)
* * Fix android new Date() cannot get accuracy time [#753](https://github.com/apache/incubator-weex/pull/753)

## v0.15
------
* + support fast click and hairlines border [#507](https://github.com/apache/incubator-weex/pull/507).
* + Add `weex.supports` api for feature detections. [#6053](https://github.com/vuejs/vue/pull/6053)
* + Change default image quality to `WXImageQuality.AUTO` [#478](https://github.com/apache/incubator-weex/pull/478)
* + Support the `scroll` event on horizontal scroller[#494](https://github.com/apache/incubator-weex/pull/494)
* * Fix the console API to adapt JSC on Android. [#470](https://github.com/apache/incubator-weex/pull/470)
* * Fix invalid call scrollToElement when has not option param [#491](https://github.com/apache/incubator-weex/pull/491)
* * Fix the lines of `text` cannot be reset [#493](https://github.com/apache/incubator-weex/pull/493)
* * Fix invalid init index on `slider` [#510](https://github.com/apache/incubator-weex/pull/510)
* * Fix Memory optimization for `list` [#512](https://github.com/apache/incubator-weex/pull/512)

## v0.14
------
* + support `waterfall` component ([#438](https://github.com/apache/incubator-weex/pull/438)).
* + support pseudo-class ([#474](https://github.com/apache/incubator-weex/pull/474)).
* + Support component method in Vue DSL. ([proposal](https://github.com/alibaba/weex/issues/969))
* + Support returning value synchronously for module methods. ([proposal](https://github.com/alibaba/weex/issues/1677))
* + Support drag-drop on `list` [#416](https://github.com/apache/incubator-weex/pull/416)
* + Support rotateX and rotateY, optimize animation as well [#418](https://github.com/apache/incubator-weex/pull/418)
* * Fix wrong vertical offset in scroll event on `waterfall` [#424](https://github.com/apache/incubator-weex/pull/424)
* * Fix `clearTimeout` and `clearInterval` doesn't work when funId is greater than 127 [#439](https://github.com/apache/incubator-weex/pull/439)


## v0.13.0
------
### Notable Changes
* Slider implemention is refactored [Pull Request#414](https://github.com/apache/incubator-weex/pull/414)
* Improve integration test. We are working with macaca team, to write better test code.[Pull Request#411](https://github.com/apache/incubator-weex/pull/411) [Pull Request#397](https://github.com/apache/incubator-weex/pull/397) [Pull Request#402](https://github.com/apache/incubator-weex/pull/402) [Pull Request#413](https://github.com/apache/incubator-weex/pull/413) [Pull Request#390](https://github.com/apache/incubator-weex/pull/390) [Pull Request#346](https://github.com/apache/incubator-weex/pull/346) [Pull Request#319](https://github.com/apache/incubator-weex/pull/319) [Pull Request#304](https://github.com/apache/incubator-weex/pull/304) [Pull Request#295](https://github.com/apache/incubator-weex/pull/295)
* `scroller` now has `pagingEnabled` attribute, which can enable `paging` feature in native [Pull Request#393](https://github.com/apache/incubator-weex/pull/393)
* New 'prerender' mechanism, which will support rendering a page in background. [Pull Request#343](https://github.com/apache/incubator-weex/pull/343) Pull Request#342](https://github.com/apache/incubator-weex/pull/342)
* Fix `line-height` feature in iOS. [Pull Request#377](https://github.com/apache/incubator-weex/pull/377) [Pull Request#305](https://github.com/apache/incubator-weex/pull/305)
* Add `needLayout` option in animation module operation after animation finished [Pull Request#337](https://github.com/apache/incubator-weex/pull/337) [Pull Request#336](https://github.com/apache/incubator-weex/pull/336)
* `list` component has new type of event for `sticky` feature [Pull Request#332](https://github.com/apache/incubator-weex/pull/332)
* Support bota and atob [Pull Request#315](https://github.com/apache/incubator-weex/pull/315)
* Fix mixing background-color and border-color(rgba) in android [Pull Request#359](https://github.com/apache/incubator-weex/pull/359)


Beside these, lots of crashes and bugs are fixed.[Pull Request#441](https://github.com/apache/incubator-weex/pull/441) [Pull Request#413](https://github.com/apache/incubator-weex/pull/413) [Pull Request#403](https://github.com/apache/incubator-weex/pull/403) [Pull Request#373](https://github.com/apache/incubator-weex/pull/373)

## v0.12.0 ( First Offical Release)
---------
- C++ timer  by lycool
Discussed in https://lists.apache.org/thread.html/567c9b19d68ccf3e0d24c1467298ebcd4316ffa524c557a34c6c087f@%3Cdev.weex.apache.org%3E
relate pull requests:[apache/incubator-weex/pull/228|https://github.com/apache/incubator-weex/pull/228], [apache/incubator-weex/pull/232|https://github.com/apache/incubator-weex/pull/232], [apache/incubator-weex/pull/221|https://github.com/apache/incubator-weex/pull/221]

- Add scroller/list scroll event in html5 render
android&iOS already have this feature in v0.11
https://github.com/apache/incubator-weex/commit/f50fba8647c8bb6ac522b1a4569a2a2269da1953

- Enhance accessibility, new `aria-label` & `role` support
[apache/incubator-weex/pull/149|https://github.com/apache/incubator-weex/pull/149]

- Native input/textarea enhancement by kfeagle &  misakuo
support `number` data type; support soft keyboard event 

- Picker module enhancement
More picker options to customize picker dialog style(background color etc.). Related pull requests: [apache/incubator-weex/pull/234|https://github.com/apache/incubator-weex/pull/234], [apache/incubator-weex/pull/233|https://github.com/apache/incubator-weex/pull/233]

- Android DOM module refactor
Seperate module code by action, increasing the maintainability.
[apache/incubator-weex/pull/104|https://github.com/apache/incubator-weex/pull/104]

## v0.10.0
------
- New Feature
  - Support Vue.js  
    The Vue.js 2.1.8 ([runtime-only build](https://vuejs.org/v2/guide/installation.html#Standalone-vs-Runtime-only-Build)) is in WeexSDK now. You can use Vue.js to build native app by WeexSDK 0.10.0.  
    We reused the original native render engine and developed a new renderer ([weex-vue-render](https://www.npmjs.com/package/weex-vue-render)) for the web platform, which is based on Vue 2.0.  
    The former front-end framework (commonly known as `.we`), which is inspired by Vue 1.0, is deprecated. Although it still works well in this release, we suggest to migrate it to Vue 2.0.
  - SDK
    - New CSS support
      - [text `font-weight`](https://weex-project.io/references/text-style.html)  
        `font-weight` can set to [`normal`|`bold`] or 100-900.
      - gradient  
        like CSS3, now you can use gradient in Weex. For example:   
        
        ``` css
        background-image: linear-gradient(to right, blue, white);
        ```
        ![img_1695](https://cloud.githubusercontent.com/assets/115201/23015955/ba075876-f46f-11e6-9d88-2ca3096551b9.jpeg)
        [Read more about gradient](https://weex-project.io/references/common-style.html).
      - Pseudo class  
        Currently, Weex supports 4 pseudo classes:`active`, `focus`, `disabled`, `enabled`. 
    - New BroadcastChannel API
      Developers can use `BroadcastChannel` API to implement inter-instance communication.   
      
      ``` js
      const Stack = new BroadcastChannel('Avengers')
      Stack.onmessage = function (event) {
        console.log(event.data) // in this case, it's "Hulk Smash !!!"
      }
      
      // in another instance
      const Hulk = new BroadcastChannel('Avengers')
      Hulk.postMessage("Hulk Smash !!!")
      ```
    - Image's `onload` event add [`naturalHeight` and `naturalWidthimage`](https://weex-project.io/references/components/image.html) to get the original size of image file.
    - Websocket Support  
      WebSockets is an advanced technology that makes it possible to open an interactive communication session between the user's h5/iOS/android and a server. With this API, you can send messages to a server and receive event-driven responses without having to poll the server for a reply.  
      [Read more about Weex's websocket.](https://weex-project.io/cn/references/modules/websocket.html)
    - Support synchronous method call
      Both module and component method can defined synchronous method exposed to JS runtime now. Means native will invoke these method in JS thread directly.
    - Support `viewport` configuration
      Similar to [W3C specification](https://drafts.csswg.org/css-device-adapt/#viewport-meta), Weex support set define `viewport` in script tag:   
      
      ``` html
      <script type="config">
        {
          "viewport": {
              "width": "device-width"
          }
        }
      </script>
      ```
  - Tools   
    - [Devtools](https://github.com/weexteam/weex-devtool)
      - Support Vue 2.0 debugging.  
      - Add network switch for network inspector.
      - Make application capable to decide which bundle is 'un-debuggable', which means page's source code is unreadable in debug mode.
    - [Weexpack](https://github.com/weexteam/weex-pack)
      - Has full set of commands for developers to setup android/ios application with his .we/.vue files.
      - Developers could easily pack/install his application with simple command.
      - Has full set of commands for developers to manage weex plugins, including create plugin template, add plugin to his project etc.
      - [Plugin market](http://market.weex-project.io) was formally used for developers to publish/download weex plugins.


## v0.9.4
------
- New features
  - SDK
    - New API to get Component's size and position:
      Now you can get these data through `getComponentRect`:   
      ``` javascript
      var dom = require('@weex-module/dom');
      dom.getComponentRect(this.$el('comp_id'), function(data){
        if(data.result)
          console.log(data);
      });
      ```
      The `data` callback parameter contains a `result` to tell if operation is success. And `size` tell you the true data(`bottom`/`top`/`left`/`right`/`width`/`height`) of component.
    - A brand new `picker` module. We have 'single-picker','date-picker' and 'time-picker' currently, and more common pickers are on the way.  
      ![img_1282](https://cloud.githubusercontent.com/assets/115201/21414801/e6341b36-c83d-11e6-9e5a-3acdabb592ee.png)
    There are two ways to use `picker`
    - Use `picker` module directly:
    ``` javascript
    var picker = require('@weex-module/picker');
    var self = this;
    picker.pickDate({
        'value':'2016-11-28',
        'max':'2029-11-28',
        'min':'2015-11-28'
    },function (ret) {
        var result = ret.result;
        if(result == 'success')
        {
            self.value = ret.data;
        }
    });
    ```
    - `input` component also add 'date' and 'time`type to work with`picker` module internally:
    ``` html
    <input
      type="date"
      placeholder="select date"
      class="input"
      autofocus="false"
      value=""
      onchange="onchange"
      max = "2029-11-28"
      min = "2015-11-28"
              ></input>
    ```
  - Support animation with `width` and `height` property.
  - Support use empty value to reset css property to default value.
  - Components can expose methods too, like modules do. Developers use the same way as create module method to achieve that.
  -  Add `blur` and `focus` method to manually control `input` component to lose or get focus.
  -  Support relative URL, which will resolve real URL by bundle's URL.
  -  Core javascript framework's unit test coverage is 100% now. we'll pay more attention to quality.  
  - DevTool
    - Support to check the node hierarchy in [weex-devtool-extension](https://github.com/weexteam/weex-devtool-extension) and highlight the node if it exceeds an specified level.
    - Support different refresh mode in devtools to reload the page or SDK automatically when source file updated.
    - Improve quality in [weex-devtools-android](https://github.com/weexteam/weex-devtools-android) module
      - Remove explicit dependency on okhttp and okhttp3 by reflection and proxy
      - Improve demo application with less and refactored code
      - Fix some crash caused by class up cast
      - Fix reflection crash caused by complier optimization
      - Fix "network on main thread" and stop screencast when disconnect
    - Add [weex-analyzer-android](https://github.com/weexteam/weex-analyzer-android) and [weex-analyzer-ios](https://github.com/weexteam/weex-analyzer-ios) which support the following on device directly:
      - Inspect FPS/CPU/memory
      - Inspect storage
      - Display log information
      - 3D viewer of the weex page
      - Javascript error prompt


## v0.8.0
------

- New Features
  - Add [globalEvent module](https://github.com/alibaba/weex/blob/doc/doc/modules/globalevent.md)
  - Support `width/height` animation in transition
  - Refactor the default js framework code, hide almost all the private APIs #777
  - iOS 10 compatibility
- Performance
  - Support `callAddElement` low-level API to make rendering faster
  - Improve SDK initialization performance, for minimise invoke thread impact.
  - Use native `Set` polyfill to fix iOS7 memory leak
  - Use `setProperty` replace reflection for better performance
  - Add `static` directive in default js framework to avoid unnecessary data-binding and take down the memory use
- Tools
  - Add [weex-pack](https://github.com/weexteam/weex-pack), our next generation of engineering development kits. It allows developers to create weex projects with simple commands and run the project on different development platforms.
  - Add [weex-devtool-extension](https://github.com/weexteam/weex-devtool-extension), a extension for Weex devtool to improve your debug experience，which equivalent an element tag for debugger page.
  - Move devtool to separate [iOS](https://github.com/weexteam/weex-devtool-iOS) and [Android](https://github.com/weexteam/weex_devtools_android) repos.
    - Add "screencast" which enable the screen of the device(or monitor) to appear on the "Inspector" page; 
    - Add "remote control" function, in Android user could control remote device(or monitor) when he moves mouse on screencast; 
    - Add "select element" function which enable the user to find the exact node in "Elements" inspector Tab when he click the mouse on screencast;  
    - Add "vdom inspector", so user can choose to see the details of native dom or vdom in "Elements" Tab at his preference; 
    - Adjust interfaces with weex SDK to support "callAddElement"; 


## v0.7.0
------
- New Features
  - [Timer Module](https://github.com/alibaba/weex/blob/doc/doc/modules/timer.md)
  - [Storage Module](https://github.com/alibaba/weex/blob/dev/doc/modules/storage.md)
  - Unify the `image` component's error page when src is invalid
  - Unify the `border`,`padding`,`background-color` style
  - Horizontal-scroller support  `scrollto`  api
  - Fix the issue that component with  `position:fixed` style can not be closed
  - Module callback support `object` params
  - Slider suppport  `setIndex` api
- Performance
  - Use `callNative` signal to stop JSFM render after instance been destroyed
  - Lazily initialize JSFM When device is in low-memory status, improve SDK stability
- [Tools](http://alibaba.github.io/weex/doc/tools/devtools.html)
  - Support debugging  weex(.we) and  react(.jsx) source 
  - Support apps debugging on the same device 
  - Support "watch" feature
  - Solve the dependency on Debugger, user could start "Inspector" first or "Debugger" at will
  - Add "refresh" function in sdk, user could inspect new file by scanning its QR code in playground;
  - Android/ios inspect module split from weex sdk,  and will deliver in separate repo in future; support inspect in windows system

## v0.6.1
------
- New Features
  1. [iOS has been open sourced](https://github.com/alibaba/weex/tree/dev/ios)
  2. [Lifecycle Page Event](https://github.com/alibaba/weex/blob/v0.6.1/doc/references/common-event.md#page-event): viewappear, viewdisappear
  3. [fetch](https://github.com/alibaba/weex/blob/v0.6.1/doc/modules/stream.md#fetchoptions-callbackprogresscallback)
  4. [line-height](https://github.com/alibaba/weex/blob/v0.6.1/doc/components/text.md#styles)
  5. [list component](https://github.com/alibaba/weex/blob/v0.6.1/doc/components/list.md) 
     - support sticky header 
     - support scrollToElement API
     - support nested horizontal scroller
     - support cell children nodes event: appear/disappear
  6. [Gesture](https://github.com/alibaba/weex/blob/v0.6.1/doc/references/gesture.md): panstart/panmove/panend, swipe, longpress
  7. Improve Android text compatibility
- Performance
  1. iOS, iPhone 5c, rendering frame rate ascends from 45FPS to 52FPS
  2. Android, Redmi Note 1, loading time of the first screen  descends from 602ms to 480ms
  3. Improve Android animation performance
- Tools
  1. [weex-toolkit](https://www.npmjs.com/package/weex-toolkit) supports require and generator
  2. Playground supports runtime performance viewer
  3. [Weex DevTools](https://github.com/alibaba/weex/blob/v0.6.1/doc/tools/devtools.md)
     
     <img src="https://img.alicdn.com/tps/TB1O.nwKFXXXXX8XpXXXXXXXXXX-1436-811.png" width="600">


## v0.5.0
------

### New Features
1. [TabBar](https://github.com/alibaba/weex/blob/dev/doc/components/wxc-tabbar.md) is a specialized component corresponding to the radio-style selection.
2. [NavPage](https://github.com/alibaba/weex/blob/dev/doc/components/wxc-navpage.md) contains a navbar at the top of the window and an embed content page.
3. [Activity Showcase](https://github.com/alibaba/weex/blob/dev/examples/showcase/new-fashion/index.we) is built by composing TabBar and NavPage.
4. [Web](https://github.com/alibaba/weex/blob/dev/doc/components/web.md) displays web content in the weex page.
5. [A](https://github.com/alibaba/weex/blob/dev/doc/components/a.md)  defines a hyperlink to a page in the web.
6. `Text` supports style [text-overflow](https://github.com/alibaba/weex/blob/dev/doc/references/text-style.md#properties).
7. `Image` supports attribute [resize](https://github.com/alibaba/weex/blob/dev/doc/components/image.md#styles).
8. `List` supports [events `appear`, `disappear`, `loadmore`](https://github.com/alibaba/weex/blob/dev/doc/components/list.md#events) and [refresh](https://github.com/alibaba/weex/blob/dev/doc/components/list.md#child-components).
9. New Syntax
   1. [Inline event](https://github.com/alibaba/weex/blob/dev/doc/syntax/events.md#inline-handler) supports a expression of calling event handler in template.
   2. [Require Native Module](https://github.com/alibaba/weex/blob/dev/doc/modules#how-to-use) requires a native module by `require('@weex-module/moduleName')`.
   3. [Computed Property](https://github.com/alibaba/weex/blob/dev/doc/syntax/data-binding.md#computed-properties) supports complicated logic in data bindings.
   4. [New Repeat Syntax](https://github.com/alibaba/weex/blob/dev/doc/syntax/display-logic.md#a-extension-of-repeat-syntax) is easy to access the key or value of repeated object.


------