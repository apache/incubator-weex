---
title: Release Note
type: releasenote
layout: post
---

# Release Note

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
- Bugfix
  - \* [android] add fixed size property 5191241
  - \* [android] bug fix NPE && cache ScreenWidth and ScreenHeight 950e843
  - \* [android] bugfix duplicate destroy 25412e0
  - \* [android] bugfix keyboard force hide (#106) 0ad2f95
  - \* [android] bugfix scroller offset transform twice cd7ad66
  - \* [android] change gradle to fix build 1feaadf
  - \* [android] ClipPath doesn't work when the following conditions met   1. rotation or scale is used   2. API level is 24 As border-radius uses clipPath, this will cause views with border-radius using rotation or scale on API level 24 doesn't. So far, this is a bug of Android N,https://code.google.com/p/android/issues/detail?id=225556&sort=-id&colspec=ID f69e918
  - \* [android] Fix bug of border width due to viewport. Refer https://github.com/sospartan/weex/commit/7cfe9752e0c95f3b23bedfa3f51ba3bafb4318f4#diff-42908d45812780fec8606537dffab67dL1054 9cd6800
  - \* [android] fix cast exception 3136fcf
  - \* [android] fix DCL thread safely (#48) 62e9b71
  - \* [android] fix duplicate add component child to list 3fd73d7
  - \* [android] fix find component 45643ab
  - \* [android] fix getScreenWidth with context parameter (#80) 16377f2
  - \* [android] fix inefficient use of keySet iterator instead of entrySet iterator (#47) dac80e2
  - \* [android] fix intrinsicSize when img.resize=contain b6272a2
  - \* [android] fix npe in onScrollerListener (#138) 8141de7
  - \* [android] fix npe in stream callback 24aa2c2
  - \* [android] fix PageActivity duplicate add view to container 06f77b8
  - \* [android] fix refresh crash after destoryed (#144) 51ea00b
  - \* [android] fix refresh/loading a726d3c
  - \* [android] fix slider auto scroll behavior 28926ba
  - \* [android] fix slider layout, ignore position offset. a0381e4
  - \* [android] fix some weak code that may cause file leak or useless (#49) 8d0f049
  - \* [android] fix stackoverflow 6a01059
  - \* [android] fix stream response status type (#63) 8580c66
  - \* [android] fix switch case (#127) c301b92
  - \* [android] fix textarea vertical gravity 9730948
  - \* [android] fix the IllegalArgumentException on empty arguments f799d67
  - \* [android] fix the wrong header location (#135) d58fce9
  - \* [android] fix typo: WebSocket -> webSocket c162d54
  - \* [android] fix wrong initial direction value on appear event (#107) 593c799
  - \* [android] modify default real position (+1 squashed commit) Squashed commits: [9470835] \* [android] fix unittest b915584
  - \* [android] move indicator layout logic to subclass;seperate fixed layout logic fc2a009
  - \* [android] rename debug activity 95ab73a
  - \* [android] update fix do not remove loading and fresh ddf74ff
  - \* [html5] fix border & border-width for less than 1px. (#2059) 3eb34b9
  - \* [html5] fix bugs of neighbor 34fafb2
  - \* [html5] fix compatibility problem & support deps arguments. 00e7079
  - \* [html5] fix content-type error 5b06140
  - \* [html5] fix h5 build 56e9c03
  - \* [html5] fix shrink & grow for flex. (#2082) b5b600d
  - \* [html5] fix slider container's style. (#2157) a79bd9c
  - \* [html5] fix uncaught dom exception for video.play. (#1877) 7a0bd03
  - \* [html5] fixbug:change name from WebSocket to webSocket] (#2146) b72bbb9
  - \* [ios]  bug fix: websocket module name is changed , playground should change 3f35b82
  - \* [ios] bug fix: onload success value type is not same as android, change it for unify 531633c
  - \* [ios] bugfix call renderFinish 1fb2e0a
  - \* [ios] bugfix module default method not called 0be9f87
  - \* [ios] bugfix moduleEvent compatible dbea448
  - \* [ios] bugfix multi input or textarea maybe wrongly change root view frame a2e97db
  - \* [ios] bugfix navigation pop animated value convert cfe98a8
  - \* [ios] bugfix navigator open and close. beef00b
  - \* [ios] bugfix rootView information  coordinate 7313703
  - \* [ios] bugfix slider-neighbor may beyond item count index cbfa9df
  - \* [ios] bugfix view event 83d3e6a
  - \* [ios] bugfix: playground cannot open c949463
  - \* [ios] bugfix:delete configpicker not delete the method in header cc3afc2
  - \* [ios] fix : Change the lastoffsetXRatio to lastOffsetXRatio de8f246
  - \* [ios] fix : Change the parameter wording to make it more in line with development habits. 869da54
  - \* [ios] fix : if _fontFamily is nil set _fontFamily value with the correct system font. In the case of Chinese characters, this logic help system select the right system default font name. 6e202b6
  - \* [ios] fix : improve slider-neighbor demo code .remove useless <container> label and <scroller> label. 2c236c8
  - \* [ios] fix bug : you can't slide to the right when the slider has only two pages. d224bfb
  - \* [ios] fix bug that border-x-radius not work 96ba5b5
  - \* [ios] fix bug that horizontalpan and verticalpan can not trigger simultaneously eb03cb0
  - \* [ios] fix bug: text input padding is not right f4accba
  - \* [ios] fix crash after returning id type, need to copy here. 4f5276b
  - \* [ios] fix issue that cells pushed down by inserted cells will not trigger disappear event. 9dcc4dd
  - \* [ios] fix issue that rotate 90->180->270 not work. 5b3c13d
  - \* [ios] fix issue that rotate(0) not work after rotate(>180degree) 0f6989d
  - \* [ios] fix issue that there will be 1px space remaining between list's top and navigator when UITableView is pulled down and finger releases. d286e3c
  - \* [ios] fix typo in [WXSDKEngine initSDKEnvironment] 17a093f
  - \* [ios] fix variable name spelling error becd631
  - \* [ios] Fixed :Merge remote-tracking branch 'alibaba/ios-feature-20170118' into ios-feature-20170118-new-slider-neighbor-fix    move "sliderview init" code to loadview method. 141fd9d
  - \* [ios] fixed component's view should be removed separately since its view is a subview of rootview. f9b2378
  - \* [ios] fixed issues that horizontal/vertical pan not respond while recyling views. c7b4bec
  - \* [ios] js service bug fixed c068bd7
  - \* [ios] removed debug code and fixed bug 548984f
  - \* [ios] update navigator module, add open and close interface. f65c42f
  - \* [ios] upgrade JSFM to 0.19.3 to fix callback problem 9ab9b92
  - \* [jsfm] fix mismatch of weex.require and weex.document ad63f2a
  - \* [jsfm] fix support native method bug ce26a6f
  - \* [jsfm] fixed Promies bug in android 17282c2

### Downloads
- [Source code (zip)](https://github.com/alibaba/weex/archive/v0.10.0.zip)
- [Source code (tar.gz)](https://github.com/alibaba/weex/archive/v0.10.0.tar.gz)
- [Cocoapods WeexSDK](https://cocoapods.org/pods/WeexSDK)
- [Bintray weex_sdk](https://bintray.com/alibabaweex/maven/weex_sdk)


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
- Bugfix
- \* [android] bug fix slider (#56) 29b9d29f5acd9677a64316b94d5c95f8c7f1a837
- \* [android] bugfix duplicate destroy 0a59a85467671ddb982fbea6def810e6fa415137
- \* [android] bugfix scroller offset transform twice 1db300d470951f543236e0b605d7b5a974b96430
- \* [android] fix appear event not trigger at first 85085a394dd2bae5e397b889c01c1e1a3b8e6b46
- \* [android] fix build 46bcf4298cd74c1d9706255143f95e0299ebdd15
- \* [android] fix callback parameter 16db52ed031f7182f2159bc18c0c6e20e7c907f7
- \* [android] fix dialog window leaked (#55) 4c478044355d6d552fb2f72935cd93867e5f0802
- \* [android] fix different behavior with iOS switch (#43) 7708a6680e51ae6af9351724940778ce953538ed
- \* [android] fix dynamic inserted header not sticky display 83c06383aa513061d144ec448ce0ccc25bde5484
- \* [android] fix fireevent change key 9fb7d0f25d6b1bfe54fb63c9b73c53f8c927e4ad
- \* [android] fix flash when render video tag a53e6b80b8fa67cce85307a2e5e93d9fb6c28784
- \* [android] fix list not lazy 0be12be1194bea0c624791662e9f38aac4f0c3fd
- \* [android] fix methods generate external-loader-component ec6d4136a29c1626470d3bbba90db3d9916c374e
- \* [android] fix move child from index to index b within the same parent 72259e7a5734ed9a595de080d05ea8e1bf1e716e
- \* [android] fix slider auto scroll a8d74031dbd526201c05433a08c8aac2047b7a66
- \* [android] fix slider auto scroll after activity resume bba3bdea45bfb4030b10ea9a847a77463d1ab20a
- \* [android] fix slider auto scroll after activity resume e0d5e79fc29e07b734d892073effc041a39c85c6
- \* [android] fix stackoverflow in slider c1f1d702110f49ead4005fae50e4e63c5e72cfac
- \* [android] fix sticky not display when scrolltoelement 22d163cb572e6b1c510c350a692a4265eb27e204
- \* [android] fix test d756db8b4c26f8610076a13a0980d024a4e734ac
- \* [android] fix textarea content vertical gravity eecc32c0a72ce82aa4e998cabb31308fc737fe95
- \* [android] fix the invalid index of slider-neighbor (#62) 59fd5742a23908174de12804b1a0992d89663afd
- \* [android] fix unittest 0b2a3778dfa39f17ae6346168b55a3418d85aa1b
- \* [android] fix unittest 0d2bd982507820f5840e99ee12cd351ae6eb18e5
- \* [android] fix unittest 4f120b6dbcb4433c8c320c310cd4de051163780c
- \* [android] fix unittest 6129cd2ed06ce7acc0366b4c172bdf199d52e997
- \* [android] fix wrong behavior with slider.index (cherry picked from commit c59ae39) 3028f32278a47603d1dc9f0f4314b44721069ac4
- \* [android] fix wrong behavior with slider.index c59ae3977837be238b08b69388aec12eb4817716
- \* [android] move element  bug fix dfc643df7fb100737c98efa7f5932e75d07ca59f
- \* [android] move element  bug fix f28fbdd3ac57c157f52d82fa9473e8651b88023b
- \* [android] NPE fix ee67a26c045cc351d317e805712bd6e50c4213cb
- \* [android] Parent's "append=tree" will prevent child's transform style, this commit fix it by add the whole dom tree to animation set instead of the root dom. \* 626915b34bfb348947709c23e09a3dac7acd6962
- \* [android] slider: fix the IllegalStateException (#58) 0e94762e1ebc240a5e8be6c131848b57bb48b1ea
- \* [android] v8 security bug fix 7a8f14b78753fd02d59b126fffbd52996376ecc3
- \* [html5] fix the conflict ac845591a65501c4eae70471502d8370f9983130
- \* [html5] fix wwp config 0a2420a7202887a0222c0294c510267f139858c8
- \* [html5] fixed lazyLoad filename 4119190c6b093f87bcb8ebd00f23bd939c307ad1
- \* [html5] fixed rollup build 03f241ce3c7d8ac786b632f80aa22b94fdc00d34
- \* [ios] 1. fix refresh location 2. cancel playground re-checkupdate 69f43da52ab22c7777d54bc2d9f45e95eb1d437f
- \* [ios] bug fix 1664, add resetLoadmore on scroller & list. 4efe8662912ec5a8ce5fac8e09c2e061a21056a4
- \* [ios] bug fix picker 2d979a58c4b37e93dda5b775d79a7a1ea2165c22
- \* [ios] bug fix picker 983fe0069ff670b92b6fa7e3e6e29c5b2c768762
- \* [ios] bug fix picker rescue 27fe3f23bb63072b766f75b69351464d163b331a
- \* [ios] bug fix setNeedsLayout in every elements. not in once. because elements not contain css node only edd47e09ff453a26c7b176279d0ab84454d4914a
- \* [ios] bug fix update code style 05b2959be0e0c35ad31b5b20b77f86f296786da2
- \* [ios] bug fix: change resetTextCSSNodes to resetTextCSSNode a782e092b175a0a34db7cfccb74b5f157ddb3fd4
- \* [ios] bug fix: refresh view layout error. b0c4a403b1e1dc8aa500c60efcef263b6f7fdd6f
- \* [ios] bug fix:slider does not remove items  synchronously when item view has been remove from superview. f60997fc3fe7bcfd220e159d61dbaae438e59131
- \* [ios] bugfix add register component type f7e72ea289ce5f909fb9f6b61705e4f4b9930c53
- \* [ios] bugfix cancel url uft8-encode 6b67a9e8a3f62849709d552bea0d947679164041
- \* [ios] bugfix cancel url uft8-encode 9a43cfc9ecb9683562560a880d0f026fd28e93ea
- \* [ios] bugfix component load rewriteurl f92a6d70e0f3db1eb14c4d90c7e30609e5175dff
- \* [ios] bugfix globalEvent removeObserver don't need specify target a5420939bac4e21ef61b15710e2fa5e205403d96
- \* [ios] bugfix iconfont load font , support woff 4c5d5812cb145d60270c5e59ae29def9be8fdcf3
- \* [ios] cancel animation while handle input keyboard and fix bug when there are many input in a page 61f9259bf77c59916d3dbcdf62b12cd0161698b4
- \* [ios] debug performance ae3d30802681990e79ce06fb4229b74f3c1d99c2
- \* [ios] fix #1192, #FFF display error in iOS 5df5cd3739e79ec6e2fefbff66ba7ba2a411d637
- \* [ios] fix #1479, add handler for slider's items count less than or equal to 2 d654ed7d926c2e20b3a34315348a87470171cdd1
- \* [ios] fix : Compatible non-standard url ("//a.b.example.png") 3da33595d6e1a261b1c761811d1d8f7672ba3db5
- \* [ios] fix border's "Overlapping Curves" not recomputed when relayout d302adf14568a32d2b635a64d1326172d9590893
- \* [ios] fix bug datepicker no update and crash eef4483b3176a0f2e18673e26068a6ec53fef5ca
- \* [ios] fix bug debugTool unload 751b752f72690954c49ffaf8b89702a0f93712f2
- \* [ios] fix bug default style f8f056e9f067b4ce23731cb3114383705a5926ac
- \* [ios] fix bug height change to zero when style change 1ed428dda5d1b4e2b360f216b1b96433479dad28
- \* [ios] fix bug height change to zero when style change a4d7ebea5e288b753ee71571b149130149768c6b
- \* [ios] fix bug height change to zero when style change aac7c013003c055e840674e5f61dba4d5073dd73
- \* [ios] fix bug height change to zero when style change bb8cdb1f2f7ff6ad9b8ac4de23323ca9dea7e0d9
- \* [ios] fix bug that setFrame incorrectly when transform has been set 84dfd4c7395ae728356b660a0b605c7b85353973
- \* [ios] fix bug that sticky component's absolutePosition will be computed twice if the actual absolute position is {0,0} a780352d0a1a139ce3e276404d03bd3f9e2e6fd6
- \* [ios] fix bug: text area cursor auto go to end .reason: if attributes[@"value"] is exist. _updateAttributesOnMainThread will change textview text.the cursor go to end. \* solution: save the  textview  selectedRange , when set textview.text ,correct it d22836ef559809f566068bfe47be960a6d2e9faa
- \* [ios] fix bug:_updateAttributesOnComponentThread reset styles delete 7c993d1952551ccfad2ad22f01ffa40ea9d12245
- \* [ios] fix bundlejs 616f4653925c801406ca5fc80fd5f890c45c2b19
- \* [ios] fix code style 31b08adf525559d9ce46f25eab349584d2f621a9
- \* [ios] fix compiler complain about warning b92e471c1195f40f8167c8253ea0a909930a9880
- \* [ios] fix componentRect infomation da08628c562dded860fe1558fb9dc187530223c9
- \* [ios] fix componentSize to get 41af993f1ffd4e041083a0150607040b9e7cdef2
- \* [ios] fix crash 55f69b502f0ff1f997dbfbe5238e35a4f123bc5b
- \* [ios] fix crash on events' copy 491df3dca1fd47c1c564823c7f46527bf064c2dc
- \* [ios] fix datepicker bug when no value can not pop right picker 24441ad6273e6093a9919b2699a4333b9d7147df
- \* [ios] fix datepicker complie error 3bf2a0753cf47170144939d42173a668b22e27f5
- \* [ios] fix datepicker complie error a362ef85918551bacac762ee660300e3108c1c5a
- \* [ios] fix deprecated method of textarea 4d24f3ff7e792a1c97aa7ee31fbc7f0899c21efb
- \* [ios] fix failed tests. b5210b244f2f2331be01c9afc4a8cde5b32ed0ee
- \* [ios] fix input bind each other feb1e7c1ce959491fefed808e46e6291203ccbd7
- \* [ios] fix ios7 crash f02eff33acab4a58a1a8d6e4d84cef44dcf09bf4
- \* [ios] fix issue about WXConvert 's UIColor method add193b643c980e310866498dc081130ee287373
- \* [ios] fix issue that iOS 7 will crash while alert still alive but instance has been destroyed 160a2dae54d6e656c980b9d008b15b0dc28aa4e1
- \* [ios] fix issue that onappear/ondisappreat not work if binding to cell component. 63f6d7507731d2004e996f99bf859c228fc8b651
- \* [ios] fix multi keyboard notification 684d3955de263cb39f8afc42c6ffa7340674c81a
- \* [ios] fix navigator parameter b3117a9c482fb5491b5206413fa7607c356cf95a
- \* [ios] fix override bundleUrl d7729716aea6652d325e4282ad6b42aaaf14f82c
- \* [ios] fix playground Podfile   remove c header file  include in WXUtility.h e299feaf4076a5bf7140335ac6cc70d9d99a01da
- \* [ios] fix responderToSelector target b0f550d6b94da6d9764863b328edfba6d730a75b
- \* [ios] fix rootComponent order 999c59da3a1dc6c91f036b938c9963cc3cfe87c4
- \* [ios] fix set navigator callback msg a30b7ed4978de3c91448a1c944431f19e942b515
- \* [ios] fix single picker but 969d4212967c084a3477e3877052af224354f0e4
- \* [ios] fix stream jsonp bug b018343b71eb7289bde326acf2f2fe1b124cfad8
- \* [ios] fix testComponentFactory 99574bf0a1072dbfba547d74545783891b15cdff
- \* [ios] fix tests compile error 461c24401e5fa1c37f9af25db35665f9f6b4a8da
- \* [ios] fix text line-height and text-align conflict b1a0673020b7a271fb540ae723ca481aa18db7b4
- \* [ios] fix textinput component inputEvent fired repetitiously 5c8242776f1410771a357735d6ae2a5b261b7d20
- \* [ios] fix warning b8464248f61ad3ed9570730608e563f9c73e8ae6
- \* [ios] fix warnning e2533d036daad60345284f276949dec06b5e0db9
- \* [ios] fix when options is nil and then  bundleUrl will be nil 5f71a1a2eb7442cf63d9732a6a2b3e2cb3765fa1
- \* [ios] fix WXSDKInstance rootview bug 3b08a516ff788ca116329dc863ffcb9f66979881
- \* [ios] texture bug fix : when input change . the text content send to change a38f1b2b8e05e149f09ffbb986268339a9042691
- \* [jsfm] fix #1447, fix binding event bug of components which have repeat attribute 421cc439d1b8b5abbdbba58882f6c39fd8f862c9
- \* [jsfm] fix build 8162124f72caf9360657ebc5a2750554d36f3659
- \* [jsfm] fix condition judgment bug in insertBefore & insertAfter 9c182af615e0e597e21a44217a511126333499e9
- \* [jsfm] fix eslint c0db1807d080a26c7344f4cc57e5834e98b06085
- \* [jsfm] fix eslint f75cbe955dc0bcdf33a608af82ab18d3290b87c3
- \* [jsfm] fix format for lint 5faa860804c1d9adb719a450b53da165ec523e83
- \* [jsfm] fix the npm run build d3e5124add2dd6a42b1209fe11a8dc58cd536487
- \* [jsfm] fixed #1818 a757deb7235792c2dd11684500651f717f43226a
- \* [jsfm] fixed all cases for default framework (black-box) c842ef884e45c568f9ae55e67a066a5de2de15ed
- \* [jsfm] fixed all test scripts 95ba6a1d6c4211d62d7d2a6693c8aa1e5b7d9607
- \* [jsfm] fixed all unit test for new file structure d4eb5c97daa34dc53dd929eaf736dcf22e8873b6
- \* [jsfm] fixed browser render file structure ba7a40bca9a5759101e821c4e043e437b848dcde
- \* [jsfm] fixed deps error 555ded73f6c6ecb5a9c110f34ab34e596c1c36eb
- \* [jsfm] fixed html5 render for Vue 980d952d71903545a0bfa7115dfeacd8c7c71ae5
- \* [jsfm] fixed lint and added case test a3e20a8c78feda705b853f2036cc17bd256f7bb8
- \* [jsfm] fixed rollup config 4a1c208cf33fb3e5649f8116133f97b003e78127
- \* [jsfm] fixed runtime instance map management b6e94fa1d54bfdc541ae7c48d98f4dc01481660f
- \* [jsfm] fixed some vue examples ca2a9c57365a67c9215f69c438c86440a28704ce
- \* [jsfm] fixed some vue examples da21405b7fab3228bda0e0b2a74853a05c5eb69a
- \* [jsfm] fixed url checker in vue example 679dc1cffbd0413b9b81624ac46f61473cdf756d
- \* [jsfm] removed timer bug case and replaced to slot bug case b6f2c8390e3a97df948fc8d2699d15eb62582625

### Downloads
- [Source code (zip)](https://github.com/alibaba/weex/archive/v0.9.4.zip)
- [Source code (tar.gz)](https://github.com/alibaba/weex/archive/v0.9.4.tar.gz)
- [Cocoapods WeexSDK](https://cocoapods.org/pods/WeexSDK)
- [Bintray weex_sdk](https://bintray.com/alibabaweex/maven/weex_sdk)

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
- Bugfix
  - [jsfm] fix the bug of Dep.target 3448104
  - [jsfm] fix the test of clearCommonModules 4be9291
  - [jsfm] fix bug for clear common modules (pr #1097) d2f8c4d
  - [jsfm] fix callAddElement callback -1 e421b41
  - [jsfm] fix #1237, fix binding event bug of components which have repeat attribute bfed029
  - [jsfm] fix bug af99cc1
  - [jsfm] fixed & closed #1148 4209850
  - [jsfm] fix merge classStyle fail between nested components 91c5cc6
  - [jsfm] fix watcher dismatched bug between multi instance 900305d
  - [jsfm] fixed _Set bug 51b219b
  - [jsfm] fixed memory management bug in dom event handler in document 9024bda
  - [jsfm] fixed Promise polyfill problem in iOS 7-8 9a61f33
  - [android] fix #1380, update method getOffsetAmount in WXCircleViewPager.java (#1382) fd74539
  - [android] bug fix native crash d8671bd
  - [android] fix editComponent set non-string value 2e2f032
  - [android] destroyInstance bugfix 6628460
  - [android] update fix global event Memory leak fc6d403
  - [android] add callAddElement debug log 95e3680
  - [android] bug fix classcast exception e399306
  - [android] fix circle ci 51d3294
  - [android] fix slider start auto scroll after activity resume. 89bae89
  - [android] fix log util implementation 10d508f
  - [android] fix x86 so file size verification 21d5135
  - [android] fix build b1fc231
  - [android] fix log utility 530fe51
  - [android] NPE bugfix 6d4385f
  - [android] add cuurentRef to avoid reused bug d836725
  - [android] bug fix update the ref when cell reused 1f7bf86
  - [android] bug fix update the ref when cell reused cef277f
  - [android] fix usertrack modify 8732d70
  - [android] fix code format 936ebd5
  - [android] bug fix- Loading layoutY ba0d34e
  - [android] bug fix - Loading layoutY a8f3c95
  - [android] bug fix - loading margin 7a3f1e0
  - [android] fix Deprecated 6aa7da9
  - [android] fix #1102 (#1103) 5aea5c6
  - [android] fix merge problem 7923b1e
  - [android] fix typo 1de6800
  - [android] fix scroller&list appearance event da1ee4d
  - [android] fix invaild add dom operation 14b5943
  - [android] fix build 0d43794
  - [android] fix build 0520743
  - [android] fix build 9d596c1
  - [android] fix list appear&disappear 425c3e1
  - [android] fix list appear&disappear event 40b52f3
  - [android] update so for fixing instanceId error 03d0973
  - [android] fix v8 security problem 06dbc38
  - [android] bugFix:  List component scrollTo inaccurate 45f0476
  - [android] fixed #1058 f25238c
  - [android] fixed #1058 33b4aaf
  - [android] fix v8 security problem e0b881f
  - [android] bugFix:  List component scrollTo inaccurate 052aeac
  - [android] fix comment fd3e0ef
  - [android] fix indicator default style 57b71fb
  - [android] fix unit test ee74b5c
  - [android] fix code style 449bad2
  - [android] fix list  orientation usage, move component's
  - [android] fix change/focus confliction b672101
  - [android] fix change/focus/blur  event confliction c3689e8
  - [android] fix add dom error a993a84
  - [android] update the mWXBridge to null when debugmode disable 35005ea
  - [android] fix invaild add dom operation 7b1ad97
  - [android] fix moveElement on the same parent dom case 2a94ba2
  - [android] fix typo 42a2099
  - [android] fix NPE c2da99e
  - [ios] fix style line-height conflict with text-align e762637
  - [ios] fix slider example href 8196792
  - [ios] fix keyboard cover input sometime 0ad56dc
  - [ios] fix #1188, support for converting string to NSUInteger. c3f6ea2
  - [ios] fix issue that fixed-position component's view be reallocated while it is in a cell. d8881c3
  - [ios] indexDict must be accessed on component thread, fix componentForRef crash 8cc3503
  - [ios] fix issue that transform-origin not works in rotation transform 5dcab9e
  - [ios] fix warning f913dab
  - [ios] fix css node children count invalid while change from or change to fixed position. 81a1dc4
  - [ios] fix compile error 98a8078
  - [ios] fix css node children count invalid while change from or change to fixed position. 0b3a2e3
  - [ios] fix podspec syntax error d0386e6
  - [ios] ios7 bugfix api 6e7cf07
  - [ios] try to fix NSMapTable crash in iOS8 a03a4bc
  - [ios] fix slider-neighbor bc03620
  - [ios] fix remove Observer crash 46fcbc5
  - [ios] fix issue that rotate transform not work in styles. 8601304
  - [ios] fix layer setBackgroundColor crash 5778422
  - [ios] fix sliderNeighbor 39cb4ea
  - [ios] fix warning ea8c8f8
  - [ios] fix warning 4d2528e
  - [ios] fix warning issue d77fd5d
  - [ios] fix issue transform does not work while initialized. c36f1bf
  - [ios] canvas component property error fix 448d4f4
  - [ios] try to fix textStorage thread-safe issue 0d62716
  - [ios] fix A component onclick event 01f9706
  - [ios] fix input disabled attribute bug 533df4f
  - [ios] fix input disabled attribute bug fd2c526
  - [ios] fix iconfont src update bug f99906b
  - [ios] customize debug switch 86346ea
  - [ios] fix typo ae99b92
  - [ios] change context name for remove JSContext debugging 75c1e42
  - [ios] fix input max length bug fe9a013
  - [ios]add LogLevel control and RemoteDebug control 164abf8
  - [ios] fix memory leak 00009c0
  - [ios] fix a bug that re-deleting procedure 717bf3f
  - [ios] fix storage module in swift f13b72f
  - [ios] fix highlight problem 93eb282
  - [html5] fix conflicts with jsfm 0.16 ce63e22
  - [html5] fix lint 9612daa
  - [html5] fix class name b224685
  - [html5] fix img lazyload 3984111
  - [html5] fix registration of tabheader d38a4c0
  - [html5] fix bug of null rules of stylesheet in spinner c058f95
  - [html5] fix top 41c603a
  - [html5] fix toast a3a4730
  - [html5] fix lazyload typo 493bee7
  - [html5] fix lint 43b24da
  - [html5] fix loadmoreoffset 3c7e262
  - [html5] fix sticky 9bdf079
  - [html5] fix lazyload 38ee957
  - [html5] fix lint 0e94380
  - [html5] fix flexible a06eb80
  - [html5] fix jsonp callback error 6db0850
  - [html5] fix typo 5f61a06
  - [html5] fix lint 363decd
  - [html5] fix bug of scroll to c007c86
    orientation field to scroller 3febd83
  - [html5] fix dist bug 9d102a5
  - [html5] fix missing viewport meta. 1cf98e5

### Downloads
- [Source code (zip)](https://github.com/alibaba/weex/archive/v0.8.0.zip)
- [Source code (tar.gz)](https://github.com/alibaba/weex/archive/v0.8.0.tar.gz)
- [Cocoapods WeexSDK](https://cocoapods.org/pods/WeexSDK)
- [Bintray weex_sdk](https://bintray.com/alibabaweex/maven/weex_sdk)

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
- Bugfix
  - [android] fix 'fixed' position not work when append tree 6ccfa4f
  - [android] fix slider set index frzee 6524849
  - [android] bug fix - setLayout LayoutParams b70f6e2
  - [android] fix get header:compitable with lower case header key 5ebec75
  - [android] fix storage module: keep open when module is live c311f15
  - [android] fix build 0389302
  - [android] fix jsonp response process 28588c6
  - [android] fix ViewHolder NPE f9005ab
  - [android] bug fix ViewHolder NPE 86dd101
  - [android] fix build b14dd26
  - [android] fix cell memory leak c03fdc1
  - [html5] fix e2e test ci 77f14af
  - [ios] fix type of callback params 2aa8f47
  - [android] fix sdk init time measurement 3d7cd02
  - [android] fix sdk init time measurement f4cbfd0
  - [android] fix sdk init time measurement 8cffa5a
  - [ios] fix stream fetch response type 9a7c296
  - [android] forbid except cell、header、fixed、refresh and loading 114860f
  - [android] fix comment cf53fd3
  - [android] fix build 82a03fd
  - [android] WXUtils bugfix 3655d9d
  - [android] bug fix - setProperty class cast d567f89
  - [android] fix argument format crash 1baac57
  - [android] function name fix : hasAppearAndDisappearEvent 1f8b501
  - [android] fix NPE crash in textarea nad modalModule c3a08ff
  - [ios] fix bug when url has chinese character. b7e2c2f
  - [android] update so fix not delRef argument b0db514
  - [jsfm] fixed repeat bug when update values with new object references 371e58c
  - [android] fix slider initialization crash. 4c52d33
  - [android] add a debugger switch, decouple the dept between inspector and debugger dfa71c4
  - [android] fix bug sticky not show dc3ca4d
  - [android] fix component register type missed 92a92dc
  - [android] sticky null child bug fix 541b68b
  - [android] fix list scroller default height fc0aa58
  - [android] fix indicator default style 610624c
  - [android] fix embed page appear event 71a5360
  - [android] update debug manifest 0541126
  - [android] update JSF to 0.14.6 fix bind data error 4f7e3c8
  - [android] fix bug mChild NPE a313b81
  - [android] fix sticky offsetX 1bae906
  - [jsfm] fixed updateMark error in compileType 9533431
  - [jsfm] fixed updateMark error in compileType 5ea0132
  - [android] Sticky fast fling bug fix 7092c6a
  - [android] fix view appear event 4bf67e4
  - [android] fix list scroll to 6355fc6
  - [ios] fix default debug value 79daf2f
  - [android] fix list scroll to ad0201b
  - [ios] fix stat 2d3ab94
  - [android] refresh the instance when reload called by the debug server 99392db
  - [android] add a stop method to IWXDebugProxy b06d352
  - [android] sticky bug fix f9d3265
  - [android] fix list scrollTo a198b2e
  - [android] fix scroller getView and getInnerView 4fa0453
  - [android] refresh the instance when reload called by the debug server fd2be06
  - [android] add a stop method to IWXDebugProxy 5789700
  - [android] refresh the instance when reload called by the debug server 5ddf782
  - [android] add a stop method to IWXDebugProxy 79cd97b
  - [ios] fix modal bug about message is number d0cdaf9
  - [html5] fix fire event 978c454
  - [android] bug fix event lost f9c4357
  - [android] add debug log cae86fa
  - [html5] fix typo b84a1b3
  - [html5] fix lint 8e905dc
  - [android] fix bug - setLayout 0b5082f
  - [jsfm] fixed native param format of setTimeout in app.differ a9f101e
  - [ios] cancel AppDelegate.m debug annotation 03abcf8
  - [android] fix bug - scroller height error 7458441
  - [ios] support weex-devtool and weex --debugger 714c924
  - [ios] resume original logger debug feature a144699
  - [ios] modify the debug switch, update WXDebugTool.h/m file 1ce1277
  - [android] fix input restore init value when batch 3a54da2
  - [android] fix build 8450473
  - [android] fix switch fireevent with domchange b6c2d31
  - [android]  fix input change event 58563b5
  - [jsfm] fixed repeat binding da9f40d
  - [html5] fix typo 09bcdb3
  - [ios] bug fix: slider is embed in list cell d56c7ec
  - [ios] fix typo for wrong quote in Podfile ab8a83b
  - [html5] fix loadmore offset of list 69ed65a
  - [android] fix model callback data from string to json 3aed566
  - [android] use diff vars flag the remote debug 5e976fc
  - [html5] fix typo b8eec69
  - [android] fix bug - refresh not compatible MI2 c814cd9
  - [android] fix problem that session is null and init framework failed b985d59
  - [ios] fix element cssstyle selected bug 41450a9
  - [ios] fix crash from cssstyle e079e5c
  - [android] update the debug init sequence to make sure the handshake well 28741da
  - [android] fix example file name fc7862c
  - [android] update debug switch f9369bb
  - [android] reuse image adapter for avoid Fresco bug 62dc67c
  - [android] add debug proxy, adapt to the weex-debugger d93f9e5
  - [android] fix NPE in stream f5f8a4d
  - [android] fix getScreenWidth npe e264f2a
  - [android] fix register order 5c8a264
  - [ios] Stupid scoll view, always reset content offset to zero after insert cells, fixed it by hooking 981dcd6
  - [ios] fix data type d815e2a
  - [ios] update debug tool 3d82054
  - [ios] fix input component width calculation ba9a3b7
  - [ios] fix notes about WXComponent+ViewManagement d31e8b6
  - [android] fix list component addsubview error 69c3286
  - [ios] fix inverted-logic 773bea7
  - [android] fix crash when list append tree 686e450
  - [ios] fix playground scan qr code bug 28f7de2
  - [android] fix bug - Scroll getView 647a927
  - [android] fix bug - delete </merge> 77680f1
  - [android] fix bug - nested scroll ac50cd2
  - [android] fix onTouchEvent bug 911e569
  - [ios] fix thread safe method indexOfObject in array 1623165
  - [ios] remove the timing of network debug 5b00fe5
  - [ios] fix warning e1d496c

### Downloads
- [Source code (zip)](https://github.com/alibaba/weex/archive/v0.7.0.zip)
- [Source code (tar.gz)](https://github.com/alibaba/weex/archive/v0.7.0.tar.gz)
- [Cocoapods WeexSDK](https://cocoapods.org/pods/WeexSDK)
- [Bintray weex_sdk](https://bintray.com/alibabaweex/maven/weex_sdk)

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

Bugfix
- [android] fix view appear event 4bf67e4
- [android] fix list scroll to 6355fc6
- [ios] fix default debug value 79daf2f
- [android] fix list scroll to ad0201b
- [android] fix build 5d1286c
- [android] bug fix 6a1788e
- [ios] fix stat 2d3ab94
- [android] bug fix 60073af
- [android] refresh the instance when reload called by the debug server 99392db
- [android] add a stop method to IWXDebugProxy b06d352
- [android] sticky bug fix f9d3265
- [android] fix build 62ee68a
- [android] fix list scrollTo a198b2e
- [android] fix scroller getView and getInnerView 4fa0453
- [android] refresh the instance when reload called by the debug server fd2be06
- [android] add a stop method to IWXDebugProxy 5789700
- [android] bug fix 7a83ddc
- [android] refresh the instance when reload called by the debug server 5ddf782
- [android] add a stop method to IWXDebugProxy 79cd97b
- [html5] fix fire event 978c454
- [android] bug fix event lost f9c4357
- [android] add debug log cae86fa
- [html5] fix typo b84a1b3
- [html5] fix lint 8e905dc
- [android] fix bug - setLayout 0b5082f
- [jsfm] fixed native param format of setTimeout in app.differ a9f101e
- [ios] cancel AppDelegate.m debug annotation 03abcf8
- [android] fix bug - scroller height error 7458441
- [ios] support weex-devtool and weex --debugger 714c924
- [ios] resume original logger debug feature a144699
- [ios] modify the debug switch, update WXDebugTool.h/m file 1ce1277
- [android] fix input restore init value when batch 3a54da2
- [android] fix build 8450473
- [android] fix switch fireevent with domchange b6c2d31
- [android]  fix input change event 58563b5
- [jsfm] fixed repeat binding da9f40d
- [html5] fix typo 09bcdb3
- [ios] bug fix: slider is embed in list cell d56c7ec
- [ios] fix typo for wrong quote in Podfile ab8a83b
- [html5] fix loadmore offset of list 69ed65a
- [android] fix build 0f39533
- [android] fix model callback data from string to json 3aed566
- [android] use diff vars flag the remote debug 5e976fc
- [html5] fix typo b8eec69
- [html5] fix ignore de5837a
- [android] fix bug - refresh not compatible MI2 c814cd9
- [android] fix problem that session is null and init framework failed b985d59
- [ios] fix element cssstyle selected bug 41450a9
- [ios] fix crash from cssstyle e079e5c
- [android] update the debug init sequence to make sure the handshake well 28741da
- [android] fix example file name fc7862c
- [ios] fix #567 3be6561
- [android] update debug switch f9369bb
- [android] reuse image adapter for avoid Fresco bug 62dc67c
- [android] add debug proxy, adapt to the weex-debugger d93f9e5
- [android] fix ut bug 85c5cae
- [android] fix NPE in stream f5f8a4d
- [android] fix getScreenWidth npe e264f2a
- [android] fix register order 5c8a264
- [ios] Stupid scoll view, always reset content offset to zero after insert cells, fixed it by hooking 981dcd6
- [ios] fix data type d815e2a
- [ios] update debug tool 3d82054
- [ios] fix input component width calculation ba9a3b7
- [ios] fix notes about WXComponent+ViewManagement d31e8b6
- [android] fix list component addsubview error 69c3286
- [ios] fix inverted-logic 773bea7
- [android] fix crash when list append tree 686e450
- [ios] fix playground scan qr code bug 28f7de2
- [android] fix bug - Scroll getView 647a927
- [android] fix bug - delete </merge> 77680f1
- [android] fix bug - nested scroll ac50cd2
- [android] fix onTouchEvent bug 911e569
- [ios]fix thread safe method indexOfObject in array 1623165
- [ios] remove the timing of network debug 5b00fe5
- [ios] fix warning e1d496c
- [ios] remove unnecessary removeFromSuperView from unload method and fix async unload view issue in list. b6c3a62
- [android] fix unit test 4dda4ac
- [android] update power info and debug protocol, mark stable 3323776
- [ios] fix indicator style  abnormal when update 7dcb54b
- [ios] fix list delete issue 579b720
- [ios] fix indicator item size abnormal e3cd91b
- [android] bufix NPE 904146e
- [android] fix  multithreading exception b1c2af1
- [android] update debug info 7a82747
- [android] fit bug - indicator error 673b7b6
- [android] fix bug loading-indicator 981728a
- [android] fix unit test aafec20
- [android] bugfix  getInstance singleton 397dc39
- [android] fix WXSwipeRefreshLayout 745705d
- [ios] fix scan qrcode after back from scan result failed 0b308aa
- [android] fix unit test 02d8489
- [android] fix create body NPE 89690f7
- [android] fix getView()==null crash b0ca974
- [android] bugfix NPE 93ef1e1
- [android] bug fix NPE when instance destroy e7aad12
- [android] NPE bugfix 9f15552
- [android] NPE bug fix when recycleImage 4edecef
- [android] bugfix layout can not be reset for list b19c0a1
- [android] add DebugAdapter f10cf9c
- [android] fix text layout unexpect quit when text is zero-length content 9320623
- [android] fix spell error 23c5440
- [android] update debug 567e9e0
- [android] fix list remove cell crash bug 84844f9
- [android] fix unit test be636fb
- [android] update debug tool 8d9cad9
- [android] bug fix remove fixed node f867bd1
- [android] update debug tool 5b75507
- [android] fix unit test 40b10de
- [android] update debug.move some code from WXPrettyFish to WXDebugTool c70927b
- [android] update debug 6047c43
- [android] update debug add class ScalpelFrameLayout d956819
- [android] fix status badge url 35a491b
- [android] fix border style 329f128
- [android] update weex_debug grade wrapper 793c67b
- [android] fix fetch response data 1503431
- [android] fix progress callback 1f72495
- [android] bugfix init sdk b6946ad
- [android] NPE fix fc9b595
- [android] update how-to-debug.md playgroup-app.md c61d7b4
- [android] update debug function 1bcfdd3
- [android] fix v8 compile potential crash & closed v8  snapshot 6d159a6
- [android] update debug d7c6d61
- [android] fix v8 compile potential crash & closed v8  snapshot 67b01a1

### Downloads
- [Source code (zip)](https://github.com/alibaba/weex/archive/v0.6.1.zip)
- [Source code (tar.gz)](https://github.com/alibaba/weex/archive/v0.6.1.tar.gz)
- [Cocoapods WeexSDK](https://cocoapods.org/pods/WeexSDK)
- [Bintray weex_sdk](https://bintray.com/alibabaweex/maven/weex_sdk)

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

### Bugfixes
- [android] fix crash https://github.com/alibaba/weex/commit/4fa27716d82d90c5e16d023c6e55cf65eeb9136b
- [android] fix #20, android slider indicator edge is not smooth https://github.com/alibaba/weex/commit/5b8f3a7e6d4d547d892223d8abc02ac2084b5a6e
- [android] fix List https://github.com/alibaba/weex/commit/4acc90a1c8c0a9181d91e2ef984266c39d255cac
- [android] fix NavBar https://github.com/alibaba/weex/commit/8b4ea4f0c18d4e732749c025ea496c59d19004a5
- [android] fix position=fixed did not setLayout https://github.com/alibaba/weex/commit/3b9128c2a421243d7c28f64abaebcbffd6a2ec73
- [jsfm] fix change data when ready https://github.com/alibaba/weex/commit/0a1aa5db40713abb2923a2effb06c28349ac0213
- [jsfm] fix when the key of trackby is zero https://github.com/alibaba/weex/commit/efa50707efd24cfb775c7e0f467279bbaade1fd6
- [jsfm] fix append tree https://github.com/alibaba/weex/commit/d3871b308e642fd71c2b7ba630aed0f573079daa
- [jsfm] fix transformer version https://github.com/alibaba/weex/commit/9f21bccb07869883edc261516272aa5e41933c78
- [jsfm] fix inline event test https://github.com/alibaba/weex/commit/1ce3212a83c58187a2be0df43eecafababce8ecb
- [jsfm] fixed transformer 0.3 https://github.com/alibaba/weex/commit/66b647afa08671812a7c5e11b440c23ae18750d9
- [jsfm] fixed jscs error https://github.com/alibaba/weex/commit/a5067d52e2a1708852ab704a9004f5c35f3451b6
- [jsfm] hotfixed transformer version bug and fixed a small sample code bug https://github.com/alibaba/weex/commit/48862dc40375bbc8caa30384d45b031440694f3b
- [jsfm] fix callback https://github.com/alibaba/weex/commit/2f85737c51ebd037a72db1c18daf4a9690c45837
- [jsfm] fix #1 #12 https://github.com/alibaba/weex/commit/56e6a79c50f1b2fd3c0ecebec000e9a693cb4b92
- [jsfm] fix undefined for element's depth https://github.com/alibaba/weex/commit/2b7eb9ea82e6552b6e57fddb332dfbbfa3401473
- [jsfm] fix calling functional data-binding multiply https://github.com/alibaba/weex/commit/dcd9b660912bea69b17622e13fddb80ecb42bc35

### Downloads
- [Source code (zip)](https://github.com/alibaba/weex/archive/v0.5.0.zip)
- [Source code (tar.gz)](https://github.com/alibaba/weex/archive/v0.5.0.tar.gz)
- [Cocoapods WeexSDK](https://cocoapods.org/pods/WeexSDK)
- [Bintray weex_sdk](https://bintray.com/alibabaweex/maven/weex_sdk)

------