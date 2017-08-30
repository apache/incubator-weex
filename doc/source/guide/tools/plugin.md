---
title: Weex Language Support Plugin
type: guide
order: 5.1
version: 2.1
---

# Weex Language Support Plugin

[Weex Language Support](https://plugins.jetbrains.com/plugin/9189-weex-language-support) is a official tools to code highlight, automatic completion，lint and other functions in IDEA, WebStorm or the others IDEs.

### Supported IDEs
You can install and use this plugin on the following IDEs on any operating system:       
**IntelliJ IDEA Ultimate, PhpStorm,  WebStorm,  PyCharm,  RubyMine,  AppCode,  CLion,  Gogland,  Rider**

### Install
Just searching the `Weex Language Support` in plugin repo to install, next you need restart IDE to enable it.    
![install plugin](https://img.alicdn.com/tfs/TB1y6nrXwvGK1Jjy0FdXXaxzVXa-1316-462.png)

### Configurations
Open `Preferences -> Other Settings -> Weex language support` to configuration plugin   
![plugin settings](https://img.alicdn.com/tfs/TB1FonrXwvGK1Jjy0FgXXX9hFXa-559-244.png)    
- Target Weex Version: Config the version of Weex that your current project in use, default is `LATEST`, it means always using the latest version
- Vue Support: Config whether to support Vue, you need to restart IDE after turning on or off the set to take effect
- Custom Rules: Import the custom Weex DSL rules, The format of the custom rules will be listed later
- Global Weex Components: Sets the location of the module that is applied in the project, in particular, the `node_modules` directory in current project and npm root will be automatically included, you do not need to add them here


### Format of Custom DSL Rules 
Custom rules are included in a json file, the root node of the json file is an array, each element in the array corresponds to a label in the DSL.
Let's take the example of the `loading>` tag: 
```js
{
    "tag": "loading", //tag name, not null
    "attrs": [ //attributes of tag, can be null
      {
        "name": "display", //attribute name, not null
        "valuePattern": null, //pattern expression to check the attribute value, can be null
        "valueEnum": [ //attribute value enumeration, can be null
          "show",
          "hide"
        ],
        "valueType": "var", //type of attribute value, must be var or function
        "since": 0, //which version the attribute is added to sdk, such as 0.11
        "weexOnly": false //whether the attribute is available only in 1.0 syntax, default is false
      }
    ],
    "events": [ //events list, can be null
      {
        "name": "loading", //event name, not null
        "since": 0 //which version the event is added to sdk
      }
    ],
    "parents": [ //The tag is allowed to be a child of which tags, null means no restrictions
      "list",
      "scroller"
    ],
    "childes": [ //which tags are allowed as their own child tags, null means no restrictions
      "text",
      "image",
      "loading-indicator"
    ],
    "document": "/references/components/loading.html" //document link
  }
```    

### Contribution
Please commiting Issues and Pull Requests into the [weex-language-support](https://github.com/misakuo/weex-language-support) project