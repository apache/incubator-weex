---
title: use weex-toolkit
type: guide
order: 5.1
version: 2.1
---

# weex-toolkit

[weex-toolkit](https://github.com/weexteam/weex-toolkit) is an official command line tool to help developers to create, debug and build their Weex project.

### Install
use npm to install weex-toolkit:

``` bash
$ npm install -g weex-toolkit
```
if you make it, you can input "weex" in your command line terminal to test it. The right output:


![weex-commands](https://img.alicdn.com/tfs/TB1NBhdQXXXXXXzXFXXXXXXXXXX-712-343.png)

if you have never installed node.js, you should go [node.js.org]( https://nodejs.org/en/) to download and install it. 
* node engine version >= 6. You can try [n](https://github.com/tj/n) to manage your node versions*
If you meet some errors when installing like `permission error`, please go [weex-toolkit issues](https://github.com/weexteam/weex-toolkit/issues) to find some solution or have a discuss with us. 


### initialize Weex project
```bash
$ weex init awesome-project
```
After command running, you can find `awesome-project` directory and there are some Weex templates in it. 
There are some useful npm scripts you will use in the future:

- `build`: build the source code and generate the JS bundle
- `dev`: run webpack watch configuration
- `serve`: start a web server
- `debug`: open the debug mode

Then we run `npm run dev & npm run serve` in root directory to start watch mode and static server. 

Finally, we can see the Weex page in `http://localhost:8080/index.html`.

### preview Weex page in time

weex-toolkit supports previewing your Weex file(`.we` or `.vue`) in a watch mode. You only need specify your file path.

``` bash
$ weex src/foo.vue 
```
The browser will auto open the location and you could see the layout and effects of your Weex page. If you have a [Playground](https://weex.apache.org/cn/playground.html) app in your mobile devices, you can scan the QR code at the opened page.
Try the command below, you’ll preview the whole directory files.

``` bash
$ weex src --entry src/foo.vue
```
You need to specify the directory path and the entry file(`--entry`).
### build Weex project
Use ` weex compile ` to build your single weex file or the whole project.
``` bash
weex compile src/foo.vue dist
```
There are two arguments. One is your code source path (a single file or a directory) and another is your destination path you want to build

### debug 

weex-toolkit has the ability to extend third party script.  And **[Weex devtools](https://github.com/weexteam/weex-devtool)** is a custom devtools for Weex that implements [Chrome Debugging Protocol](https://developer.chrome.com/devtools/docs/debugger-protocol), it is designed to help you quickly inspect your app and debug your JS bundle source in a Chrome web page, both android and iOS platform are supported. So you can use weex-devtools feature by weex-toolkit.

#### usage

``` bash
weex debug [options] [we_file|bundles_dir]

  Options:

    -h, --help           output usage information
    -V, --verbose        display logs of debugger server
    -v, --version        display version
    -p, --port [port]    set debugger server port
    -e, --entry [entry]  set the entry bundlejs path when you specific the bundle server root path
    -m, --mode [mode]    set build mode [transformer|loader]
    -w, --watch          watch we file changes auto build them and refresh debugger page![default enabled]
```
#### start debugger

```
$ weex debug
```


this command will start debug server and launch a chrome opening `DeviceList` page.
this page will display a qrcode ,you can use `Playground App` scan it for starting debug.

#### debug `.we` | `.vue` file

```
$ weex debug your_weex.vue
```

this command will compile `your_weex.we` to `your_weex.js`  and start the debug server as upon command.
`your_weex.js` will deploy on the server and displayed in `DeviceList` page as  another qrcode contain the url of your_weex.js.

.
#### start debugger with a directory of we files


```
$weex debug your/we/path  -e index.we
```

this command will build every file in `your/we/path `and deploy them on the bundle server. your directory will mapping to  http://localhost:port/weex/ 
use `-e` to set the entry of these bundles. and the url of `index.we` will display on device list page as another qrcode.


#### Features

##### Connect devices

![devtools-main](https://img.alicdn.com/tps/TB13fwSKFXXXXXDaXXXXXXXXXXX-887-828.png)

##### Inspector

 Inspector can be used to show your `Element` \ `Network` \ `Console log` \ `ScreenCast` \ `BoxModel` \ `Native View` and so on.

![devtools-inspector](https://img.alicdn.com/tps/TB1O.nwKFXXXXX8XpXXXXXXXXXX-1436-811.png)
##### Element

![inspector-element](https://img.alicdn.com/tps/TB1.02bKFXXXXXwaXXXXXXXXXXX-2880-1800.png)
##### NetWork
##### show the total time and latency

![inspector-network](https://img.alicdn.com/tps/TB1NjO_KFXXXXcaaXXXXXXXXXXX-2880-1800.png)
##### show the header and response

![inspector-network](https://img.alicdn.com/tps/TB1ck6lKFXXXXbZXFXXXXXXXXXX-2880-1800.png)
##### Console

![inspector-console](https://img.alicdn.com/tps/TB1a7HqKFXXXXXMXFXXXXXXXXXX-2880-1800.png)
##### Resource

![inspector-resource](https://img.alicdn.com/tps/TB1oY6cKFXXXXXQaXXXXXXXXXXX-2880-1800.png)
#### Debugger

![devtools-debugger](https://img.alicdn.com/tps/TB1aPTEKFXXXXXaXXXXXXXXXXXX-1436-813.png)
##### Breakpoint and CallStack

![debugger-breakpoint](https://img.alicdn.com/tps/TB1_trbKFXXXXc0XVXXXXXXXXXX-2880-1800.png)
#### Integrate devtools
* Android
    * See the doc [Weex devtools (Android)](../../references/advanced/integrate-devtool-to-android.html), it will lead you to config and use it step by step.
* IOS
    * See the doc [Weex devtools (IOS)](../../references/advanced/integrate-devtool-to-ios.html), it will lead you to config and use it step by step.
  
### weex-toolkit extends weexpack command

[weexpack](https://github.com/weexteam/weex-pack) helps to setup Weex application from scratch quickly. With simple commands, developers could create a Weex project, add different platform template, could install plugins from local, GitHub or Weex market, could pack up his application project and run on mobile. For those who would like to share his own plugins, he could publish them to the Weex market.

Now weex-toolkit can run the same commands of `weexpack` because of the new architecture. If your directory is generated by `weexpack`, you can build your iOS or android app.

### weex platform and run commands

Use `platform add|remove` to add or remove Weex app template and run it in your target devices.

``` bash
$ weex platform add ios
```
If you use these commands firstly, you might see the prompt. Just enter Y.

![install weexpack](https://gw.alicdn.com/tfs/TB19n4AQXXXXXawXVXXXXXXXXXX-577-70.png)

Then run platform, you will see an iPhone simulator.

``` bash
$ weex run ios
```


### weex plugin commands

If you want to use some plugins on the [weex market](https://market.dotwe.org), weex-toolkit is the right choice.

```bash
$ weex plugin add plugin_name
```
You need to specify the plugin name from market like "weex-chart":

``` bash
$ weex plugin add weex-chart
```

Remove some plugins(eg: weex-chart):

``` bash
$ weex plugin remove weex-chart
```

Learn more about [weexpack](https://github.com/weexteam/weex-pack) .