---
title: Using Weexpack
type: guide
order: 5.2
version: 2.1
---

# Using Weexpack

## Weexpack introduction

Weexpack is our next generation of engineering development kits, it helps to setup wWex application from scratch quickly. With simple commands, developers could create a Weex project, add different platform template, could install plugins from local, GitHub or Weex market, could pack up his application project and run on mobile. For those who would like to share his own plugins, he could publish them to the Weex market.

weexpack commands can be grouped to three parts:
* application package
 * **weexpack create** — create Weex project.
 * **weexpack platform add/remove** — add／remove Weex platform template，it support plugins and Weex bundle debugging by default.
 * **weexpack platform list** — query installed platforms and its version.
 * **weexpack platform run** - pack/build/run application.

* plugin usuage
 * **weexpack plugin add/remove** — add/remove Weex plugin, support to install from local, Github or market.
 * **weexpack plugin list** — query installed plugins and its version.

* plugin development
 * **weexpack plugin create** - create plugin template with necessary config file and directory.
 * **weexpack plugin publish** - publish plugin to market.


## Installation
### pre-environmental requirements

- Supported on Mac linux windows.
- Configure the [Node.js] [1] environment and install the [npm] [2] package manager.(need node 6.0+)
- Configure iOS development environment:
    - Install [Xcode IDE] [3] to launch Xcode once so that Xcode automatically installs the Developer Tools and confirms the usage protocol.
    - Install cocoaPods
- Configure the Android development environment:
    - Install [Android Studio] [4] and open the new project. Open the menu bar, open the [AVD Manager] [5], the new Android emulator and start. (If you have installed [Docker] [6], close the Docker Server.)
    - Or just download the [Android SDK] [7], run the command line [AVD Manager] [8], create a new Android emulator and launch it.
    - Make sure that Android build-tool version is 23.0.2

### Instructions

First, install the weex-pack command globally:

```bash
$ npm install -g weexpack
```

or run the following command under the root path of your cloned weexpack repo

```bash
$ npm install
```

## Pack application (and plugin usuage)

Following graph shows how to pack a application, plugin related parts werenot a must.

