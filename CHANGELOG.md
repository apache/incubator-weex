# Abstract
## Feature
* Support direction:rtl
	* For international useage, support [direction:rtl](https://developer.mozilla.org/en-US/docs/Web/CSS/direction)
	* Pull Request: https://github.com/apache/incubator-weex/pull/1664

* Support richtext component
	* Support richtext component [richtext](http://weex-project.io/cn/references/components/richtext.html)
	* Pull Request: https://github.com/apache/incubator-weex/pull/1796

*  Performance optimization(init)
	* rm useless log && reduce reflection
	* Pull Request: https://github.com/apache/incubator-weex/pull/1844

* **Major change** refine weex code:（integration code from weex_core and weex_v8core）
    * pr : https://github.com/apache/incubator-weex/pull/1999
    * change log:
    	* Modify file naming from capitalized hump to lowercase
    	* Modify the header file inclusion method, mainly based on google style
    	* Change the file directory and move the basic function files to the base directory
    	* Merge jssengine code into WeexCore repository, integrate compilation
    	* Add libWTF.so to adapt to compile
    	
* send interaction time with event(after 8s)
	* Pull Request ：
		* https://github.com/apache/incubator-weex/pull/2000
		* https://github.com/apache/incubator-weex/pull/2002

* JsEngine multi-threaded
    * The jss sub-process adds I/O Thread, which is specially used to handle IPC message forwarding. Move the IPC operation originally attached to jsThread to I/O Thread. 
    * The jss sub-process adds a JSThread to execute the tag. The page instance for use_back_thread=true holds a VM for each thread, independent of each other. The two threads do not interfere with each other, so that they can run in parallel.

* ***Major change** Remove AutoConfigScan
    * Pull Request: https://github.com/apache/incubator-weex/pull/2142

* Throws the exception in the componentHooks callback and hands it over to the client.
    * Pull Request: https://github.com/apache/incubator-weex/pull/2285

* Add completion callback for registerService.
  * Pull Request: https://github.com/apache/incubator-weex/pull/1776

* Supprot devtool show the new performance info
  * Pull Request : <https://github.com/apache/incubator-weex/pull/1751>

* Support change device width and height by external interface
  * Pull Request:  <https://github.com/apache/incubator-weex/pull/2263>

## Bugfix
*  Emoji not show on Android 9.0(init)
	* Emoji not show on Android 9.0(init)
	* Pull Request:  https://github.com/apache/incubator-weex/pull/1804/files

* Support ontouch event on scroller component
	* Pull Request: https://github.com/apache/incubator-weex/pull/1896

* Fix NPE && No such Element Exception
	* Pull Request: https://github.com/apache/incubator-weex/pull/1896

* Bugfix java.lang.NullPointerException: println needs a message
	* Pull Request: https://github.com/apache/incubator-weex/pull/1925

* ignore fixed element for interactionTime
	* Pull Request：https://github.com/apache/incubator-weex/pull/1900

* Use JSON to find bundleType
	* Pull Request：https://github.com/apache/incubator-weex/pull/1927

* Upload attribute information if list's column count equals or smaller than 0 
	* Pull Request: https://github.com/apache/incubator-weex/pull/1957/files

* try to fix threadpool rejectexception cause  shutdown 
	* Pull Request：https://github.com/apache/incubator-weex/pull/2001

* try to fix js exception: scroll event.xxx undefined
    * Pull Request: https://github.com/apache/incubator-weex/pull/2034

* fix multithread crash : task excute after instance destroy
    * Pull Request: https://github.com/apache/incubator-weex/pull/2029