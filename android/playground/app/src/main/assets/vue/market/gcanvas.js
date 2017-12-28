// { "framework": "Vue" }

/******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};

/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {

/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId])
/******/ 			return installedModules[moduleId].exports;

/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			exports: {},
/******/ 			id: moduleId,
/******/ 			loaded: false
/******/ 		};

/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);

/******/ 		// Flag the module as loaded
/******/ 		module.loaded = true;

/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}


/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;

/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;

/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "";

/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(0);
/******/ })
/************************************************************************/
/******/ ({

/***/ 0:
/***/ (function(module, exports, __webpack_require__) {

	var __vue_exports__, __vue_options__
	var __vue_styles__ = []

	/* script */
	__vue_exports__ = __webpack_require__(110)

	/* template */
	var __vue_template__ = __webpack_require__(119)
	__vue_options__ = __vue_exports__ = __vue_exports__ || {}
	if (
	  typeof __vue_exports__.default === "object" ||
	  typeof __vue_exports__.default === "function"
	) {
	if (Object.keys(__vue_exports__).some(function (key) { return key !== "default" && key !== "__esModule" })) {console.error("named exports are not supported in *.vue files.")}
	__vue_options__ = __vue_exports__ = __vue_exports__.default
	}
	if (typeof __vue_options__ === "function") {
	  __vue_options__ = __vue_options__.options
	}
	__vue_options__.__file = "/Users/Hanks/Codes/work/incubator-weex/examples/vue/market/gcanvas.vue"
	__vue_options__.render = __vue_template__.render
	__vue_options__.staticRenderFns = __vue_template__.staticRenderFns
	__vue_options__.style = __vue_options__.style || {}
	__vue_styles__.forEach(function (module) {
	  for (var name in module) {
	    __vue_options__.style[name] = module[name]
	  }
	})
	if (typeof __register_static_styles__ === "function") {
	  __register_static_styles__(__vue_options__._scopeId, __vue_styles__)
	}

	module.exports = __vue_exports__
	module.exports.el = 'true'
	new Vue(module.exports)


/***/ }),

/***/ 110:
/***/ (function(module, exports, __webpack_require__) {

	'use strict';

	//
	//
	//
	//
	//


	var GCanvas = __webpack_require__(111);
	var Image = __webpack_require__(112);

	module.exports = {
	  data: function data() {
	    return {
	      tagName: weex.config.env.platform === 'Web' ? 'canvas' : 'gcanvas'
	    };
	  },

	  mounted: function mounted() {
	    var ref = this.$refs.canvas_holder;
	    var gcanvas = GCanvas.start(ref);
	    var ctx = gcanvas.getContext('2d');

	    ctx.fillStyle = 'red';
	    ctx.fillRect(0, 0, 100, 100);

	    ctx.fillStyle = 'black';
	    ctx.fillRect(100, 100, 100, 100);
	    ctx.fillRect(25, 210, 700, 5);

	    ctx.arc(450, 200, 100, 0, Math.PI * 2, true);
	    ctx.fill();

	    var img = new Image();
	    img.onload = function () {
	      ctx.drawImage(img, 100, 200, 210, 330);
	      // ctx.drawImage(img, 0,0,105,165, 100, 200, 210, 330);
	    };
	    img.src = 'https://img.alicdn.com/tps/TB1TFNdKVXXXXbeaXXXXXXXXXXX-210-330.png';
	  }
	};

/***/ }),

/***/ 111:
/***/ (function(module, exports, __webpack_require__) {

	/**

	gcanvas.js使用说明:
	1、引入gcanvas库
	2、调用gcanvas库的createElement(component)接口，创建一个canvas对象。
	3、调用canvas对象的getContext(param)，获取用于渲染的context。

	扩展用法：
	1、对于Android环境，部分机型可能无法运行。建议在页面入口处调用gcanvas库的start(successCallback, errorCallback)函数，进行黑白名单判断。
	2、默认每16ms，会自动下发一次渲染指令。某些特殊场景下，希望自行控制下发频率的，可直接调用context.render()接口。调用后会关闭自动下发的操作，切换成每次主动调用render时才下发。

	完整示例如下：
	var libGCanvas = require('../../core/gcanvas');
	libGCanvas.start(function(){
	    nativeLog('gcanvas.start success');
	    var canvasObj = libGCanvas.createElement(gcanvasComponent);
	    var context = canvasObj.getContext('2d');
	    //do any action here
	},function(){
	    nativeLog('gcanvas.start failed');
	});

	*/

	var GImage = __webpack_require__(112);
	var GBridge = __webpack_require__(113).GBridge;
	var GLog = __webpack_require__(113).GLog;
	var GContextWebGL = __webpack_require__(116);
	var GContext2D = __webpack_require__(117);
	var GHashMap = __webpack_require__(115);
	var htmlPlugin = __webpack_require__(118);


	///////////////////////////////
	var GSupport = {};
	var model_check;
	var version_check;
	GSupport.renderMode = 0;// 0--RENDERMODE_WHEN_DIRTY, 1--RENDERMODE_CONTINUOUSLY
	GSupport.hybridLayerType = -1;// 0--LAYER_TYPE_NONE 1--LAYER_TYPE_SOFTWARE 2--LAYER_TYPE_HARDWARE. change hybrid layer type from LAYER_TYPE_SOFTWARE to unset, avoid block when use html5 audio.
	GSupport.checkType = 0;// 0--all support, 1--white list check
	GSupport.nativeVer = 0;
	GSupport.defaultHiQualityMode = true; // false-- normal true--hiQuality
	GSupport.supportScroll = false;
	GSupport.newCanvasMode = false;             //true: GCanvasView in Webview
	GSupport.sameLevel = false; //newCanvasMode = true && true: GCanvasView and Webview is same level;
	GSupport.clearColor = "white";
	GSupport.WHITE_LIST = {

	    model_check : [
	        function(info) {return info.MODEL == 'GT-I9300';},
	        function(info) {return info.MODEL == 'GT-I9500';},
	        function(info) {return info.MODEL == 'GT-N7108';},
	        function(info) {return info.MODEL == 'HIKe 848A';},
	        function(info) {return info.MODEL == 'HTC 601e';},
	        function(info) {return info.MODEL == 'HUAWEI C8813';},
	        function(info) {return info.MODEL == 'Lenovo K900';},
	        function(info) {return info.MODEL == 'M351';},
	        function(info) {return info.MODEL == 'M51w';},
	        function(info) {return info.MODEL == 'MI 3';},
	        function(info) {return info.MODEL == 'MI 3W';},
	        function(info) {return info.MODEL == 'SM-G9006V';},
	        function(info) {return info.MODEL == 'SM-N9006';}
	    ],
	    version_check : [
	        function(info) {GLog.d("info.OS_RELEASE=" + info.OS_RELEASE); return false;},
	        function(info) {return (info.OS_RELEASE >= '4.1.0')&&( info.OS_RELEASE <= '4.4.2');}
	    ]
	};


	GSupport.checkList = function(successFunc, failureFunc){
	    var checkType = GSupport.checkType;
	    GLog.d("[checkList] checkType:" + checkType);
	    if (1 == checkType) {//white list check
	        var whitelist = GSupport.WHITE_LIST;
	        var length = whitelist.length;
	        for (var i = 0; i < length; i++) {
	            var lenSub = whitelist[i].length;
	            var found = false;
	            for (var j = 0; j < lenSub; j++){
	                if (whitelist[i][j](GDeviceInfo)) {
	                    found = true;
	                    break;
	                }
	            }
	            if (!found){ // unfound in white list
	                GLog.d("the device is not supported, " + GDeviceInfo.MODEL);
	                failureFunc&&failureFunc();
	                return;
	            }
	        }
	    }
	    successFunc&&successFunc();
	};
	///////////////////////////////

	var GDeviceInfo = {};
	// var _context = null;
	// var _context_type = 0;//0--2d;1--webgl
	///////////////////////////////

	var GCanvasPlatform = GBridge.isBrowser()? 0 : 2;//0--H5;1--iOS;2--Android
	var currentEl;
	var contextTypes = ['2d','webgl']
	var currentContextOfType = {};

	function GCanvas(componentId)
	{
	    this.componentId = componentId;
	    this.id = ++(GCanvas.idCounter);
	}

	GCanvas.idCounter = 0;
	GCanvas.canvasMap = new GHashMap();

	//-----------------------------
	// GCanvas.start
	//-----------------------------
	GCanvas.start = function(el){
	    GLog.d('gcanvas#start=====>>>');

	    if (typeof WXEnvironment === 'object' && /ios/i.test(WXEnvironment.platform)) {
	        GCanvasPlatform = 1;
	    } else if (typeof navigator === 'object' && /ios/i.test(navigator.userAgent)) {
	        GCanvasPlatform = 1;
	    } else {
	        GCanvasPlatform = 2;
	    }

	    GBridge.setup( {platform:GCanvasPlatform} );

	    if(GCanvasPlatform === 0)
	    {
	        currentEl = el
	        return currentEl;
	    }
	    else 
	    {
	        //bind canvas
	        var config = [];
	        config.push(GSupport.renderMode);
	        config.push(GSupport.hybridLayerType);
	        config.push(GSupport.supportScroll);
	        config.push(GSupport.newCanvasMode);
	        config.push(1);//compatible. 1 will call GCanvasJNI.getAllParameter("gcanvas");
	        config.push(GSupport.clearColor);
	        config.push(GSupport.sameLevel);
	        GCanvas.enableRet = GBridge.callEnable(el.ref, config);
	        var canvas = new GCanvas(el.ref);
	        GCanvas.canvasMap.put(el.ref, canvas);
	        return canvas;
	    }
	}

	//-----------------------------
	// Instance Method: getContext
	//-----------------------------
	GCanvas.prototype.getContext = function(contextID){
	    GLog.d('gcanvas#getContext=====>>>');

	    var context = this.context;
	    //brower
	    if(GCanvasPlatform === 0) {
	        if (context){
	            return context;//unsupport change type after create
	        }
	        else if(currentEl&&currentEl.getContext) {
	            context = currentEl.getContext(contextID)
	            if(context&&!context.render) context.render = function(){}
	        }
	        return context
	    }

	    if (context){
	        return context;//unsupport change type after create
	    }

	    var context_type;
	    if (contextID.match(/webgl/i)){
	        context = new GContextWebGL(GCanvas.enableRet);
	        context_type = 1;
	    }else{
	        context = new GContext2D();
	        context_type = 0;
	    }

	    GBridge.setContextType(this.componentId, context_type);

	    context.componentId = this.componentId;
	    // if (!context.timer) {
	    //    context.timer = setInterval(this.render.bind(this), 16);
	    // }

	    this.context = context;
	    GBridge.callRegisterReattachJSCallback(this.componentId, context._clearImageTextures);

	    this.startLoop();

	    return context;
	}

	//-----------------------------
	// Instance Method: render
	//-----------------------------
	GCanvas.prototype.render = function(){
	    if(GCanvasPlatform !== 0 && this.context)
	    {
	        this.context.render("auto");
	    }
	}

	//-----------------------------
	// Instance Method: stopRender
	//-----------------------------
	GCanvas.prototype.stopRender = function(){
	    if(!this.context){
	        return;
	    }

	    if(this.context.timer){
	        clearInterval(this.context.timer);
	        this.context.timer = null;
	    }
	}

	//-----------------------------
	// Instance Method: startLoop
	//-----------------------------
	GCanvas.prototype.startLoop = function(fps){
	    if(!this.context){
	        return;
	    }
	 
	    fps = parseInt(fps) || 16;
	    if(!this.context.timer){
	        this.context.timer = setInterval(this.render.bind(this),fps);
	    }
	}

	//-----------------------------
	// Instance Method: stopLoop
	//-----------------------------
	GCanvas.prototype.stopLoop = function(){
	    if(!this.context){
	        return;
	    }
	 
	    if(this.context.timer){
	        clearInterval(this.context.timer);
	        this.context.timer = null;
	    }
	}


	//-----------------------------
	// Instance Method: reset
	//-----------------------------
	GCanvas.prototype.reset = function(){
	    if(GCanvasPlatform !== 0){
	        GBridge.resetComponent(this.componentId);
	    }
	}

	//-----------------------------
	// Instance Method: setDevicePixelRatio
	//-----------------------------
	GCanvas.prototype.setDevicePixelRatio = function(){
	    if(!this.context){
	        return;
	    }

	    GBridge.callSetDevPixelRatio(this.componentId);
	}

	//-----------------------------
	// GCanvas.disable
	//-----------------------------
	GCanvas.disable = function(){
	    // GLog.d('gcanvas#disable=====>>>');
	    // if(GCanvasPlatform !== 0){
	    //     GBridge.callDisable();
	    // }
	}

	GCanvas.prototype.setup = function(support){
	    if(!this.context){
	        return;
	    }
	    var config = [];
	    var mySupport = support || GSupport;

	    for(var attr in GSupport){
	      if(mySupport[attr] != undefined){
	         GSupport[attr] = mySupport[attr];
	      }
	    }

	    var config = {
	      'renderMode':GSupport.renderMode,
	      'hybridLayerType':GSupport.hybridLayerType,
	      'supportScroll':GSupport.supportScroll,
	      'sameLevel':GSupport. sameLevel,
	      'newCanvasMode':GSupport.newCanvasMode,
	      'clearColor': GSupport.clearColor
	    };

	    GBridge.callSetup(config, this.componentId, function(e){});
	}

	//-----------------------------
	// GCanvas.setHiQuality
	//-----------------------------
	GCanvas.setHiQuality = function(){
	    GLog.d('gcanvas#setHiQuality=====>>>' + quality);
	    if(GCanvasPlatform !== 0) {
	        GBridge.setHiQuality(quality);
	    }
	}

	//-----------------------------
	// GCanvas.setLogLevel
	//-----------------------------
	GCanvas.setLogLevel = function(level){
	    GLog.d('gcanvas#setLogLevel=====>>> ' + level);
	    if(GCanvasPlatform !== 0) {
	        GBridge.setLogLevel(level);
	    }
	}

	//-----------------------------
	// GCanvas.htmlPlugin
	//-----------------------------
	GCanvas.htmlPlugin = htmlPlugin;

	module.exports = GCanvas;


/***/ }),

/***/ 112:
/***/ (function(module, exports, __webpack_require__) {

	var GBridge = __webpack_require__(113).GBridge;
	var GHashMap = __webpack_require__(115);

	// GCanvasImage
	function GCanvasImage() {
	  /**
	   * The width of the image after it is loaded.
	   *
	   * @type {number}
	   */
	  this.width = 0;

	  /**
	   * The height of the image after it is loaded.
	   *
	   * @type {number}
	   */
	  this.height = 0;

	  /**
	   * A unique id assigned to each image upon creation.
	   *
	   * @type {number}
	   */
	  // this.id = (++GCanvasImage.idCounter);

	  // this._id = this.id; // public facing "id" but _id used to internally
	  // track image
	  this._src = ""; // image source path
	  this._complete = true; // "is loading" identifier for complete property
	}

	/**
	 * Iterator for generating id values for GCanvasImage instances
	 * internally.
	 *
	 * @private
	 */
	GCanvasImage.idCounter = 0;
	GCanvasImage.imageMap = new GHashMap();

	/**
	 * Callback for when an image has successfully loaded a file path assigned
	 * to {@link GCanvasImage#src}.
	 *
	 * @type {function}
	 * @name GCanvasImage#onload
	 */

	/**
	 * Callback for when an image has failed to load a file path assigned to
	 * {@link GCanvasImage#src}.
	 *
	 * @type {function}
	 * @name GCanvasImage#onerror
	 */
	GCanvasImage.prototype.removeEventListener = function(type, callback, force) {};

	GCanvasImage.prototype.addEventListener = function(type, listener, force) {
	  if (type === "load" && typeof listener === 'function') {
	    this.onload = listener;
	  }

	  if (type === "error" && typeof listener === 'function') {
	    this.onerror = listener;
	  }
	};

	/**
	 * The source property, identifying a URL path to the image's file location.
	 * Upon setting this value to a file path value, the file is loaded into the
	 * GCanvas plugin. For GCanvas images can be unloaded by setting the
	 * src to null or "".
	 *
	 * @type {string}
	 * @name GCanvasImage#src
	 * @example var myImage = GCanvas.createImage(); myImage.onload =
	 *          function(){ // ... myContext.drawImage(myImage, 0,0,100,100,
	 *          0,0,100,100); GCanvas.render(); } myImage.onerror =
	 *          function(){ console.log("Could not load image!"); } myImage.src =
	 *          "images/spritesheet.jpg";
	 */

	Object.defineProperty(GCanvasImage.prototype, "src", {
	  get: function() {
	    return this._src;
	  },
	  set: function(src) {

	    if (!src || src === this._src) {
	      return;
	    }

	    this._src = src;

	    // Loading
	    this.complete = false;

	    // callback wrappers
	    var me = this;

	    var data = GCanvasImage.imageMap.get(src);
	    if (data) {
	      me.id = data.id;
	      me._id = data.id;
	      me.complete = true;
	      me.width = data.width;
	      me.height = data.height;
	      me.onload && me.onload();
	      return;
	    }

	    this.id = (++GCanvasImage.idCounter);
	    this._id = this.id; // public facing "id" but _id used to internally

	    GBridge.preLoadImage([src, this.id], function(data) {
	      if (typeof data === 'string') {
	        try {
	          data = JSON.parse(data);
	        } catch (err) {}
	      }

	      if (data.error) {
	        me.onerror && me.onerror();
	      } else {
	        me.complete = true;
	        me.width = typeof data.width === 'number' ? data.width : 0;
	        me.height = typeof data.height === 'number' ? data.height : 0;

	        me.onload && me.onload();
	        GCanvasImage.imageMap.put(src, data);
	      }
	    });
	  }
	});

	/**
	 * False when the image is in the process of loading an image after the src
	 * property has been set. True when loading is complete or if src is never
	 * set. If an error occurred when attempting to load the image, once the
	 * process of loading is complete, despite the error, this value will still
	 * become true.
	 *
	 * @type {boolean}
	 * @name GCanvasImage#complete
	 */
	Object.defineProperty(GCanvasImage.prototype, "complete", {
	  get: function() {
	    return this._complete;
	  },
	  set: function(value) {
	    this._complete = value;
	  }
	});

	GCanvasImage.prototype.tagName = 'img';
	GCanvasImage.prototype.getAttribute = function(name) {
	  return this[name];
	};

	module.exports = typeof Image === 'function' ? Image : GCanvasImage;

/***/ }),

/***/ 113:
/***/ (function(module, exports, __webpack_require__) {

	/////////////////////////////////////////////////////////////////
	//GBridge
	/////////////////////////////////////////////////////////////////
	var GLog = __webpack_require__(114).GLog;

	var inWeex = typeof callNative !== 'undefined';
	var debug = true;
	var platform;
	var canvasModule;

	canvasModule = (typeof weex!=='undefined'&&weex.requireModule) ? ( weex.requireModule('gcanvas') ) : (__weex_require__('@weex-module/gcanvas') );

	var GBridge = {

	    setup: function(data){
	        platform = data.platform;
	    },

	    isIOS: function(){
	      return platform === 1;
	    },

	    callRegisterReattachJSCallback: function(componentId, cb){
	      if(!inWeex){
	        return;
	      }

	      if(typeof cb !== 'function'){
	        return;
	      }

	      canvasModule.registerRetachFunction && canvasModule.registerRetachFunction(componentId, cb);
	    },

	    /**执行render指令*/
	    callRender: function (componentId, commands) {
	        if (!inWeex) {
	            return;
	        }

	        canvasModule.render && canvasModule.render( commands, componentId );
	    },

	    /**Android use**/
	    callSetup:function(configObj, componentId, callback){
	        if (!inWeex) {
	            return;
	        }

	        var config = configObj || {};
	        //GLog.d('bridge#callRender() commands is ' + commands);
	        canvasModule.setup && canvasModule.setup(JSON.stringify(config), componentId , callback);
	    },

	    /**预加载图片*/
	    preLoadImage: function (image, cb) {
	        if (!inWeex) {
	            return;
	        }

	        //返回width和height
	        //image[src, id]
	        canvasModule.preLoadImage(image, function (e) {
	            GLog.d('bridge#preLoadImage() callback, e ' + JSON.stringify(e));
	            e.url = image[0];
	            e.id = image[1];
	            cb && cb(e);  
	        });
	    },

	    /**绑定纹理*/
	    bindImageTexture: function (componentId, src, callback) {
	        if (!inWeex) {
	            return;
	        }

	        canvasModule.bindImageTexture && canvasModule.bindImageTexture(src, componentId, callback);
	    },

	    /**
	     * 获取canvas引用
	     * @param ref wx-canvas 引用
	     * @param configArray 配置参数
	     **/
	    callEnable: function (ref, configArray, callback) {
	        if (!inWeex) {
	            return;
	        }
	        var params = {
	            componentId: ref,
	            config:configArray
	        };

	        return canvasModule.enable && canvasModule.enable(params);

	        // canvasModule.enable(params, function (e) {
	        //     GLog.d('bridge#callEnable() return val:' + JSON.stringify(e));
	        //     callback && callback(e);
	        // });
	    },

	    callSetDevPixelRatio: function(componentId){
	        if(!inWeex){
	          return;
	        }
	        canvasModule.setDevicePixelRatio && canvasModule.setDevicePixelRatio(componentId);
	    },

	    /**
	     * 获取设备信息(android)
	     * @param callback 设备信息
	     **/
	    getDeviceInfo: function (callback) {
	        if (!inWeex) {
	            return;
	        }

	        if(this.isBrowser()){
	            //浏览器端不实现
	            callback && callback({
	                data:{platform:0}
	            });
	        }
	        else {
	            canvasModule.getDeviceInfo({}, function (e) {
	                GLog.d('bridge#getDeviceInfo() return val:' + JSON.stringify(e));
	                callback && callback(e);
	            });
	        }

	    },

	    /**
	     * 判断是不是浏览器
	     *
	     **/
	    isBrowser: function () {

	        if(!canvasModule||!canvasModule.getDeviceInfo){
	            return true
	        }
	    },

	    /**
	     *
	     * 设置context类型,2d或者webgl
	     *
	     * @param context_type 0代表2d,1代表3d
	     * */
	    setContextType: function (componentId, context_type){
	        if(context_type != 0 && context_type != 1){
	            GLog.d('bridge#setContextType(): invalid context type===>' + context_type);
	            return;
	        }
	        GLog.d('bridge#setContextType(): context type is ' + context_type + ' componentId:' + componentId);
	        canvasModule.setContextType(context_type, componentId);
	    },

	    /**
	     *
	     * 设置日志级别
	     *
	     * @param context_type 0代表2d,1代表3d
	     * */
	    setLogLevel: function (level){
	        GLog.d('bridge#setLogLevel(): native logLevel ' + level);
	        canvasModule.setLogLevel(level);
	    },

	    /**
	     *
	     * 设置opengl渲染质量
	     *
	     * @param context_type 0代表2d,1代表3d
	     * */
	    setHiQuality: function (quality){
	        GLog.d('bridge#setHiQuality(): quality: ' + quality);
	        canvasModule.setHiQuality(quality);
	    },


	    resetComponent: function(componentId){
	        GLog.d('bridge#resetComponent(): componentId: ' + componentId);
	        canvasModule.resetComponent && canvasModule.resetComponent(componentId);
	    },

	    exeSyncCmd: function (componentId, action, args){
	    	GLog.d('bridge#exeSyncCmd(): action: ' + action + ',args:' + args);
	    	return canvasModule.execGcanvaSyncCMD(action,args);
	    }
	};


	module.exports = {
	    GBridge: GBridge,
	    GLog: GLog
	};


/***/ }),

/***/ 114:
/***/ (function(module, exports) {

	/////////////////////////////////////////////////////////////////
	//GLog
	/////////////////////////////////////////////////////////////////
	var GLOG_DEBUG	= 0;
	var GLOG_INFO   = 1;
	var GLOG_WARN   = 2;
	var GLOG_ERROR	= 3;
	var GLOG_NULL   = -1;
	var GLog = {};
	GLog._nullFunc = function(){};
	GLog.d = GLog._nullFunc;
	GLog.i = GLog._nullFunc
	GLog.w = GLog._nullFunc;
	GLog.e = GLog._nullFunc;
	GLog._nativeEnable = false;
	GLog._setNativeLevel = function(level){
		/*
		if (!this._nativeEnable)
			return;
		if (level == GLOG_DEBUG)
			GCanvas._toNative(null, null, 'GCanvas', 'setLogLevel', [ "debug" ]);
		else if (level == GLOG_INFO)
			GCanvas._toNative(null, null, 'GCanvas', 'setLogLevel', [ "info" ]);
		else if (level == GLOG_WARN)
			GCanvas._toNative(null, null, 'GCanvas', 'setLogLevel', [ "warn" ]);
		else if (level == GLOG_ERROR)
			GCanvas._toNative(null, null, 'GCanvas', 'setLogLevel', [ "error" ]);
		else 
			GCanvas._toNative(null, null, 'GCanvas', 'setLogLevel', [ "fatal" ]);	
	     */
	}
	GLog._refresh = function(){

		
		if (this.enable == false){
			this._setNativeLevel(GLOG_NULL);
			this.d = this._nullFunc;
			this.i = this._nullFunc
			this.w = this._nullFunc;
			this.e = this._nullFunc;
		}
		else
		{
			if (this.level <= GLOG_ERROR)
				this.e = function(msg){ console.error(msg);};
			else
				this.e = this._nullFunc;
				
			if (this.level <= GLOG_WARN)
				this.w = function(msg){ console.warn(msg);};
			else
				this.w = this._nullFunc;
			
			if (this.level <= GLOG_INFO)
				this.i = function(msg){ console.info(msg);
					var args = {
						msg:msg
					}
				//WindVane.call("GLog", "writeLog", args || {}, null, null);
			};
			else
				this.i = this._nullFunc;
			
			if (this.level <= GLOG_DEBUG)
				this.d = function(msg){ console.info(msg);
					var args = {
						msg:msg
					}
				//WindVane.call("GLog", "writeLog", args || {}, null, null);
			};
			else
				this.d = this._nullFunc;
			
			this._setNativeLevel(this.level);	
		}
	}
	GLog.enable = function(){
		this.enable = true;
		this._refresh();
	}
	GLog.disable = function(){
		this.enable = false;
		this._refresh();
	}
	GLog.setLevel = function(level){
		console.info("[setLevel] "+ this.level + "=>" + level);
		this.level = level;
		this.enable = true;
		this._refresh();
	}

	//GLog.setLevel(GLOG_WARN);
	GLog.setLevel(GLOG_DEBUG);

	module.exports.GLog = GLog

/***/ }),

/***/ 115:
/***/ (function(module, exports) {

	function GHashMap() {
	    /**Map大小**/
	    var size = 0;
	    /**对象**/
	    var entry = new Object();
	    /**Map的存put方法**/
	    this.put = function(key, value) {
	        if (!this.containsKey(key)) {
	            size++;
	            entry[key] = value;
	        }
	    }
	    /**Map取get方法**/
	    this.get = function(key) {
	        return this.containsKey(key) ? entry[key] : null;
	    }
	    /**Map删除remove方法**/
	    this.remove = function(key) {
	        if (this.containsKey(key) && (delete entry[key])) {
	            size--;
	        }
	    }
	    /**是否包含Key**/
	    this.containsKey = function(key) {
	        return (key in entry);
	    }
	    /**是否包含Value**/
	    this.containsValue = function(value) {
	        for (var prop in entry) {
	            if (entry[prop] == value) {
	                return true;
	            }
	        }
	        return false;
	    }
	    /**所有的Value**/
	    this.values = function() {
	        var values = new Array();
	        for (var prop in entry) {
	            values.push(entry[prop]);
	        }
	        return values;
	    }
	    /**所有的 Key**/
	    this.keys = function() {
	        var keys = new Array();
	        for (var prop in entry) {
	            keys.push(prop);
	        }
	        return keys;
	    }
	    /**Map size**/
	    this.size = function() {
	        return size;
	    }
	    /**清空Map**/
	    this.clear = function() {
	        size = 0;
	        entry = new Object();
	    }
	}

	module.exports=GHashMap;

/***/ }),

/***/ 116:
/***/ (function(module, exports, __webpack_require__) {

	var GBridge = __webpack_require__(113).GBridge;
	var GLog = __webpack_require__(113).GLog;
	var GCanvas = __webpack_require__(111);

	function GContextWebGL(params){
	    GInitWebGLFuncId(this);
	    GInitWebGLEnum(this);
	    GInitWebGLParams(params);

	    this._drawCommands = "";
	    this._globalAlpha = 1.0;
	    this._fillStyle = "rgb(0,0,0)";
	    this._strokeStyle = "rgb(0,0,0)";
	    this._lineWidth = 1;
	    this._lineCap = "butt";
	    this._lineJoin= "miter";
	    this._miterLimit = 10;
	    this._globalCompositeOperation = "source-over";
	    this._textAlign = "start";
	    this._textBaseline = "alphabetic";
	    this._font = "10px sans-serif";
	    this._images = {};
	    this._canvases1 = {};
	    this._canvases2 = {};
	    this._getImageData = new Array();

	    this._uniformMgr = {};
	    this._uniformCount = 3;

	    //GCanvas._forbiddenAutoReplaceCanvas =true;
	    //this._apiCanvas  = document.createElement('canvas');
	    //GCanvas._forbiddenAutoReplaceCanvas =false;
	    //console.log("apicanvas="+this._apiCanvas);
	    //this._apiContext = this._apiCanvas.getContext("2d");
	    //this._apiContext.font = this._font;

	    this._savedGlobalAlpha =[];
	    this.componentId = null;

	    
	}

	function GInitWebGLParams(params) 
	{
	    // if( !params || params.length == 0 )
	    //     return;

	    // var u8ar = Gbase64ToArr(params);
	    // GCanvas._glParams = new Int32Array(u8ar.buffer);

	    // console.log("GInitWebGLParams:"+GCanvas._glParams);
	}

	//////////////////////////////////////////////////////////////////////////

	function GInitWebGLFuncId(obj){
	    var i=1;
	    obj.activeTextureId=(i++)+",";
	    obj.attachShaderId=(i++)+",";
	    obj.bindAttribLocationId=(i++)+",";
	    obj.bindBufferId=(i++)+",";
	    obj.bindFramebufferId=(i++)+",";
	    obj.bindRenderbufferId=(i++)+",";
	    obj.bindTextureId=(i++)+",";
	    obj.blendColorId=(i++)+",";
	    obj.blendEquationId=(i++)+",";
	    obj.blendEquationSeparateId=(i++)+",";
	    obj.blendFuncId=(i++)+",";
	    obj.blendFuncSeparateId=(i++)+",";
	    obj.bufferDataId=(i++)+",";
	    obj.bufferSubDataId=(i++)+",";
	    obj.checkFramebufferStatusId=(i++)+",";
	    obj.clearId=(i++)+",";
	    obj.clearColorId=(i++)+",";
	    obj.clearDepthId=(i++)+",";
	    obj.clearStencilId=(i++)+",";
	    obj.colorMaskId=(i++)+",";
	    obj.compileShaderId=(i++)+",";
	    obj.compressedTexImage2DId=(i++)+",";
	    obj.compressedTexSubImage2DId=(i++)+",";
	    obj.copyTexImage2DId=(i++)+",";
	    obj.copyTexSubImage2DId=(i++)+",";
	    obj.createBufferId=(i++)+",";
	    obj.createFramebufferId=(i++)+",";
	    obj.createProgramId=(i++)+",";
	    obj.createRenderbufferId=(i++)+",";
	    obj.createShaderId=(i++)+",";
	    obj.createTextureId=(i++)+",";
	    obj.cullFaceId=(i++)+",";
	    obj.deleteBufferId=(i++)+",";
	    obj.deleteFramebufferId=(i++)+",";
	    obj.deleteProgramId=(i++)+",";
	    obj.deleteRenderbufferId=(i++)+",";
	    obj.deleteShaderId=(i++)+",";
	    obj.deleteTextureId=(i++)+",";
	    obj.depthFuncId=(i++)+",";
	    obj.depthMaskId=(i++)+",";
	    obj.depthRangeId=(i++)+",";
	    obj.detachShaderId=(i++)+",";
	    obj.disableId=(i++)+",";
	    obj.disableVertexAttribArrayId=(i++)+",";
	    obj.drawArraysId=(i++)+",";
	    obj.drawArraysInstancedANGLEId=(i++)+",";
	    obj.drawElementsId=(i++)+",";
	    obj.drawElementsInstancedANGLEId=(i++)+",";
	    obj.enableId=(i++)+",";
	    obj.enableVertexAttribArrayId=(i++)+",";
	    obj.flushId=(i++)+",";
	    obj.framebufferRenderbufferId=(i++)+",";
	    obj.framebufferTexture2DId=(i++)+",";
	    obj.frontFaceId=(i++)+",";
	    obj.generateMipmapId=(i++)+",";
	    obj.getActiveAttribId=(i++)+",";
	    obj.getActiveUniformId=(i++)+",";
	    obj.getAttachedShadersId=(i++)+",";
	    obj.getAttribLocationId=(i++)+",";
	    obj.getBufferParameterId=(i++)+",";
	    obj.getContextAttributesId=(i++)+",";
	    obj.getErrorId=(i++)+",";
	    obj.getExtensionId=(i++)+",";
	    obj.getFramebufferAttachmentParameterId=(i++)+",";
	    obj.getParameterId=(i++)+",";
	    obj.getProgramInfoLogId=(i++)+",";
	    obj.getProgramParameterId=(i++)+",";
	    obj.getRenderbufferParameterId=(i++)+",";
	    obj.getShaderInfoLogId=(i++)+",";
	    obj.getShaderParameterId=(i++)+",";
	    obj.getShaderPrecisionFormatId=(i++)+",";
	    obj.getShaderSourceId=(i++)+",";
	    obj.getSupportedExtensionsId=(i++)+",";
	    obj.getTexParameterId=(i++)+",";
	    obj.getUniformId=(i++)+",";
	    obj.getUniformLocationId=(i++)+",";
	    obj.getVertexAttribId=(i++)+",";
	    obj.getVertexAttribOffsetId=(i++)+",";
	    obj.isBufferId=(i++)+",";
	    obj.isContextLostId=(i++)+",";
	    obj.isEnabledId=(i++)+",";
	    obj.isFramebufferId=(i++)+",";
	    obj.isProgramId=(i++)+",";
	    obj.isRenderbufferId=(i++)+",";
	    obj.isShaderId=(i++)+",";
	    obj.isTextureId=(i++)+",";
	    obj.lineWidthId=(i++)+",";
	    obj.linkProgramId=(i++)+",";
	    obj.pixelStoreiId=(i++)+",";
	    obj.polygonOffsetId=(i++)+",";
	    obj.readPixelsId=(i++)+",";
	    obj.renderbufferStorageId=(i++)+",";
	    obj.sampleCoverageId=(i++)+",";
	    obj.scissorId=(i++)+",";
	    obj.shaderSourceId=(i++)+",";
	    obj.stencilFuncId=(i++)+",";
	    obj.stencilFuncSeparateId=(i++)+",";
	    obj.stencilMaskId=(i++)+",";
	    obj.stencilMaskSeparateId=(i++)+",";
	    obj.stencilOpId=(i++)+",";
	    obj.stencilOpSeparateId=(i++)+",";
	    obj.texImage2DId=(i++)+",";
	    obj.texParameterfId=(i++)+",";
	    obj.texParameteriId=(i++)+",";
	    obj.texSubImage2DId=(i++)+",";
	    obj.uniform1fId=(i++)+",";
	    obj.uniform1fvId=(i++)+",";
	    obj.uniform1iId=(i++)+",";
	    obj.uniform1ivId=(i++)+",";
	    obj.uniform2fId=(i++)+",";
	    obj.uniform2fvId=(i++)+",";
	    obj.uniform2iId=(i++)+",";
	    obj.uniform2ivId=(i++)+",";
	    obj.uniform3fId=(i++)+",";
	    obj.uniform3fvId=(i++)+",";
	    obj.uniform3iId=(i++)+",";
	    obj.uniform3ivId=(i++)+",";
	    obj.uniform4fId=(i++)+",";
	    obj.uniform4fvId=(i++)+",";
	    obj.uniform4iId=(i++)+",";
	    obj.uniform4ivId=(i++)+",";
	    obj.uniformMatrix2fvId=(i++)+",";
	    obj.uniformMatrix3fvId=(i++)+",";
	    obj.uniformMatrix4fvId=(i++)+",";
	    obj.useProgramId=(i++)+",";
	    obj.validateProgramId=(i++)+",";
	    obj.vertexAttribDivisorANGLEId=(i++)+",";
	    obj.vertexAttrib2fvId=(i++)+",";
	    obj.vertexAttribPointerId=(i++)+",";
	    obj.viewportId=(i++)+",";

	    //extension for OES_vertex_array_object
	    obj.bindVertexArrayOESId=(i++)+",";
	    obj.deleteVertexArraysOESId=(i++)+",";
	    obj.genVertexArraysOESId=(i++)+","
	    obj.isVertexArrayOESId=(i++)+","
	}

	function GInitWebGLEnum(obj){
	    //GL Constant Define
	    obj.NONE = 0x0;
	    obj.ONE = 0x1;
	    obj.LINE_LOOP = 0x2;
	    obj.LINE_STRIP = 0x3;
	    obj.TRIANGLES = 0x4;
	    obj.TRIANGLE_STRIP = 0x5;
	    obj.TRIANGLE_FAN = 0x6;
	    obj.DEPTH_BUFFER_BIT = 0x100;
	    obj.NEVER = 0x200;
	    obj.LESS = 0x201;
	    obj.EQUAL = 0x202;
	    obj.LEQUAL = 0x203;
	    obj.GREATER = 0x204;
	    obj.NOTEQUAL = 0x205;
	    obj.GEQUAL = 0x206;
	    obj.ALWAYS = 0x207;
	    obj.SRC_COLOR = 0x300;
	    obj.ONE_MINUS_SRC_COLOR = 0x301;
	    obj.SRC_ALPHA = 0x302;
	    obj.ONE_MINUS_SRC_ALPHA = 0x303;
	    obj.DST_ALPHA = 0x304;
	    obj.ONE_MINUS_DST_ALPHA = 0x305;
	    obj.DST_COLOR = 0x306;
	    obj.ONE_MINUS_DST_COLOR = 0x307;
	    obj.SRC_ALPHA_SATURATE = 0x308;
	    obj.STENCIL_BUFFER_BIT = 0x400;
	    obj.FRONT = 0x404;
	    obj.BACK = 0x405;
	    obj.FRONT_AND_BACK = 0x408;
	    obj.INVALID_ENUM = 0x500;
	    obj.INVALID_VALUE = 0x501;
	    obj.INVALID_OPERATION = 0x502;
	    obj.OUT_OF_MEMORY = 0x505;
	    obj.INVALID_FRAMEBUFFER_OPERATION = 0x506;
	    obj.CW = 0x900;
	    obj.CCW = 0x901;
	    obj.LINE_WIDTH = 0xB21;
	    obj.CULL_FACE = 0xB44;
	    obj.CULL_FACE_MODE = 0xB45;
	    obj.FRONT_FACE = 0xB46;
	    obj.DEPTH_RANGE = 0xB70;
	    obj.DEPTH_TEST = 0xB71;
	    obj.DEPTH_WRITEMASK = 0xB72;
	    obj.DEPTH_CLEAR_VALUE = 0xB73;
	    obj.DEPTH_FUNC = 0xB74;
	    obj.STENCIL_TEST = 0xB90;
	    obj.STENCIL_CLEAR_VALUE = 0xB91;
	    obj.STENCIL_FUNC = 0xB92;
	    obj.STENCIL_VALUE_MASK = 0xB93;
	    obj.STENCIL_FAIL = 0xB94;
	    obj.STENCIL_PASS_DEPTH_FAIL = 0xB95;
	    obj.STENCIL_PASS_DEPTH_PASS = 0xB96;
	    obj.STENCIL_REF = 0xB97;
	    obj.STENCIL_WRITEMASK = 0xB98;
	    obj.VIEWPORT = 0xBA2;
	    obj.DITHER = 0xBD0;
	    obj.BLEND = 0xBE2;
	    obj.SCISSOR_BOX = 0xC10;
	    obj.SCISSOR_TEST = 0xC11;
	    obj.COLOR_CLEAR_VALUE = 0xC22;
	    obj.COLOR_WRITEMASK = 0xC23;
	    obj.UNPACK_ALIGNMENT = 0xCF5;
	    obj.PACK_ALIGNMENT = 0xD05;
	    obj.MAX_TEXTURE_SIZE = 0xD33;
	    obj.MAX_VIEWPORT_DIMS = 0xD3A;
	    obj.SUBPIXEL_BITS = 0xD50;
	    obj.RED_BITS = 0xD52;
	    obj.GREEN_BITS = 0xD53;
	    obj.BLUE_BITS = 0xD54;
	    obj.ALPHA_BITS = 0xD55;
	    obj.DEPTH_BITS = 0xD56;
	    obj.STENCIL_BITS = 0xD57;
	    obj.TEXTURE_2D = 0xDE1;
	    obj.DONT_CARE = 0x1100;
	    obj.FASTEST = 0x1101;
	    obj.NICEST = 0x1102;
	    obj.BYTE = 0x1400;
	    obj.UNSIGNED_BYTE = 0x1401;
	    obj.SHORT = 0x1402;
	    obj.UNSIGNED_SHORT = 0x1403;
	    obj.INT = 0x1404;
	    obj.UNSIGNED_INT = 0x1405;
	    obj.FLOAT = 0x1406;
	    obj.INVERT = 0x150A;
	    obj.TEXTURE = 0x1702;
	    obj.STENCIL_INDEX = 0x1901;
	    obj.DEPTH_COMPONENT = 0x1902;
	    obj.ALPHA = 0x1906;
	    obj.RGB = 0x1907;
	    obj.RGBA = 0x1908;
	    obj.LUMINANCE = 0x1909;
	    obj.LUMINANCE_ALPHA = 0x190A;
	    obj.KEEP = 0x1E00;
	    obj.REPLACE = 0x1E01;
	    obj.INCR = 0x1E02;
	    obj.DECR = 0x1E03;
	    obj.VENDOR = 0x1F00;
	    obj.RENDERER = 0x1F01;
	    obj.VERSION = 0x1F02;
	    obj.NEAREST = 0x2600;
	    obj.LINEAR = 0x2601;
	    obj.NEAREST_MIPMAP_NEAREST = 0x2700;
	    obj.LINEAR_MIPMAP_NEAREST = 0x2701;
	    obj.NEAREST_MIPMAP_LINEAR = 0x2702;
	    obj.LINEAR_MIPMAP_LINEAR = 0x2703;
	    obj.TEXTURE_MAG_FILTER = 0x2800;
	    obj.TEXTURE_MIN_FILTER = 0x2801;
	    obj.TEXTURE_WRAP_S = 0x2802;
	    obj.TEXTURE_WRAP_T = 0x2803;
	    obj.REPEAT = 0x2901;
	    obj.POLYGON_OFFSET_UNITS = 0x2A00;
	    obj.COLOR_BUFFER_BIT = 0x4000;
	    obj.CONSTANT_COLOR = 0x8001;
	    obj.ONE_MINUS_CONSTANT_COLOR = 0x8002;
	    obj.CONSTANT_ALPHA = 0x8003;
	    obj.ONE_MINUS_CONSTANT_ALPHA = 0x8004;
	    obj.BLEND_COLOR = 0x8005;
	    obj.FUNC_ADD = 0x8006;
	    obj.BLEND_EQUATION_RGB = 0x8009;
	    obj.FUNC_SUBTRACT = 0x800A;
	    obj.FUNC_REVERSE_SUBTRACT = 0x800B;
	    obj.UNSIGNED_SHORT_4_4_4_4 = 0x8033;
	    obj.UNSIGNED_SHORT_5_5_5_1 = 0x8034;
	    obj.POLYGON_OFFSET_FILL = 0x8037;
	    obj.POLYGON_OFFSET_FACTOR = 0x8038;
	    obj.RGBA4 = 0x8056;
	    obj.RGB5_A1 = 0x8057;
	    obj.TEXTURE_BINDING_2D = 0x8069;
	    obj.SAMPLE_ALPHA_TO_COVERAGE = 0x809E;
	    obj.SAMPLE_COVERAGE = 0x80A0;
	    obj.SAMPLE_BUFFERS = 0x80A8;
	    obj.SAMPLES = 0x80A9;
	    obj.SAMPLE_COVERAGE_VALUE = 0x80AA;
	    obj.SAMPLE_COVERAGE_INVERT = 0x80AB;
	    obj.BLEND_DST_RGB = 0x80C8;
	    obj.BLEND_SRC_RGB = 0x80C9;
	    obj.BLEND_DST_ALPHA = 0x80CA;
	    obj.BLEND_SRC_ALPHA = 0x80CB;
	    obj.CLAMP_TO_EDGE = 0x812F;
	    obj.GENERATE_MIPMAP_HINT = 0x8192;
	    obj.DEPTH_COMPONENT16 = 0x81A5;
	    obj.DEPTH_STENCIL_ATTACHMENT = 0x821A;
	    obj.UNSIGNED_SHORT_5_6_5 = 0x8363;
	    obj.MIRRORED_REPEAT = 0x8370;
	    obj.ALIASED_POINT_SIZE_RANGE = 0x846D;
	    obj.ALIASED_LINE_WIDTH_RANGE = 0x846E;
	    obj.TEXTURE0 = 0x84C0;
	    obj.TEXTURE1 = 0x84C1;
	    obj.TEXTURE2 = 0x84C2;
	    obj.TEXTURE3 = 0x84C3;
	    obj.TEXTURE4 = 0x84C4;
	    obj.TEXTURE5 = 0x84C5;
	    obj.TEXTURE6 = 0x84C6;
	    obj.TEXTURE7 = 0x84C7;
	    obj.TEXTURE8 = 0x84C8;
	    obj.TEXTURE9 = 0x84C9;
	    obj.TEXTURE10 = 0x84CA;
	    obj.TEXTURE11 = 0x84CB;
	    obj.TEXTURE12 = 0x84CC;
	    obj.TEXTURE13 = 0x84CD;
	    obj.TEXTURE14 = 0x84CE;
	    obj.TEXTURE15 = 0x84CF;
	    obj.TEXTURE16 = 0x84D0;
	    obj.TEXTURE17 = 0x84D1;
	    obj.TEXTURE18 = 0x84D2;
	    obj.TEXTURE19 = 0x84D3;
	    obj.TEXTURE20 = 0x84D4;
	    obj.TEXTURE21 = 0x84D5;
	    obj.TEXTURE22 = 0x84D6;
	    obj.TEXTURE23 = 0x84D7;
	    obj.TEXTURE24 = 0x84D8;
	    obj.TEXTURE25 = 0x84D9;
	    obj.TEXTURE26 = 0x84DA;
	    obj.TEXTURE27 = 0x84DB;
	    obj.TEXTURE28 = 0x84DC;
	    obj.TEXTURE29 = 0x84DD;
	    obj.TEXTURE30 = 0x84DE;
	    obj.TEXTURE31 = 0x84DF;
	    obj.ACTIVE_TEXTURE = 0x84E0;
	    obj.MAX_RENDERBUFFER_SIZE = 0x84E8;
	    obj.DEPTH_STENCIL = 0x84F9;
	    obj.INCR_WRAP = 0x8507;
	    obj.DECR_WRAP = 0x8508;
	    obj.TEXTURE_CUBE_MAP = 0x8513;
	    obj.TEXTURE_BINDING_CUBE_MAP = 0x8514;
	    obj.TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515;
	    obj.TEXTURE_CUBE_MAP_NEGATIVE_X = 0x8516;
	    obj.TEXTURE_CUBE_MAP_POSITIVE_Y = 0x8517;
	    obj.TEXTURE_CUBE_MAP_NEGATIVE_Y = 0x8518;
	    obj.TEXTURE_CUBE_MAP_POSITIVE_Z = 0x8519;
	    obj.TEXTURE_CUBE_MAP_NEGATIVE_Z = 0x851A;
	    obj.MAX_CUBE_MAP_TEXTURE_SIZE = 0x851C;
	    obj.VERTEX_ATTRIB_ARRAY_ENABLED = 0x8622;
	    obj.VERTEX_ATTRIB_ARRAY_SIZE = 0x8623;
	    obj.VERTEX_ATTRIB_ARRAY_STRIDE = 0x8624;
	    obj.VERTEX_ATTRIB_ARRAY_TYPE = 0x8625;
	    obj.CURRENT_VERTEX_ATTRIB = 0x8626;
	    obj.VERTEX_ATTRIB_ARRAY_POINTER = 0x8645;
	    obj.NUM_COMPRESSED_TEXTURE_FORMATS = 0x86A2;
	    obj.COMPRESSED_TEXTURE_FORMATS = 0x86A3;
	    obj.BUFFER_SIZE = 0x8764;
	    obj.BUFFER_USAGE = 0x8765;
	    obj.STENCIL_BACK_FUNC = 0x8800;
	    obj.STENCIL_BACK_FAIL = 0x8801;
	    obj.STENCIL_BACK_PASS_DEPTH_FAIL = 0x8802;
	    obj.STENCIL_BACK_PASS_DEPTH_PASS = 0x8803;
	    obj.BLEND_EQUATION_ALPHA = 0x883D;
	    obj.MAX_VERTEX_ATTRIBS = 0x8869;
	    obj.VERTEX_ATTRIB_ARRAY_NORMALIZED = 0x886A;
	    obj.MAX_TEXTURE_IMAGE_UNITS = 0x8872;
	    obj.ARRAY_BUFFER = 0x8892;
	    obj.ELEMENT_ARRAY_BUFFER = 0x8893;
	    obj.ARRAY_BUFFER_BINDING = 0x8894;
	    obj.ELEMENT_ARRAY_BUFFER_BINDING = 0x8895;
	    obj.VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F;
	    obj.STREAM_DRAW = 0x88E0;
	    obj.STATIC_DRAW = 0x88E4;
	    obj.DYNAMIC_DRAW = 0x88E8;
	    obj.FRAGMENT_SHADER = 0x8B30;
	    obj.VERTEX_SHADER = 0x8B31;
	    obj.MAX_VERTEX_TEXTURE_IMAGE_UNITS = 0x8B4C;
	    obj.MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D;
	    obj.SHADER_TYPE = 0x8B4F;
	    obj.FLOAT_VEC2 = 0x8B50;
	    obj.FLOAT_VEC3 = 0x8B51;
	    obj.FLOAT_VEC4 = 0x8B52;
	    obj.INT_VEC2 = 0x8B53;
	    obj.INT_VEC3 = 0x8B54;
	    obj.INT_VEC4 = 0x8B55;
	    obj.BOOL = 0x8B56;
	    obj.BOOL_VEC2 = 0x8B57;
	    obj.BOOL_VEC3 = 0x8B58;
	    obj.BOOL_VEC4 = 0x8B59;
	    obj.FLOAT_MAT2 = 0x8B5A;
	    obj.FLOAT_MAT3 = 0x8B5B;
	    obj.FLOAT_MAT4 = 0x8B5C;
	    obj.SAMPLER_2D = 0x8B5E;
	    obj.SAMPLER_CUBE = 0x8B60;
	    obj.DELETE_STATUS = 0x8B80;
	    obj.COMPILE_STATUS = 0x8B81;
	    obj.LINK_STATUS = 0x8B82;
	    obj.VALIDATE_STATUS = 0x8B83;
	    obj.INFO_LOG_LENGTH = 0x8B84;
	    obj.ATTACHED_SHADERS = 0x8B85;
	    obj.ACTIVE_UNIFORMS = 0x8B86;
	    obj.ACTIVE_UNIFORM_MAX_LENGTH = 0x8B87;
	    obj.SHADER_SOURCE_LENGTH = 0x8B88;
	    obj.ACTIVE_ATTRIBUTES = 0x8B89;
	    obj.ACTIVE_ATTRIBUTE_MAX_LENGTH = 0x8B8A;
	    obj.SHADING_LANGUAGE_VERSION = 0x8B8C;
	    obj.CURRENT_PROGRAM = 0x8B8D;
	    obj.STENCIL_BACK_REF = 0x8CA3;
	    obj.STENCIL_BACK_VALUE_MASK = 0x8CA4;
	    obj.STENCIL_BACK_WRITEMASK = 0x8CA5;
	    obj.FRAMEBUFFER_BINDING = 0x8CA6;
	    obj.RENDERBUFFER_BINDING = 0x8CA7;
	    obj.FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE = 0x8CD0;
	    obj.FRAMEBUFFER_ATTACHMENT_OBJECT_NAME = 0x8CD1;
	    obj.FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL = 0x8CD2;
	    obj.FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8CD3;
	    obj.FRAMEBUFFER_COMPLETE = 0x8CD5;
	    obj.FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6;
	    obj.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7;
	    obj.FRAMEBUFFER_INCOMPLETE_DIMENSIONS = 0x8CD9;
	    obj.FRAMEBUFFER_UNSUPPORTED = 0x8CDD;
	    obj.COLOR_ATTACHMENT0 = 0x8CE0;
	    obj.DEPTH_ATTACHMENT = 0x8D00;
	    obj.STENCIL_ATTACHMENT = 0x8D20;
	    obj.FRAMEBUFFER = 0x8D40;
	    obj.RENDERBUFFER = 0x8D41;
	    obj.RENDERBUFFER_WIDTH = 0x8D42;
	    obj.RENDERBUFFER_HEIGHT = 0x8D43;
	    obj.RENDERBUFFER_INTERNAL_FORMAT = 0x8D44;
	    obj.STENCIL_INDEX8 = 0x8D48;
	    obj.RENDERBUFFER_RED_SIZE = 0x8D50;
	    obj.RENDERBUFFER_GREEN_SIZE = 0x8D51;
	    obj.RENDERBUFFER_BLUE_SIZE = 0x8D52;
	    obj.RENDERBUFFER_ALPHA_SIZE = 0x8D53;
	    obj.RENDERBUFFER_DEPTH_SIZE = 0x8D54;
	    obj.RENDERBUFFER_STENCIL_SIZE = 0x8D55;
	    obj.RGB565 = 0x8D62;
	    obj.LOW_FLOAT = 0x8DF0;
	    obj.MEDIUM_FLOAT = 0x8DF1;
	    obj.HIGH_FLOAT = 0x8DF2;
	    obj.LOW_INT = 0x8DF3;
	    obj.MEDIUM_INT = 0x8DF4;
	    obj.HIGH_INT = 0x8DF5;
	    obj.SHADER_COMPILER = 0x8DFA;
	    obj.MAX_VERTEX_UNIFORM_VECTORS = 0x8DFB;
	    obj.MAX_VARYING_VECTORS = 0x8DFC;
	    obj.MAX_FRAGMENT_UNIFORM_VECTORS = 0x8DFD;

	    obj.UNPACK_FLIP_Y_WEBGL = 0x9240;
	    // obj.UNPACK_PREMULTIPLY_ALPHA_WEBGL = 0x9241;
	    // obj.CONTEXT_LOST_WEBGL = 0x9242;
	    // obj.UNPACK_COLORSPACE_CONVERSION_WEBGL = 0x9243;
	    // obj.BROWSER_DEFAULT_WEBGL = 0x9244;

	    //extsion
	    obj.OES_vertex_array_object = 1;
	    obj.OES_texture_float = 1;
	    obj.OES_element_index_uint = 1;
	}

	function GAttribLocation() {
	    this.id = (++GAttribLocation.idCounter);
	}
	GAttribLocation.idCounter = 0;
	GAttribLocation.mapKey = [];
	GAttribLocation.mapVal = [];


	function GProgram() {
	    this.id = (++GProgram.idCounter);
	}
	GProgram.idCounter = 0;

	function GShader() {
	    this.id = (++GShader.idCounter);
	}
	GShader.idCounter = 0;

	function GTexture() {
	    this.id = (++GTexture.idCounter);
	}
	GTexture.idCounter = 0;

	function GUniformLocation() {
	    this.id = (++GUniformLocation.idCounter);
	}
	GUniformLocation.idCounter = 0;
	GUniformLocation.mapKey = [];
	GUniformLocation.mapVal = [];

	function GWebGLBuffer() {
	    this.id = (++GWebGLBuffer.idCounter);
	}
	GWebGLBuffer.idCounter = 0;

	function GWebGLFramebuffer() {
	    this.id = (++GWebGLFramebuffer.idCounter);
	}
	GWebGLFramebuffer.idCounter = 0;

	function GWebGLRenderbuffer() {
	    this.id = (++GWebGLRenderbuffer.idCounter);
	}
	GWebGLRenderbuffer.idCounter = 0;

	function GWebGLShaderPrecisionFormat(){
	}



	function GarrToBase64(array) 
	{   
	    var str = '';
	    if( array.join === 'function' )
	    {
	        str = array.join();
	    }
	    else
	    {
	        for (var i = 0; i < array.length; i++) 
	        {
	            if( i < array.length - 1 )
	            {
	                str = str + array[i] + ',';
	            }
	            else
	            {
	                str = str + array[i];
	            }
	        }
	    }
	    // GLog.d("GarrToBase64(), before: "+ str);
	    // GLog.d("GarrToBase64(), after : "+ btoa(str));
	    return btoa(str);
	}

	function Gbase64ToArr(base64)
	{
	    // GLog.d("base64:" + base64);
	    var binary_string = atob(base64);
	    // GLog.d("binary_string:" + binary_string);
	    var array = binary_string.slice();
	    return array;
	    // var len = binary_string.length;
	    // var bytes = new Uint8Array( len );
	    // for (var i = 0; i < len; i++)        {
	    //     bytes[i] = binary_string.charCodeAt(i);
	    // }
	    // return bytes.buffer;
	}
	//////////////////////////////////////////////////////////////////////////



	GContextWebGL.prototype.render = function() {
	    var commands = this._drawCommands;
	    this._drawCommands = "";
	    if (commands != null && commands != "") {
	        // GLog.d("GContextWebGL#render() called, commands is "+ commands);
	        //GCanvas._toNative(null, null, 'GCanvas', 'render', [ commands ]);
	        GBridge.callRender(this.componentId, commands)
	    }
	};


	//////////////////////////////////////////////////////////////////////////

	////////////////////////////// WEBGL API //////////////////////////////////

	GContextWebGL.prototype.activeTexture = function(texture){
	    this._drawCommands += (this.activeTextureId + texture + ";");
	};

	GContextWebGL.prototype.attachShader = function(program, shader){
	    this._drawCommands += (this.attachShaderId + program.id + "," + shader.id + ";");
	};

	GContextWebGL.prototype.bindAttribLocation = function(program, index, name){
	    this._drawCommands += (this.bindAttribLocationId + program.id + "," + index + "," + name + ";");
	};

	GContextWebGL.prototype.bindBuffer = function(target, buffer){
	    this._drawCommands += (this.bindBufferId + target + "," + ((null == buffer)?-1:buffer.id) + ";");
	};

	GContextWebGL.prototype.bindFramebuffer = function(target, buf){
	    this._drawCommands += (this.bindFramebufferId + target + "," + ((null == buf)?-1:buf.id) + ";");
	};

	GContextWebGL.prototype.bindRenderbuffer = function(target, buf){
	    this._drawCommands += (this.bindRenderbufferId + target + "," + ((null == buf)?-1:buf.id) + ";");
	};

	GContextWebGL.prototype.bindTexture = function(target, texture){
	    this._drawCommands += (this.bindTextureId + target + "," + ((null == texture)?-1:texture.id) + ";");
	};

	GContextWebGL.prototype.blendEquation = function(mode){
	    this._drawCommands += (this.blendEquationId + mode + ";");
	};

	GContextWebGL.prototype.blendEquationSeparate = function(modeRGB, modeAlpha){
	    this._drawCommands += (this.blendEquationSeparateId + modeRGB + "," + modeAlpha  + ";");
	};


	GContextWebGL.prototype.blendFunc = function(sfactor, dfactor){
	    this._drawCommands += (this.blendFuncId + sfactor + "," + dfactor  + ";");
	};

	GContextWebGL.prototype.blendFuncSeparate = function(srcRGB, dstRGB, srcAlpha, dstAlpha){
	    this._drawCommands += (this.blendFuncSeparateId + srcRGB + "," + dstRGB + "," + srcAlpha + "," + dstAlpha  + ";");
	};

	GContextWebGL.prototype.bufferData = function(target, array, usage){
	    // GLog.d("[bufferData] before:_drawCommands.length=" + this._drawCommands.length);
	    // GLog.d("[bufferData] array.length=" + array.length);
	    this._drawCommands += (this.bufferDataId + target + "," + array.BYTES_PER_ELEMENT + "," + GarrToBase64(array) + "," + usage + ";");
	    // GLog.d("[bufferData] after :_drawCommands.length=" + this._drawCommands.length);
	};

	GContextWebGL.prototype.checkFramebufferStatus_ = function(target){
	    return this.FRAMEBUFFER_COMPLETE;// TODO:
	};

	GContextWebGL.prototype.clear = function(mask){
	    this._drawCommands += (this.clearId + mask + ";");
	};

	GContextWebGL.prototype.clearColor = function(r, g, b, a){
	    this._drawCommands += (this.clearColorId + r + "," + g + "," + b + "," + a + ";");
	};

	GContextWebGL.prototype.clearDepth = function(depth){
	    this._drawCommands += (this.clearDepthId + depth + ";");
	};

	GContextWebGL.prototype.clearStencil = function(s){
	    this._drawCommands += (this.clearStencilId + s + ";");
	};

	GContextWebGL.prototype.colorMask = function(r, g, b, a){
	    this._drawCommands += (this.colorMaskId + r?1: true?1:0 + "," + b?1:0 + "," + a?1:0 + ";");
	};

	GContextWebGL.prototype.compileShader = function(shader) {
	    this._drawCommands += (this.compileShaderId + shader.id + ";");
	};

	GContextWebGL.prototype.createBuffer = function(){
	    var buffer = new GWebGLBuffer();
	    this._drawCommands += (this.createBufferId + buffer.id + ";");
	    return buffer;
	};

	GContextWebGL.prototype.createFramebuffer = function(){
	    var framebuffer = new GWebGLFramebuffer();
	    this._drawCommands += (this.createFramebufferId + framebuffer.id + ";");
	    return framebuffer;
	};

	GContextWebGL.prototype.createRenderbuffer = function(){
	    var renderbuffer= new GWebGLRenderbuffer();
	    this._drawCommands += (this.createRenderbufferId + renderbuffer.id + ";");
	    return renderbuffer;
	};

	GContextWebGL.prototype.createProgram = function(){
	    var program = new GProgram();
	    this._drawCommands += (this.createProgramId + program.id + ";");
	    return program;
	};

	GContextWebGL.prototype.createShader = function(type) {
	    var shader = new GShader();
	    this._drawCommands += (this.createShaderId + shader.id + "," + type + ";");
	    return shader;
	};

	GContextWebGL.prototype.createTexture = function() {
	    var texture = new GTexture();
	    this._drawCommands += (this.createTextureId + texture.id + ";");
	    return texture;
	};

	GContextWebGL.prototype.cullFace = function(mode){
	    this._drawCommands += (this.cullFaceId + mode + ";");
	};

	GContextWebGL.prototype.deleteBuffer = function(buffer){
	    this._drawCommands += (this.deleteBufferId + buffer.id + ";");
	};

	GContextWebGL.prototype.deleteFramebuffer = function(framebuffer){
	    this._drawCommands += (this.deleteFramebufferId + framebuffer.id + ";");
	};

	GContextWebGL.prototype.deleteProgram = function(program){
	    this._drawCommands += (this.deleteProgramId + program.id + ";");
	};

	GContextWebGL.prototype.deleteRenderbuffer = function(renderbuffer){
	    this._drawCommands += (this.deleteRenderbufferId + renderbuffer.id + ";");
	};

	GContextWebGL.prototype.deleteShader = function(shader){
	    this._drawCommands += (this.deleteShaderId + shader.id + ";");
	};

	GContextWebGL.prototype.deleteTexture = function(texture){
	    this._drawCommands += (this.deleteTextureId + texture.id + ";");
	};

	GContextWebGL.prototype.depthFunc = function(func){
	    this._drawCommands += (this.depthFuncId + func + ";");
	};

	GContextWebGL.prototype.depthMask = function(flag){
	    this._drawCommands += (this.depthMaskId + (flag?1:0) + ";");
	};

	GContextWebGL.prototype.depthRange = function(near, far){
	    this._drawCommands += (this.depthRangeId + near + ", " + far + ";");
	}

	GContextWebGL.prototype.disable = function(cap){
	    this._drawCommands += (this.disableId + cap + ";");
	};

	GContextWebGL.prototype.disableVertexAttribArray = function(index){
	    this._drawCommands += (this.disableVertexAttribArrayId + index + ";");
	};

	GContextWebGL.prototype.drawArrays = function(mode, first, count){
	    this._drawCommands += (this.drawArraysId + mode + "," + first + "," + count +  ";");
	};

	GContextWebGL.prototype.drawElements = function(mode, count, type, offset){
	    this._drawCommands += (this.drawElementsId + mode + "," + count + "," + type + "," + offset +  ";");
	};

	GContextWebGL.prototype.enable = function(cap){
	    this._drawCommands += (this.enableId + cap + ";");
	};

	GContextWebGL.prototype.enableVertexAttribArray = function(index){
	    this._drawCommands += (this.enableVertexAttribArrayId + index + ";");
	};

	GContextWebGL.prototype.flush = function(){
	    this._drawCommands += (this.flushId + ";");
	};

	GContextWebGL.prototype.generateMipmap = function(target){
	    this._drawCommands += (this.generateMipmapId + target + ";");
	};

	GContextWebGL.prototype.framebufferRenderbuffer = function(target, attachment, renderbuffertarget, renderbuffer){
	    this._drawCommands += (this.framebufferRenderbufferId + target + "," + attachment + "," + renderbuffertarget + "," + renderbuffer.id + ";");
	};

	GContextWebGL.prototype.framebufferTexture2D = function(target, attachment, textarget, texture, level){
	    this._drawCommands += (this.framebufferTexture2DId + target + "," + attachment + "," + textarget + "," + texture.id + "," + level + ";");
	};

	GContextWebGL.prototype.frontFace = function(mode){
	    this._drawCommands += (this.frontFaceId + mode + ";");
	};

	GContextWebGL.prototype.getAttribLocation   = function(program, name) {
	    var key = program.id + ":" + name;
	    var index = GAttribLocation.mapKey.indexOf(key);
	    if (index > -1){
	        //GLog.d("[getAttribLocation] " + key + "=" + GAttribLocation.mapVal[index].id);
	        return GAttribLocation.mapVal[index];
	    }

	    var id = GAttribLocation.idCounter++;
	    this._drawCommands += (this.getAttribLocationId + program.id + "," + name + "," + id + ";");

	    GAttribLocation.mapKey.push(key);
	    GAttribLocation.mapVal.push(id);
	    //GLog.d("[GAttribLocation] " + key + "=" + id);

	    return id;
	};

	GContextWebGL.prototype.getExtension = function(name) {
	    //GLog.w("[getExtension] " + name);
	    return null;
	//  var ret = new Object();
	//  return ret;// TODO: need call opengl es
	};


	//TODO
	GContextWebGL.prototype.getParameter = function(name) {
	    if (typeof(GCanvas._glParams) == 'undefined'){
	        return null;
	    }
	    if (name == this.ALIASED_POINT_SIZE_RANGE){
	        var f32ar =  new Float32Array(2);
	        f32ar[0]= 1;
	        f32ar[1]= 256;
	        return f32ar;
	    }
	    //GLog.d("[GContextWebGL::getParameter] " + name + "=" + GCanvas._glParams[name]);
	    return GCanvas._glParams[name];
	    // TODO: unsupport this code:var _compressedTextureFormats = _glExtensionCompressedTextureS3TC ? _gl.getParameter( _gl.COMPRESSED_TEXTURE_FORMATS ) : [];
	};

	GContextWebGL.prototype.getProgramInfoLog = function(program){
	    return ""; // TODO:need asyn deal
	};

	GContextWebGL.prototype.getProgramParameter = function(id, type){
	    var args = id + ',' + type;
	    var result = GBridge.exeSyncCmd('getProgramParameter',args);
	    
	    // return result;
	    return "";
	};

	GContextWebGL.prototype.getShaderInfoLog = function(id){
	    var args = id;
	    var result = GBridge.exeSyncCmd('getShaderInfoLog',args);
	    
	    // return result;
	    return "";
	};

	GContextWebGL.prototype.getShaderParameter = function(shader, pname){
	    var args = shader + ',' + pname;
	    var result = GBridge.exeSyncCmd('getShaderParameter',args);
	    
	    // return result;
	    return "";
	};

	GContextWebGL.prototype.getActiveUniform = function(id, index){
	    var args = id + ',' + index;
	    var result = GBridge.exeSyncCmd('getActiveUniform',args);
	    var tmp = result.split(',');
	    return {
	        type: tmp[0],
	        name: tmp[1]
	    };
	}

	GContextWebGL.prototype.getActiveAttrib = function(id, index){
	    var args = id + ',' + index;
	    var result = GBridge.exeSyncCmd('getActiveAttrib',args);
	    var tmp = result.split(',');
	    return {
	        type: tmp[0],
	        name: tmp[1]
	    };
	}

	GContextWebGL.prototype.scissor = function(x, y, w, h) {
	    this._drawCommands += (this.scissorId + x + "," + y + ","
	    + w+ "," + h + ";");
	};


	GContextWebGL.prototype.getShaderPrecisionFormat = function(shader, pname){
	    //var ret = new GWebGLShaderPrecisionFormat();
	    //ret.range = 127;
	    //ret.precision = 23;
	    //if (shader == gl.VERTEX_SHADER) {
	    //    switch (pname) {
	    //        case gl.LOW_FLOAT:
	    //        {
	    //            ret.range = GCanvas._glParams[0];
	    //            ret.precision = GCanvas._glParams[1];
	    //            break;
	    //        }
	    //        case gl.MEDIUM_FLOAT:
	    //        {
	    //            ret.range = GCanvas._glParams[2];
	    //            ret.precision = GCanvas._glParams[3];
	    //            break;
	    //        }
	    //        case gl.HIGH_FLOAT:
	    //        {
	    //            ret.range = GCanvas._glParams[4];
	    //            ret.precision = GCanvas._glParams[5];
	    //            break;
	    //        }
	    //        case gl.LOW_INT:
	    //        {
	    //            ret.range = GCanvas._glParams[6];
	    //            ret.precision = GCanvas._glParams[7];
	    //            break;
	    //        }
	    //        case gl.MEDIUM_INT:
	    //        {
	    //            ret.range = GCanvas._glParams[8];
	    //            ret.precision = GCanvas._glParams[9];
	    //            break;
	    //        }
	    //        case gl.HIGH_INT:
	    //        {
	    //            ret.range = GCanvas._glParams[10];
	    //            ret.precision = GCanvas._glParams[11];
	    //            break;
	    //        }
	    //
	    //    } //end switch
	    //}
	    //if (shader == gl.FRAGMENT_SHADER) {
	    //    switch (pname) {
	    //        case gl.LOW_FLOAT:
	    //        {
	    //            ret.range = GCanvas._glParams[12];
	    //            ret.precision = GCanvas._glParams[13];
	    //            break;
	    //        }
	    //        case gl.MEDIUM_FLOAT:
	    //        {
	    //            ret.range = GCanvas._glParams[14];
	    //            ret.precision = GCanvas._glParams[15];
	    //            break;
	    //        }
	    //        case gl.HIGH_FLOAT:
	    //        {
	    //            ret.range = GCanvas._glParams[16];
	    //            ret.precision = GCanvas._glParams[17];
	    //            break;
	    //        }
	    //        case gl.LOW_INT:
	    //        {
	    //            ret.range = GCanvas._glParams[18];
	    //            ret.precision = GCanvas._glParams[19];
	    //            break;
	    //        }
	    //        case gl.MEDIUM_INT:
	    //        {
	    //            ret.range = GCanvas._glParams[20];
	    //            ret.precision = GCanvas._glParams[21];
	    //            break;
	    //        }
	    //        case gl.HIGH_INT:
	    //        {
	    //            ret.range = GCanvas._glParams[22];
	    //            ret.precision = GCanvas._glParams[23];
	    //            break;
	    //        }
	    //
	    //    } //end switch
	    //}
	    //
	    //return ret;
	};

	GContextWebGL.prototype.getUniformLocation = function(program, name) {

	    var key = program.id + ":" + name;
	    var index = GUniformLocation.mapKey.indexOf(key);
	    if (index > -1){
	        //GLog.d("[getUniformLocation] " + key + "=" + GUniformLocation.mapVal[index].id);
	        return GUniformLocation.mapVal[index];
	    }

	    var uniform = new GUniformLocation();
	    this._drawCommands += (this.getUniformLocationId + program.id + "," + name + "," + uniform.id + ";");

	    GUniformLocation.mapKey.push(key);
	    GUniformLocation.mapVal.push(uniform);
	    //GLog.d("[getUniformLocation] " + key + "=" + uniform.id);
	    return uniform;
	};

	GContextWebGL.prototype.isContextLost = function(){
	    return false;
	};
	GContextWebGL.prototype.lineWidth = function(width){
	    this._drawCommands += (this.lineWidthId + width + ";");
	};

	GContextWebGL.prototype.linkProgram = function(program){
	    this._drawCommands += (this.linkProgramId + program.id + ";");
	};

	GContextWebGL.prototype.pixelStorei = function(pname, param){
	    if (true == param)
	        param = 1;
	    else if (false == param)
	        param = 0;
	    this._drawCommands += (this.pixelStoreiId + pname + "," + param + ";");
	};

	GContextWebGL.prototype.shaderSource = function(shader, source){
	    this._drawCommands += (this.shaderSourceId + shader.id + "," + btoa(source) + ";");
	};



	GContextWebGL.prototype.renderbufferStorage = function(target, internalformat, width, height){
	    this._drawCommands += (this.renderbufferStorageId + target + "," + internalformat + "," + width + "," + height + ";");
	};




	//texImage2D(webgl.TEXTURE_2D, 0, webgl.RGB, webgl.RGB, webgl.UNSIGNED_BYTE, img);
	//WebGLRenderingContext.texImage2D(target, level, internalformat, width, height, border, format, type, pixels);
	//texImage2D( _gl.TEXTURE_2D, 0, _gl.RGB, 16, 16, 0, _gl.RGB, _gl.UNSIGNED_BYTE, null );
	GContextWebGL.prototype.texImage2D = function(target, level, internalformat){
	    var argc = arguments.length;
	    //GLog.d("[texImage2D]arguments.length=" + argc);

	    if (6==argc){
	        var image = arguments[5];
	        var imgData;
	        // if (image instanceof HTMLCanvasElement){
	        //     imgData = image.toDataURL("image/jpeg");
	        // }else{
	            imgData = image.src;
	        // }
	        this._drawCommands += (this.texImage2DId + argc + "," + target + "," + level
	        + "," + internalformat + "," + arguments[3] + "," + arguments[4]
	        + "," + imgData + ";");
	        GLog.d("[texImage2D] finish..." + imgData.length);
	    }else if (9==argc){
	        var image = arguments[5];
	        this._drawCommands += (this.texImage2DId + argc + "," + target + "," + level
	        + "," + internalformat + "," + arguments[3] + "," + arguments[4]
	        + "," + arguments[5] + "," + arguments[6] + "," + arguments[7]
	        + "," + arguments[8] + ";");
	    }
	};

	GContextWebGL.prototype.texParameteri = function(target, pname, param){
	    this._drawCommands += (this.texParameteriId + target + "," + pname + "," + param + ";");
	};


	GContextWebGL.prototype.uniform1f = function(location, value){
	    this._drawCommands += (this.uniform1fId + location.id + "," + value + ";");
	};

	function trans2ArrayType(type, ar){
	    if (ar instanceof type)
	        return ar;
	    var len = ar.length;
	    var f32ar = new type(len);
	    for (var i = 0; i < len; i++) {
	        f32ar[i]=ar[i];
	    }
	    return f32ar;
	}

	GContextWebGL.prototype.uniformXXv = function(id, value, type, cmd){
	    if (value.length == 0)
	        return;

	    value = trans2ArrayType(type, value);
	    this._drawCommands += (cmd + id + ","
	    + GarrToBase64(value) + ";");
	};

	GContextWebGL.prototype.uniform1f = function(location, value){
	    this._drawCommands += (this.uniform1fId  + location.id + "," + value + ";");
	};

	GContextWebGL.prototype.uniform1fv = function(location, value){
	    this.uniformXXv(location.id, value, Float32Array, this.uniform1fvId );
	};

	GContextWebGL.prototype.uniform1i = function(location, value){
	    if (true == value)
	        value= 1;
	    else if (false == value)
	        value = 0;
	    this._drawCommands += (this.uniform1iId + location.id + "," + value + ";");
	};

	GContextWebGL.prototype.uniform1iv = function(location, value){
	    this.uniformXXv(location.id, value, Int32Array, this.uniform1ivId );
	};

	GContextWebGL.prototype.uniform2f = function(location, x, y){
	    this._drawCommands += (this.uniform2fId  + location.id + "," + x + "," + y + ";");
	};

	GContextWebGL.prototype.uniform2fv = function(location, value){
	    this.uniformXXv(location.id, value, Float32Array, this.uniform2fvId);
	};

	GContextWebGL.prototype.uniform2i = function(location, x, y){
	    this._drawCommands += (this.uniform2iId + location.id + "," + x + "," + y + ";");
	};

	GContextWebGL.prototype.uniform2iv = function(location, value){
	    this.uniformXXv(location.id, value, Int32Array, this.uniform2ivId );
	};

	GContextWebGL.prototype.uniform3f = function(location, x, y, z){
	    this._drawCommands += (this.uniform3fId + location.id + "," + x + "," + y + "," + z + ";");
	};

	GContextWebGL.prototype.uniform3fv = function(location, value){
	    this.uniformXXv(location.id, value, Float32Array, this.uniform3fvId);
	};

	GContextWebGL.prototype.uniform3i = function(location, x, y, z){
	    this._drawCommands += (this.uniform3iId + location.id + "," + x + "," + y + "," + z + ";");
	};

	GContextWebGL.prototype.uniform3iv = function(location, value){
	    this.uniformXXv(location.id, value, Int32Array, this.uniform3ivId );
	};

	GContextWebGL.prototype.uniform4f = function(location, x, y, z, w){
	    this._drawCommands += (this.uniform4fId + location.id + "," + x + "," + y + "," + z + "," + w + ";");
	};

	GContextWebGL.prototype.uniform4fv = function(location, value){
	    this.uniformXXv(location.id, value, Float32Array, this.uniform4fvId);
	};

	GContextWebGL.prototype.uniform4i = function(location, x, y, z, w){
	    this._drawCommands += (this.uniform4iId + location.id + "," + x + "," + y + "," + z + "," + w + ";");
	};

	GContextWebGL.prototype.uniform4iv = function(location, value){
	    this.uniformXXv(location.id, value, Int32Array, this.uniform4ivId );
	};



	GContextWebGL.prototype.uniformMatrixXfv = function(location, transpose, value, apiId){
	    if (value.length == 0)
	        return;
	    this._drawCommands += (apiId + location.id + "," + (transpose?1:0));
	    this._drawCommands += "," + GarrToBase64(value) + (";");
	};


	GContextWebGL.prototype.uniformMatrix2fv = function(location, transpose, value){
	    this.uniformMatrixXfv(location, transpose, value, this.uniformMatrix2fvId);
	};

	GContextWebGL.prototype.uniformMatrix3fv = function(location, transpose, value){
	    this.uniformMatrixXfv(location, transpose, value, this.uniformMatrix3fvId);
	};

	GContextWebGL.prototype.uniformMatrix4fv = function(location, transpose, value){
	    this.uniformMatrixXfv(location, transpose, value, this.uniformMatrix4fvId);
	};


	GContextWebGL.prototype.useProgram = function(program){
	    this._drawCommands += (this.useProgramId + program.id + ";");
	};

	GContextWebGL.prototype.validateProgram = function(program){
	    // TODO:
	};

	GContextWebGL.prototype.vertexAttrib2fv = function(index, value){
	    this._drawCommands += this.vertexAttrib2fvId + index;
	    for (var i = 0; i < value.length; i++) {
	        this._drawCommands += ",";
	        this._drawCommands += value[i].toFixed(3);
	    }
	};

	GContextWebGL.prototype.vertexAttribPointer = function(index, size, type, normalized, stride, offset){
	    this._drawCommands += (this.vertexAttribPointerId + index + "," + size + ","
	    + type + "," + (normalized?1:0) + "," + stride + "," + offset + ";");
	};


	GContextWebGL.prototype.viewport = function(x, y, w, h) {
	    this._drawCommands += (this.viewportId + x + "," + y + ","
	    + w+ "," + h + ";");
	};

	GContextWebGL.prototype.getString = function(name) {
	    var args = name;
	    var result = GBridge.exeSyncCmd('getString',args);
	    return result;
	}
	//extension for OES_vertex_array_object
	GContextWebGL.prototype.bindVertexArrayOES = function(array) {
	    this._drawCommands += (this.bindVertexArrayOESId + array + ";");
	};

	GContextWebGL.prototype.deleteVertexArraysOES = function(size, arrayRef) {
	    this._drawCommands += (this.deleteVertexArraysOESId + size + "," + arrayRef + ";");
	};

	GContextWebGL.prototype.genVertexArraysOES = function(size, arrayRef) {
	    this._drawCommands += (this.genVertexArraysOESId + size + "," + arrayRef + ";");
	};

	GContextWebGL.prototype.isVertexArrayOES = function(array) {
	    var args = array;
	    var result = GBridge.exeSyncCmd('isVertexArrayOES',args);
	    return result;
	};

	//function autoInjectMetaViewport(){
	//    var metas = document.getElementsByTagName("meta");
	//    for (var i_matas = 0; i_matas < metas.length; ++i_matas) {
	//        var meta_name = metas[i_matas].getAttribute("name");
	//        if (meta_name == "viewport") {
	//            return;
	//        }
	//    }
	//    var injectMeta = "<meta n" + "ame='viewport' content='width=device-width, initial-scale=1.0' />"
	//    GLog.d("[autoInjectMetaViewport] injectMeta:" + injectMeta);
	//    document.write(injectMeta);
	//};
	//
	//autoInjectMetaViewport();






	module.exports = GContextWebGL;


/***/ }),

/***/ 117:
/***/ (function(module, exports, __webpack_require__) {

	var GBridge = __webpack_require__(113).GBridge;
	var GLog = __webpack_require__(113).GLog;
	var GHashMap = __webpack_require__(115);
	var GCanvasImage = __webpack_require__(112);

	function GContext2D() {
	    this._drawCommands = "";
	    this._globalAlpha = 1.0;
	    this._fillStyle = "rgb(0,0,0)";
	    this._strokeStyle = "rgb(0,0,0)";
	    this._lineWidth = 1;
	    this._lineCap = "butt";
	    this._lineJoin= "miter";
	    this._miterLimit = 10;
	    this._globalCompositeOperation = "source-over";
	    this._textAlign = "start";
	    this._textBaseline = "alphabetic";
	    this._font = "10px sans-serif";
	    this._images = {};
	    this._canvases1 = {};
	    this._canvases2 = {};
	    this._getImageData = new Array();

	//    GCanvas._forbiddenAutoReplaceCanvas =true;
	//    this._apiCanvas  = document.createElement('canvas');
	//    GCanvas._forbiddenAutoReplaceCanvas =false;
	//    console.error("apicanvas="+this._apiCanvas);
	//    this._apiContext = this._apiCanvas.getContext("2d");
	//    this._apiContext.font = this._font;

	    this._savedGlobalAlpha =[];
	    this.timer = null;
	    this.componentId = null;

	    this._imageMap = new GHashMap();
	    this._textureMap = new GHashMap();
	    this._firstBindFlag = true;

	}

	function FillStylePattern(img, pattern) {
	    this._style = pattern;
	    this._img = img;
	}

	function FillStyleLinearGradient(x0, y0, x1, y1) {
	    this._start_pos = { _x : x0, _y : y0 };
	    this._end_pos = { _x : x1, _y : y1 };
	    this._stop_count = 0;
	    this._stops = [0, 0, 0, 0, 0];
	}

	FillStyleLinearGradient.prototype.addColorStop = function(pos, color) {
	    if (this._stop_count < 5 && 0.0 <= pos && pos <= 1.0) {
	        this._stops[this._stop_count] = { _pos : pos, _color : color };
	        this._stop_count++;
	    }
	}

	function FillStyleRadialGradient(x0, y0, r0, x1, y1, r1) {
	    this._start_pos = { _x : x0, _y : y0, _r : r0 };
	    this._end_pos = { _x : x1, _y : y1, _r : r1 };
	    this._stop_count = 0;
	    this._stops = [0, 0, 0, 0, 0];
	}

	FillStyleRadialGradient.prototype.addColorStop = function(pos, color) {
	    if (this._stop_count < 5 && 0.0 <= pos && pos <= 1.0) {
	        this._stops[this._stop_count] = { _pos : pos, _color : color };
	        this._stop_count++;
	    }
	}

	/**
	 * Represents the alpha value to be used with drawing commands where 1 is
	 * completely visible and 0 is fully transparent.
	 *
	 * @type {number}
	 * @name GContext2D#globalAlpha
	 */
	Object.defineProperty(GContext2D.prototype, "globalAlpha", {
	    get : function() {
	        return this._globalAlpha;
	    },
	    set : function(value) {
	        // if (this._globalAlpha != value) {
	        this._globalAlpha = value;
	        this._drawCommands = this._drawCommands.concat("a" + value.toFixed(6)
	                + ";");
	        // }
	    }
	});

	/**
	 * Represents the color or style to use inside shapes. It can only be a
	 * string which must be parsed as CSS <color> value for now.
	 *
	 * @type {string}
	 * @name GContext2D#fillStyle
	 * @example // set context fillStyle context.fillStyle = 'rgb(121,194,245)';
	 */
	Object.defineProperty(GContext2D.prototype, "fillStyle", {
	    get : function() {
	        return this._fillStyle;
	    },
	    set : function(value) {
	        this._fillStyle = value;

	        if (typeof(value) == 'string') {
	            this._drawCommands = this._drawCommands.concat("F" + value + ";");
	        }
	        else if (value instanceof FillStylePattern) {
	            if (value._img instanceof Image) {
	                if (!(value._img.src in this._images)) {
	                    var new_image = GCanvas.createImage();
	                    new_image.width = value._img.width;
	                    new_image.height = value._img.height;
	                    new_image.src = value._img.src;
	                    new_image.complete = value._img.complete;
	                    this._images[value._img.src] = new_image;
	                } else {
	                    this._drawCommands = this._drawCommands.concat("G" + this._images[value._img.src]._id + "," + value._style + ";");
	                 }
	            }
	            else if (value._img instanceof GCanvasImage){
	                this._drawCommands = this._drawCommands.concat("G" + value._img._id + "," + value._style + ";");
	            }
	        }
	        else if (value instanceof FillStyleLinearGradient) {
	            var command = "D" + value._start_pos._x + "," + value._start_pos._y + ","
	                + value._end_pos._x + "," + value._end_pos._y + "," + value._stop_count;

	            for (var i = 0; i < value._stop_count; ++i) {
	                command += ("," + value._stops[i]._pos + "," + value._stops[i]._color);
	            }
	            this._drawCommands = this._drawCommands.concat(command + ";");
	            //console.log('createLinearGradient command -> ' + command);
	        }
	        else if (value instanceof FillStyleRadialGradient) {
	            var command = "H" + value._start_pos._x + "," + value._start_pos._y + "," + value._start_pos._r + ","
	                + value._end_pos._x + "," + value._end_pos._y + "," + value._end_pos._r + "," + value._stop_count;

	            for (var i = 0; i < value._stop_count; ++i) {
	                command += ("," + value._stops[i]._pos + "," + value._stops[i]._color);
	            }
	            this._drawCommands = this._drawCommands.concat(command + ";");
	            //console.log('FillStyleRadialGradient command -> ' + command);
	        }
	    }
	});

	/**
	 * Represents the color or style for the lines. It can only be a string
	 * which must be parsed as CSS <color> value for now.
	 *
	 * @type {string}
	 * @name GContext2D#strokeStyle
	 * @example // set context strokeStyle context.strokeStyle = 'rgb(121,194,245)';
	 */
	Object.defineProperty(GContext2D.prototype, "strokeStyle", {
	    get : function() {
	        return this._strokeStyle;
	    },
	    set : function(value) {
	        this._strokeStyle = value;

	        if (typeof(value) == 'string') {
	            this._drawCommands = this._drawCommands.concat("S" + value + ";");
	        }
	        else if (value instanceof FillStylePattern) {
	            if (value._img instanceof Image) {
	                if (!(value._img.src in this._images)) {
	                    var new_image = GCanvas.createImage();
	                    new_image.width = value._img.width;
	                    new_image.height = value._img.height;
	                    new_image.src = value._img.src;
	                    new_image.complete = value._img.complete;
	                    this._images[value._img.src] = new_image;
	                } else {
	                    this._drawCommands = this._drawCommands.concat("G" + this._images[value._img.src]._id + "," + value._style + ";");
	                 }
	            }
	            else if (value._img instanceof GCanvasImage){
	                this._drawCommands = this._drawCommands.concat("G" + value._img._id + "," + value._style + ";");
	            }
	        }
	        else if (value instanceof FillStyleLinearGradient) {
	            var command = "D" + value._start_pos._x + "," + value._start_pos._y + ","
	                + value._end_pos._x + "," + value._end_pos._y + "," + value._stop_count;

	            for (var i = 0; i < value._stop_count; ++i) {
	                command += ("," + value._stops[i]._pos + "," + value._stops[i]._color);
	            }
	            this._drawCommands = this._drawCommands.concat(command + ";");
	            //console.log('createLinearGradient command -> ' + command);
	        }
	        else if (value instanceof FillStyleRadialGradient) {
	            var command = "H" + value._start_pos._x + "," + value._start_pos._y + "," + value._start_pos._r + ","
	                + value._end_pos._x + "," + value._end_pos._y + "," + value._end_pos._r + "," + value._stop_count;

	            for (var i = 0; i < value._stop_count; ++i) {
	                command += ("," + value._stops[i]._pos + "," + value._stops[i]._color);
	            }
	            this._drawCommands = this._drawCommands.concat(command + ";");
	            //console.log('FillStyleRadialGradient command -> ' + command);
	        }
	    }
	});

	/**
	 * Represents the width of the lines.
	 *
	 * @type {number}
	 * @name GContext2D#lineWidth
	 * @example // set context lineWidth context.lineWidth = 2;
	 */
	Object.defineProperty(GContext2D.prototype, "lineWidth", {
	    get : function() {
	        return this._lineWidth;
	    },
	    set : function(value) {
	        this._lineWidth = value;
	        this._drawCommands = this._drawCommands.concat("W" + value
	                + ";");
	    }
	});
	/**
	 * The lineCap property sets or returns the style of the end caps for a line.
	 *
	 * @type {number}
	 * @name GContext2D#lineCap
	 * @example // set context lineCap context.lineCap="round";
	 */
	Object.defineProperty(GContext2D.prototype, "lineCap", {
	    get : function() {
	        return this._lineCap;
	    },
	    set : function(value) {
	        this._lineCap = value;
	        this._drawCommands = this._drawCommands.concat("C" + value + ";");
	    }
	});


	/**
	 * Sets or returns the type of corner created, when two lines meet
	 *
	 * @type {number}
	 * @name GContext2D#lineJoin
	 * @example // set context lineJoin context.lineJoin="round";
	 */
	Object.defineProperty(GContext2D.prototype, "lineJoin", {
	    get : function() {
	        return this._lineJoin;
	    },
	    set : function(value) {
	        this._lineJoin = value;
	        this._drawCommands = this._drawCommands.concat("J" + value + ";");
	    }
	});


	/**
	 * Sets or returns the maximum miter length
	 *
	 * @type {number}
	 * @name GContext2D#miterLimit
	 * @example // set context miterLimit context.miterLimit=10;
	 */
	Object.defineProperty(GContext2D.prototype, "miterLimit", {
	    get : function() {
	        return this._miterLimit;
	    },
	    set : function(value) {
	        this._miterLimit = value;
	        this._drawCommands = this._drawCommands.concat("M" + value + ";");
	    }
	});

	/**
	 * Represents the globalCompositeOperation value to be used with drawing
	 * commands where 1 is completely visible and 0 is fully transparent.
	 *
	 * @type {number}
	 * @name GContext2D#globalCompositeOperation
	 */
	Object.defineProperty(GContext2D.prototype, "globalCompositeOperation", {
	    get : function() {
	        return this._globalCompositeOperation;
	    },

	    set : function(value) {
	        // if (this._globalCompositeOperation != value) {

	        this._globalCompositeOperation = value;
	        var mode = 0;
	        switch (value) {
	        case "source-over":
	            mode = 0;
	            break;
	        case "source-atop":
	            mode = 5;
	            break;
	        case "source-in":
	            mode = 0;
	            break;
	        case "source-out":
	            mode = 2;
	            break;
	        case "destination-over":
	            mode = 4;
	            break;
	        case "destination-atop":
	            mode = 4;
	            break;
	        case "destination-in":
	            mode = 4;
	            break;
	        case "destination-out":
	            mode = 3;
	            break;
	        case "lighter":
	            mode = 1;
	            break;
	        case "copy":
	            mode = 2;
	            break;
	        case "xor":
	            mode = 6;
	            break;
	        default:
	            mode = 0;
	        }

	        this._drawCommands = this._drawCommands.concat("B" + mode + ";");
	        // }
	    }
	});

	/**
	 * Represents the textAlign value to be used with drawing commands
	 *
	 * @type {number}
	 * @name GContext2D#textAlign
	 */
	Object.defineProperty(GContext2D.prototype, "textAlign", {
	    get : function() {
	        return this._textAlign;
	    },

	    set : function(value) {
	        // if (this._textAlign != value) {
	        this._textAlign = value;
	        var Align = 0;
	        switch (value) {
	        case "start":
	            Align = 0;
	            break;
	        case "end":
	            Align = 1;
	            break;
	        case "left":
	            Align = 2;
	            break;
	        case "center":
	            Align = 3;
	            break;
	        case "right":
	            Align = 4;
	            break;
	        default:
	            Align = 0;
	        }

	        this._drawCommands = this._drawCommands.concat("A" + Align + ";");
	        // }
	    }

	});

	/**
	 * Represents the _textBaseline value to be used with drawing commands
	 *
	 * @type {number}
	 * @name GContext2D#_textBaseline
	 */
	Object.defineProperty(GContext2D.prototype, "textBaseline", {
	    get : function() {
	        return this._textBaseline;
	    },

	    set : function(value) {
	        this._textBaseline = value;
	        var baseline = 0;
	        switch (value) {
	        case "alphabetic":
	            baseline = 0;
	            break;
	        case "middle":
	            baseline = 1;
	            break;
	        case "top":
	            baseline = 2;
	            break;
	        case "hanging":
	            baseline = 3;
	            break;
	        case "bottom":
	            baseline = 4;
	            break;
	        case "ideographic":
	            baseline = 5;
	            break;
	        default:
	            baseline = 0;
	            break;
	        }

	        this._drawCommands = this._drawCommands.concat("E" + baseline + ";");
	    }

	});

	/**
	 * Represents the textAlign value to be used with drawing commands
	 *
	 * @type {number}
	 * @name GContext2D#textAlign
	 */
	Object.defineProperty(GContext2D.prototype, "font", {
	    get : function() {
	        return this._font;
	    },
	    set : function(value) {
	        // if (this._font != value) {
	        this._font = value;
	        //this._apiContext.font = this._font;
	        this._drawCommands = this._drawCommands.concat("j" + value + ";");
	        // }
	    }

	});

	/**
	 * Loads an image into the plugin to be used as a texture in the GCanvas.
	 * Generally this method is never called directly. Instead, it is called
	 * indirectly through GCanvasImage instances upon setting their
	 * {@link GCanvasImage#src|GCanvasImage.src} property.
	 *
	 * @param {GCanvasImage}
	 *            image The image to be loaded into the GCanvas plugin.
	 * @param {function}
	 *            [successCallback] A callback that is fired when the image has
	 *            been successfully loaded.
	 * @param {function}
	 *            [errorCallback] A callback that is fired when there was an
	 *            error in loading the image.
	 * @example // create a new image and load // it from a relative URL path
	 *          var myImage = GCanvas.createImage(); myImage.src =
	 *          "images/spritesheet.jpg"; // calls loadTexture for you
	 * @private
	 */
	GContext2D.prototype.loadTexture = function(image, successCallback, errorCallback) {
	    var data = this._imageMap.get(image.src);
	    if( data )
	    {
	        successCallback && successCallback(data);
	        return;
	    }

	    var that = this;
	    GBridge.preLoadImage([image.src, image.id], function(e){
	        if (e){
	            that._imageMap.put(image.src, e);
	            successCallback && successCallback(e);
	        }else{
	            GLog.d("GContext2D loadTexture errorCallback!");
	            errorCallback && errorCallback(e);
	        }
	    });
	};

	/**
	 * Unloads an image from the GCanvas plugin. Generally this method is
	 * never called directly. Instead, it is called indirectly through
	 * GCanvasImage instances upon setting their
	 * {@link GCanvasImage#src|GCanvasImage.src} property to a false value
	 * such as <code>null</code> or an empty string (<code>""</code>).
	 *
	 * @param {GCanvasImage}
	 *            image The image to be unloaded from the GCanvas plugin.
	 * @example // unload an image from memory myImage.src = null; // calls
	 *          unloadTexture for you
	 * @private
	 */
	GContext2D.prototype.unloadTexture = function(image) {
	    this._imageMap.remove(image.src);
	};

	/**
	 * Defines the 2D matrix transform applied to drawings within the context.
	 *
	 * @param {number}
	 *            a The value that affects the positioning of pixels along the x
	 *            axis when scaling or rotating the context.
	 * @param {number}
	 *            b The value that affects the positioning of pixels along the y
	 *            axis when rotating or skewing the context.
	 * @param {number}
	 *            c The value that affects the positioning of pixels along the x
	 *            axis when rotating or skewing the context.
	 * @param {number}
	 *            d The value that affects the positioning of pixels along the y
	 *            axis when scaling or rotating the context.
	 * @param {number}
	 *            tx The distance by which to translate the context along the x
	 *            axis.
	 * @param {number}
	 *            ty The distance by which to translate the context along the y
	 *            axis.
	 */
	GContext2D.prototype.setTransform = function(a, b, c, d, tx, ty) {
	    this._drawCommands = this._drawCommands.concat("t"
	            + (a === 1 ? "1" : a.toFixed(6)) + ","
	            + (b === 0 ? "0" : b.toFixed(6)) + ","
	            + (c === 0 ? "0" : c.toFixed(6)) + ","
	            + (d === 1 ? "1" : d.toFixed(6)) + "," + tx + "," + ty + ";");
	};

	/**
	 * Defines an added 2D matrix transform applied to drawings within the
	 * context.
	 *
	 * @param {number}
	 *            a The value added to the value that affects the positioning of
	 *            pixels along the x axis when scaling or rotating the context.
	 * @param {number}
	 *            b The value added to the value that affects the positioning of
	 *            pixels along the y axis when rotating or skewing the context.
	 * @param {number}
	 *            c The value added to the value that affects the positioning of
	 *            pixels along the x axis when rotating or skewing the context.
	 * @param {number}
	 *            d The value added to the value that affects the positioning of
	 *            pixels along the y axis when scaling or rotating the context.
	 * @param {number}
	 *            tx The value added to the distance by which to translate the
	 *            context along the x axis.
	 * @param {number}
	 *            ty The value added to the distance by which to translate the
	 *            context along the y axis.
	 */
	GContext2D.prototype.transform = function(a, b, c, d, tx, ty) {
	    this._drawCommands = this._drawCommands.concat("f"
	            + (a === 1 ? "1" : a.toFixed(6)) + ","
	            + (b === 0 ? "0" : b.toFixed(6)) + ","
	            + (c === 0 ? "0" : c.toFixed(6)) + ","
	            + (d === 1 ? "1" : d.toFixed(6)) + "," + tx + "," + ty + ";");
	};

	/**
	 * Restores the 2D matrix transform to the identity matrix. This is
	 * equivalent to calling <code>context.setTransform(1,0,0,1,0,0)</code>.
	 */
	GContext2D.prototype.resetTransform = function() {
	    this._drawCommands = this._drawCommands.concat("m;");
	};

	/**
	 * Scales the 2D matrix transform along the x and y axes.
	 *
	 * @param {number}
	 *            a The value added to the value that affects the positioning of
	 *            pixels along the x axis when scaling or rotating the context.
	 * @param {number}
	 *            d The value added to the value that affects the positioning of
	 *            pixels along the y axis when scaling or rotating the context.
	 */
	GContext2D.prototype.scale = function(a, d) {
	    this._drawCommands = this._drawCommands.concat("k" + a.toFixed(6) + ","
	            + d.toFixed(6) + ";");
	};

	/**
	 * Rotates the 2D matrix transform by a specified number of radians.
	 *
	 * @param {number}
	 *            angle The value in radians to rotate the context.
	 */
	GContext2D.prototype.rotate = function(angle) {
	    this._drawCommands = this._drawCommands
	            .concat("r" + angle.toFixed(6) + ";");
	};

	/**
	 * Moves the 2D matrix transform along the x and y axes.
	 *
	 * @param {number}
	 *            tx The value added to the distance by which to translate the
	 *            context along the x axis.
	 * @param {number}
	 *            ty The value added to the distance by which to translate the
	 *            context along the y axis.
	 */
	GContext2D.prototype.translate = function(tx, ty) {
	    this._drawCommands = this._drawCommands.concat("l" + tx + "," + ty + ";");
	};

	/**
	 * Sets a save point for the current context transform. This allows you to
	 * arbitrarily modify the transform and restore it back to its to its
	 * original state at the time save() was called by using restore().
	 *
	 * @see GContext2D#restore
	 */
	GContext2D.prototype.save = function() {
	    this._savedGlobalAlpha.push(this._globalAlpha);
	    this._drawCommands = this._drawCommands.concat("v;");
	};

	/**
	 * Restores the state of the context transform to the state at the point in
	 * time when save() was last called.
	 *
	 * @see GContext2D#save
	 */
	GContext2D.prototype.restore = function() {
	    this._drawCommands = this._drawCommands.concat("e;");
	    this._globalAlpha = this._savedGlobalAlpha.pop();
	};


	GContext2D.prototype._concatDrawCmd = function(numArgs, imageInfo,
	    sx, sy, sw, sh, // source (or destination if fewer args)
	    dx, dy, dw, dh){// destination

	    if(!imageInfo){
	        return;
	    }

	    if(numArgs === 3){
	        var x = parseFloat(sx) || 0;
	        var y = parseFloat(sy) || 0;

	        this._drawCommands += ("d" + imageInfo.id + ",0,0,"
	            + imageInfo.width + "," + imageInfo.height + ","
	            + x + "," + y + "," + imageInfo.width + "," + imageInfo.height + ";");
	    }else if(numArgs === 5){
	        var x = parseFloat(sx) || 0;
	        var y = parseFloat(sy) || 0;
	        var width = parseInt(sw) || imageInfo.width;
	        var height = parseInt(sh) || imageInfo.height;

	        this._drawCommands += ("d" + imageInfo.id + ",0,0,"
	            + imageInfo.width + "," + imageInfo.height + ","
	            + x + "," + y + "," + width + "," + height + ";");
	    }else if(numArgs === 9){
	        var sx = parseFloat(sx) || 0;
	        var sy = parseFloat(sy) || 0;
	        var sw = parseInt(sw) || imageInfo.width;
	        var sh = parseInt(sh) || imageInfo.height;
	        var dx = parseFloat(dx) || 0;
	        var dy = parseFloat(dy) || 0;
	        var dw = parseInt(dw) || imageInfo.width;
	        var dh = parseInt(dh) || imageInfo.height;

	        this._drawCommands += ("d" + imageInfo.id + ","
	            + sx + "," + sy + "," + sw + "," + sh + ","
	            + dx + "," + dy + "," + dw + "," + dh + ";");
	    }
	};

	GContext2D.prototype.drawImage = function(image, // image
	    sx, sy, sw, sh, // source (or destination if fewer args)
	    dx, dy, dw, dh) { // destination

	    //GLog.d("[GContext2D.drawImage] start...");

	    var that = this;
	    var numArgs = arguments.length;

	    var imageCache = this._getImageTexture(image.src);
	    if (imageCache) {
	        this._concatDrawCmd(numArgs, image, sx, sy, sw, sh, dx, dy, dw, dh);
	        return;
	    }

	    if( GBridge.isIOS() )
	    {
	        GBridge.bindImageTexture(that.componentId, image.src, function(){});
	        that._concatDrawCmd(numArgs, image, sx, sy, sw, sh, dx, dy, dw, dh);
	        that._saveImageTexture(image.src, image);
	    }
	    else
	    {
	        GBridge.bindImageTexture(that.componentId, image.src, function(e){
	            if( !e.error )
	            {
	                if(image.width === 0 && e.width > 0){
	                  image.width = e.width;
	                }

	                if(image.height === 0 && e.height > 0){
	                  image.height = e.height;
	                }
	                that._concatDrawCmd(numArgs, image, sx, sy, sw, sh, dx, dy, dw, dh);
	                that._saveImageTexture(image.src, image);
	            }
	        });
	    }
	};


	GContext2D.prototype._getImageTexture = function(url){
	    if( url )
	    {
	        return this._textureMap.get(url);
	    }
	    return null;
	}

	GContext2D.prototype._removeImageTexture = function(url){
	    if( url )
	    {
	        this._textureMap.remove(url);
	    }
	}


	GContext2D.prototype._saveImageTexture = function(url, e){
	    if( e && e.src )
	    {
	        this._textureMap.put(url, e);
	    }
	}

	GContext2D.prototype._clearImageTextures = function(){
	  this._textureMap.clear();
	}


	/**
	 * Informs the drawing context that drawing commands have completed for the
	 * current frame and the should be sent to the GCanvas plugin for drawing
	 * to the screen.
	 * <p>
	 * This method is unique to GContext2D and does not exist within the HTML
	 * 2D context, so the utility method {@link GCanvas.render} should be
	 * used to make it easy to call or not call this method depending on the
	 * context you are currently working with.
	 * </p>
	 *
	 * @example // makes necessary GCanvas render call // if canvas being
	 *          used is GCanvas var myCanvas = GCanvas.create(); var
	 *          myContext = myCanvas.getContext("2d");
	 *  // ... myContext.translate(10,10); myContext.rotate(Math.PI); //
	 * ...
	 *  // after all context calls are complete // for the current frame:
	 * GCanvas.render(); // calls GContext2D.render()
	 */

	GContext2D.prototype.render = function(flag) {
	    if (this.timer && typeof flag === "undefined"){
	        clearInterval(this.timer);
	        this.timer = null;
	    }

	    var commands = this._drawCommands;
	    this._drawCommands = "";
	    if (commands !== null && commands !== "") {
	        GBridge.callRender(this.componentId, commands);
	    }
	};

	/**
	 * Implementation of GCanvas.capture.
	 *
	 * @private
	 */
	GContext2D.prototype.capture = function(x, y, w, h, fileName, successCallback, errorCallback) {
	    // if (successCallback && typeof successCallback !== 'function') {
	    //     throw new Error('successCallback parameter not a function');
	    // }
	    // if (errorCallback && typeof errorCallback !== 'function') {
	    //     throw new Error('errorCallback parameter not a function');
	    // }

	    // GCanvas._toNative(successCallback, errorCallback, 'GCanvas',
	    //         'capture', [ x, y, w, h, fileName ]);
	};


	GContext2D.prototype.createPattern = function(img, pattern) {
	    return new FillStylePattern(img, pattern);
	};

	/**
	 * Implementation of GCanvas.createLinearGradient(x0, y0, x1, y1).
	 *
	 * @private
	 */
	 GContext2D.prototype.createLinearGradient = function(x0, y0, x1, y1) {
	    return new FillStyleLinearGradient(x0, y0, x1, y1);
	};

	/**
	 * Implementation of GCanvas.createRadialGradient(x0, y0, x1, y1).
	 *
	 * @private
	 */
	 GContext2D.prototype.createRadialGradient = function(x0, y0, r0, x1, y1, r1) {
	    return new FillStyleRadialGradient(x0, y0, r0, x1, y1, r1);
	};

	GContext2D.prototype.strokeRect = function(x, y, w, h, successCallback,
	        errorCallback) {
	    this._drawCommands = this._drawCommands.concat("s" + x + "," + y + "," + w
	            + "," + h + ";");
	};

	GContext2D.prototype.clearRect = function(x, y, w, h, successCallback,
	        errorCallback) {
	    // TODO: enable it later.
	    this._drawCommands = this._drawCommands.concat("c" + x + "," + y + "," + w
	        + "," + h + ";");
	}

	GContext2D.prototype.clip = function(successCallback, errorCallback) {
	    this._drawCommands = this._drawCommands.concat("p;");
	}

	GContext2D.prototype.resetClip = function(successCallback, errorCallback) {
	    this._drawCommands = this._drawCommands.concat("q;");
	}

	GContext2D.prototype.closePath = function(successCallback, errorCallback) {
	    this._drawCommands = this._drawCommands.concat("o;");
	}

	GContext2D.prototype.moveTo = function(x, y, successCallback, errorCallback) {
	    this._drawCommands = this._drawCommands.concat("g" + x.toFixed(6) + ","
	            + y.toFixed(6) + ";");
	}

	GContext2D.prototype.lineTo = function(x, y, successCallback, errorCallback) {
	    this._drawCommands = this._drawCommands.concat("i" + x.toFixed(6) + ","
	            + y.toFixed(6) + ";");
	}

	GContext2D.prototype.quadraticCurveTo = function(cpx, cpy, x, y,
	        successCallback, errorCallback) {
	    this._drawCommands = this._drawCommands.concat("u" + cpx + "," + cpy + ","
	            + x + "," + y + ";");
	}

	GContext2D.prototype.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y,
	        successCallback, errorCallback) {
	    this._drawCommands = this._drawCommands.concat("z" + cp1x + "," + cp1y
	            + "," + cp2x + "," + cp2y + "," + x + "," + y + ";");
	}

	GContext2D.prototype.arcTo = function(x1, y1, x2, y2, radius,
	        successCallback, errorCallback) {
	    this._drawCommands = this._drawCommands.concat("h" + x1 + "," + y1 + ","
	            + x2 + "," + y2 + "," + radius + ";");
	}

	/**
	 * Resets the current default path.
	 *
	 * @param null
	 */
	GContext2D.prototype.beginPath = function() {
	    this._drawCommands = this._drawCommands.concat("b;");
	};

	/**
	 * Paint the specified rectangular area using the fillStyle. If either
	 * height or width are zero, this method has no effect.
	 *
	 * @param {number}
	 *            x The x location of the source clipping rectangle
	 * @param {number}
	 *            y The y location of the source clipping rectangle
	 * @param {number}
	 *            w The width of the rectangle
	 * @param {number}
	 *            h The height of the rectangle
	 */
	GContext2D.prototype.fillRect = function(x, y, w, h) {
	    this._drawCommands = this._drawCommands.concat("n" + x + "," + y + "," + w
	            + "," + h + ";");
	};

	/**
	 * Adds a new closed subpath to the path, representing the given rectangle.
	 *
	 * @param {number}
	 *            x The x location of the rectangle
	 * @param {number}
	 *            y The y location of the rectangle
	 * @param {number}
	 *            w The width of the rectangle
	 * @param {number}
	 *            h The height of the rectangle
	 */
	GContext2D.prototype.rect = function(x, y, w, h) {
	    this._drawCommands = this._drawCommands.concat("w" + x + "," + y + "," + w
	            + "," + h + ";");
	};

	/**
	 * Fills the subpaths of the current default path or the given path with the
	 * current fill style.
	 *
	 * @param {string}
	 *            path The given path to fill.
	 */
	GContext2D.prototype.fill = function(path) {
	    this._drawCommands = this._drawCommands.concat("L;");
	};

	/**
	 * Strokes the subpaths of the current default path or the given path with
	 * the current stroke style.
	 *
	 * @param {string}
	 *            path The given path to stroke.
	 */
	GContext2D.prototype.stroke = function(path) {
	    this._drawCommands = this._drawCommands.concat("x;");
	};

	/**
	 * Adds points to the subpath such that the arc described by the
	 * circumference of the circle described by the arguments, starting at the
	 * given start angle and ending at the given end angle, going in the given
	 * direction (defaulting to clockwise), is added to the path, connected to
	 * the previous point by a straight line.
	 *
	 * @param {number}
	 *            x
	 * @param {number}
	 *            y
	 * @param {number}
	 *            radius
	 * @param {number}
	 *            startAngle
	 * @param {number}
	 *            endAngle
	 * @param {string}
	 *            anticlockwise
	 */
	GContext2D.prototype.arc = function(x, y, radius, startAngle, endAngle,
	        anticlockwise) {

	    var ianticlockwise = 0;
	    if (anticlockwise)
	        ianticlockwise = 1;

	    this._drawCommands = this._drawCommands.concat("y" + x + "," + y + ","
	            + radius + "," + startAngle + "," + endAngle + "," + ianticlockwise
	            + ";");
	};



	GContext2D.prototype.fillText = function(text, x, y) {
	    var tmptext =text.replace(/!/g,"!!");
	        tmptext =tmptext.replace(/,/g,"!,");
	        tmptext =tmptext.replace(/;/g,"!;");
	    this._drawCommands = this._drawCommands.concat("T" + tmptext + "," + x + ","
	            + y + ",0.0;");
	};

	GContext2D.prototype.strokeText = function(text, x, y) {
	    this._drawCommands = this._drawCommands.concat("U" + text + "," + x + ","
	            + y + ",0.0;");
	};


	//TODO:这个api有用需要原生的canvas对象，所以不支持
	GContext2D.prototype.measureText = function(text) {
	    return -1;
	    //return this._apiContext.measureText(text);
	};

	GContext2D.prototype.isPointInPath = function(x,y) {
	    return true;
	};


	/////////////////////////////////////////////////////////////////
	//base64
	/////////////////////////////////////////////////////////////////

	function GarrToBase64(buffer) {
	    var binary = ''
	    var bytes = new Uint8Array( buffer )
	    var len = bytes.byteLength;
	    for (var i = 0; i < len; i++) {
	        binary += String.fromCharCode( bytes[ i ] )
	    }
	    return btoa( binary );
	}

	function _GcharDecode (nChr) {
	  return nChr > 64 && nChr < 91 ?
	      nChr - 65
	    : nChr > 96 && nChr < 123 ?
	      nChr - 71
	    : nChr > 47 && nChr < 58 ?
	      nChr + 4
	    : nChr === 43 ?
	      62
	    : nChr === 47 ?
	      63
	    :
	      0;
	}

	function Gbase64ToArr (sBase64, nBlocksSize) {
	  var
	    sB64Enc = sBase64.replace(/[^A-Za-z0-9\+\/]/g, ""), nInLen = sB64Enc.length,
	    nOutLen = nBlocksSize ? Math.ceil((nInLen * 3 + 1 >> 2) / nBlocksSize) * nBlocksSize : nInLen * 3 + 1 >> 2, taBytes = new Uint8Array(nOutLen);

	  for (var nMod3, nMod4, nUint24 = 0, nOutIdx = 0, nInIdx = 0; nInIdx < nInLen; nInIdx++) {
	    nMod4 = nInIdx & 3;
	    nUint24 |= _GcharDecode(sB64Enc.charCodeAt(nInIdx)) << 18 - 6 * nMod4;
	    if (nMod4 === 3 || nInLen - nInIdx === 1) {
	      for (nMod3 = 0; nMod3 < 3 && nOutIdx < nOutLen; nMod3++, nOutIdx++) {
	        taBytes[nOutIdx] = nUint24 >>> (16 >>> nMod3 & 24) & 255;
	      }
	      nUint24 = 0;

	    }
	  }

	  return taBytes;
	}



	/////////////////////////////////////////////////////////////////
	//GCanvasImage
	/////////////////////////////////////////////////////////////////
	function GImageData(w, h) {
	    GLog.d("GImageData wh=" + w + "," + h);
	    this.width = w;
	    this.height = h;
	    this.data = new Uint8Array(w*h*4);
	}

	GContext2D.prototype.createImageData = function(w, h) {
	    GLog.d("GContext2D::createImageData wh=" + w + "," + h);
	    return new GImageData(w,h);
	};


	GContext2D.prototype._putImageData = function(data,dx, dy, sw, sh,  dw, dh){
	    this._drawCommands = this._drawCommands.concat("P"
	        + dx + ","
	        + dy + ","
	        + sw + ","
	        + sh + ","
	        + dw + ","
	        + dh + ","
	        + GarrToBase64(data) + ";");
	}
	GContext2D.prototype.putImageData = function(imgData, x, y, dirtyX, dirtyY, dirtyWidth, dirtyHeight) {
	    GLog.d("GContext2D::putImageData [" + arguments.length + "] "
	        + "dest_xy=(" + x + "," + y + ") "
	        + "dirty_xy=(" + dirtyX + "," + dirtyY + ") "
	        + "dirty_wh=(" + dirtyWidth + "," + dirtyHeight + ") ");

	    if (arguments.length <= 3){
	        this._putImageData(imgData.data, x, y, imgData.width, imgData.height, imgData.width, imgData.height);
	    }
	    else{
	        var destData = new Uint8Array(dirtyWidth*dirtyHeight*4);
	        var imgPos;
	        var destPos = 0
	        for(var i =0; i < dirtyHeight; i++){
	            imgPos = (imgData.width*(dirtyY + i) + dirtyX)*4;
	            for(var j=0; j< dirtyWidth; ++j){
	                destData[destPos++]=imgData.data[imgPos++];
	                destData[destPos++]=imgData.data[imgPos++];
	                destData[destPos++]=imgData.data[imgPos++];
	                destData[destPos++]=imgData.data[imgPos++];
	            }
	        }

	        this._putImageData(destData, x+dirtyX, y+dirtyY, dirtyWidth, dirtyHeight, dirtyWidth, dirtyHeight);
	    }
	};


	GContext2D.prototype.getImageDataAsyn = function(x, y, w, h) {
	    return '';
	    // GLog.d("GContext2D::getImageDataAsyn xy=(" + x + "," + y + "), wh=(" + w + ","+ h +")");
	    // GCanvas._instance.getContext().render("auto");
	    // var len = w*h;
	    // var imgData = new GImageData(w,h);
	    // imgData._x = x;
	    // imgData._y = y;
	    // imgData._dataGet = 0;
	    // imgData._split = 0;
	    // var me = this;
	    // me._getImageData.push(imgData);

	    // var h2 = Math.floor(262144/w);// 2^18
	    // if (h2 < h)
	    //     imgData._split = 1;

	    // function getImageDataAsynSuccess(getData) {
	    //     var destData = me._getImageData[0];
	    //     GLog.d("GContext2D::getImageDataAsyn: dataGet=" + destData._dataGet);
	    //     if (0 == destData._split){// one part
	    //         destData.data = Gbase64ToArr(getData);
	    //         destData._dataGet += destData.data.length;
	    //     }else{// multi parts
	    //         var taBytes  = Gbase64ToArr(getData);
	    //         destData._dataGet += taBytes.length;
	    //         for (var i=0;i<taBytes.length;i++){
	    //             destData.data[destData._dataGet+i] = taBytes[i];
	    //         }
	    //     }

	    //     if (destData._dataGet >= (destData._x*destData._y)){
	    //         if (typeof destData.onload === 'function') {
	    //             GLog.d("GContext2D::getImageDataAsyn: callback exec.");
	    //             destData.onload();
	    //         }
	    //         me._getImageData.splice(0,1);//delete first data
	    //     }
	    // }

	    // for(var i=0; i<h; i+= h2){
	    //     GCanvas._toNative(getImageDataAsynSuccess, getImageDataAsynSuccess, 'GCanvas',
	    //             'getImageData', [ x, y+i, w, (i+h2>h)?(h-i):h2 ]);
	    // }


	    // return imgData;
	};



	module.exports = GContext2D;


/***/ }),

/***/ 118:
/***/ (function(module, exports) {

	
	var context;
	function getProto (Weex) {
	  return {
	    create: function () {
	      this.node = document.createElement('canvas')
	      return this.node
	    },
	    getContext: function (type) {
	      context = this.node.getContext(type || '2d')
	      return context
	    },

	    getContextAsyn: function (type, callback) {
	      context = this.node.getContext(type || '2d')
	      callback(context)
	    },

	    drawImage: function (url) {
	      var img = new Image();
	      img.src = url
	      arguments[0] = img
	      return context.drawImage.apply(context, arguments)
	    }

	  }
	}

	function init (Weex) {
	  var Component = Weex.Component
	  var extend = Weex.utils.extend

	  function GCanvas (data) {
	    Component.call(this, data)
	  }

	  GCanvas.prototype = Object.create(Component.prototype)
	  extend(GCanvas.prototype, getProto(Weex))

	  Weex.registerComponent('gcanvas', GCanvas)
	}

	module.exports =  {
	  init: init
	}



/***/ }),

/***/ 119:
/***/ (function(module, exports) {

	module.exports={render:function (){var _vm=this;var _h=_vm.$createElement;var _c=_vm._self._c||_h;
	  return _c('div', {
	    ref: "test"
	  }, [_c(_vm.tagName, {
	    ref: "canvas_holder",
	    tag: "component",
	    staticStyle: {
	      width: "750px",
	      height: "750px"
	    },
	    attrs: {
	      "width": "750",
	      "height": "750"
	    }
	  })], 1)
	},staticRenderFns: []}
	module.exports.render._withStripped = true

/***/ })

/******/ });