![weexpack1](https://img.alicdn.com/tps/TB1.bpVPXXXXXarapXXXXXXXXXX-462-310.png)

### Steps in detail

#### 1. create a weexpack project

```bash
$ weexpack create appName
```

will create directory as follows：

```
WeexProject
├── README.md
├── android.config.json
├── config.xml
├── hooks
│   └── README.md
├── ios.config.json
├── package.json
├── platforms     // platform template directory
├── plugins       // plugins download to here
│   └── README.md
├── src           // application（we/vue file）directory
│   └── index.we
├── start
├── start.bat
├── tools
│   └── webpack.config.plugin.js
├── web
│   ├── index.html
│   ├── index.js
│   └── js
│       └── init.js
└── webpack.config.js
```

Above project template has no ios/android directory by default, you should change to appName path to install necessary dependency.

```bash
$ cd appName && npm install
```

#### 2. install Weex application template

Application template by default support Weex plugins and Weex bundle debugging. Please note template name are in lower-case, template was installed under platforms directory.

* android template

```bash
$ weexpack platform add android
```

* ios template

```bash
$ weexpack platform add ios
```

Take android platform as example，the directory under platforms looks as following:

```
platforms
└── android
    ├── LICENSE
    ├── NOTICE
    ├── README.md
    ├── app
    │   ├── build
    │   ├── build.gradle
    │   ├── proguard-rules.pro
    │   └── src
    ├── appframework
    │   ├── build
    │   ├── build.gradle
    │   ├── proguard-rules.pro
    │   └── src
    ├── build
    │   └── generated
    ├── build.gradle
    ├── codeStyleSettings.xml
    ├── gradle
    │   └── wrapper
    ├── gradle.properties
    ├── gradlew
    ├── gradlew.bat
    ├── settings.gradle
    └── weexplugin
        ├── build.gradle
        ├── proguard-rules.pro
        └── src

```

* query installed platforms

  ```bash
  $ weexpack platform list
  ```

  you will get result like：

  ```
  Installed platforms:
    android
  Available platforms:
    android ^6.2.1
  ```

* remove platform template

  Please use remove command to remove unnecessary platform, for example user want to remove ios template in windows platform.

  ```bash
  $ weexpack platform remove ios
  ```

#### 3. install Weex plugin（optional）

* add plugins from local

  ```bash
  $ weexpack plugin add path/to/plugin
  ```

* add plugins from Weex market，for example weex-chart

  ```bash
  $ weexpack plugin add weex-chart
  ```

  weexplugin directory was the target directory where plugin was installed. Take android as an example：

  ```
    weexplugin
    ├── build.gradle  //  plugin build script, maintained by weexpack tool
    ├── libs
    ├── proguard-rules.pro
    ├── src
    │   └── main
    │       ├── AndroidManifest.xml // plugin android manifest file
    │       ├── java
    │       │   ├── // 插件src安装目录
    │       │   └── com
    │       │       └── alibaba
    │       │           └── weex
    │       │               └── plugin
    │       │                   ├── ConfigXmlParser.java // config file parser
    │       │                   ├── PluginConfig.java    // external interface
    │       │                   ├── PluginEntry.java     // plugin description
    │       │                   └── PluginManager.java   // plugin manager
    │       └── res // plugin resource path
    │           ├── drawable
    │           ├── values
    │           │   └── strings.xml
    │           └── xml
    │               └── config.xml // plugin config file, which used by PluginManager when install
  ```

* query installed plugins

  ```bash
  $ weexpack plugin list
  ```

* remove installed plugin，for example weex-chart

  ```bash
  $ weexpack plugin remove weex-chart
  ```

#### 4. pack application and run

After all above steps, you could modify your application we/vue files under src directory. And then, use run command to pack and run your application. Run command will download building tools and all dependencies which may take a while, and open monitor or connect to mobile device to execute your application afterwards.

* for android application

  ```bash
  $ weexpack run android
  ```

  You could modify your project information in android.config.json file:

  - AppName: the name of the project
  - AppId: application_id the name of the package
  - SplashText: the text in welcome page
  - WeexBundle: the bundle file (could be local file or remote url). For Local file please put your we/vue files under the src directory

* for ios platform

  * simulator

    ```bash
    $ weexpack run ios
    ```

  * Build ipa

    ```bash
    $ weexpack build ios
    ```

  this command will prompt for CodeSign, Profile(provisioning profile), AppId to build ipa. Other information like AppName, Weex bundle could be configured as you like in the file ios.config.json. After this command, ipa file could be created under the directory  `/playground/build/ipa_build/`.

  Note: For details about various requirements to deploy to a device, refer to the Launch Your App On Devices section of Apple's [About App Distribution Workflows][9]. Briefly, you need to do the following before deploying:

  1. CodeSign should be installed to keychain, click keychain to get the id;
  2. provisioning profile need UUID. you could use the file mobileprovision_UUID.sh to generate UUID as follows:

    ```
    $ ./mobileprovision_UUID.sh abcpath
    ```
    abcpath is the path of provisioning profile file.

* for Web platform

  ```bash
  $ weexpack run web
  ```

## Plugin development and publishment

![weexpack2](https://img.alicdn.com/tps/TB18hxjPXXXXXXgapXXXXXXXXXX-248-258.png)

[Plugin development guide](https://github.com/weexteam/weex-pack/blob/master/doc/plugin-devloping-web.md)

## Examples

 [Create Weex One App with Weexpack](https://github.com/weexteam/weex-pack/wiki/Create-Weex-One-App-with-Weexpack)

  [1]: https://nodejs.org/
  [2]: https://www.npmjs.com/
  [3]: https://itunes.apple.com/us/app/xcode/id497799835?mt=12
  [4]: https://developer.android.com/studio/install.html
  [5]: https://developer.android.com/studio/run/managing-avds.html
  [6]: https://www.docker.com/
  [7]: https://developer.android.com/studio/releases/sdk-tools.html
  [8]: https://developer.android.com/studio/run/managing-avds.html
  [9]: https://developer.apple.com/library/content/documentation/IDEs/Conceptual/AppDistributionGuide/Introduction/Introduction.html
